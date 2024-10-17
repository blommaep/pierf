// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "Packet.hpp"
#include "Port.hpp"
#include "Ethernet.hpp"
#include "Raw.hpp"
#include <sstream>

Packet::Packet()
  : mOutPort(NULL), mRawPacket(NULL), mRawSize(0), mAllocSize(0), mAuto(eAutoEnherit),mBinaryReady(false),mAnalysisReady(false), mHasVar(false), mShaper(NULL)
  {
  }

Packet::Packet(char* id)
  : mOutPort(NULL), mRawPacket(NULL), mRawSize(0), mAllocSize(0), mId(id),mBinaryReady(false),mAnalysisReady(false), mHasVar(false), mShaper(NULL)
  {
  }

Packet::~Packet()
  {
  vector<Element *>::iterator iter;
  for (iter = mElems.begin();iter != mElems.end();iter++)
    {
    Element* elem= *iter;
    delete elem;
    }  

//  if (mRawPacket != NULL) // Currently raw packet isn't copied but pointer is kept
//    {
//    delete mRawPacket; 
//    mRawPacket = NULL;
//    }

  }

void Packet::setBinaryReady(bool ready)
  {
  mBinaryReady = true;
  }

void Packet::setAnalysisReady(bool ready)
  {
  mAnalysisReady = true;
  }

bool Packet::getBinaryReady()
  {
  return mBinaryReady;
  }

bool Packet::getAnalysisReady()
  {
  return mAnalysisReady;
  }

void Packet::setRawPacket(uchar* rawPacket, ulong rawSize) throw (Exception)
  {
  if (mBinaryReady)
    {
    throw Exception("Adding new binary in a packet that has already one. Code bug?");
    }

  mRawPacket = rawPacket;
  mRawSize = rawSize;
  mBinaryReady = true;
  }

void Packet::analyse() throw (Exception)
  {
  if (mAnalysisReady)
    {
    // Not again
    return;
    }

  if (!mBinaryReady)
    {
    throw Exception("Trying to do analysis while no binary packet was available. Code bug?");
    }

  if (!mRawSize > 0)
    {
    throw Exception("Trying to analyse a packet of size 0???");
    }

  uchar* packetpos = mRawPacket;
  ulong remainingSize = mRawSize;

  Ethernet* eth = new Ethernet;
  if (!eth->analyze_Head(packetpos,remainingSize)) // not an ethernet packet, so can only consider it as raw bytes
    {
    delete eth;
    Raw* raw = new Raw;
    raw->analyze_Head(packetpos,remainingSize);
    push_back(raw);
    }
  else
    {
    bool succeed = true;
    Element* curelem = eth;
    Element* nextelem = eth->analyze_GetNextElem();
    while (remainingSize > 0 && nextelem != NULL && succeed)
      {
      push_back(curelem);
      curelem = nextelem;
      succeed = curelem->analyze_Head(packetpos,remainingSize);
      if (succeed)
        {
        nextelem = curelem->analyze_GetNextElem();
        }
      }
    push_back(curelem); // the last one still to be pushed
    
    // The rest are raw bytes.
    if (remainingSize > 0 && nextelem == NULL)
      {
      Raw* raw = new Raw;
      raw->analyze_Head(packetpos,remainingSize);
      push_back(raw);
      }

    // Now the analyseTail
    vector<Element *>::reverse_iterator riter;

    for (riter = mElems.rbegin();(riter != mElems.rend() && succeed);riter++)
      {
      curelem = *riter;
      curelem->analyze_Tail(packetpos,remainingSize);
      }

    }

  mAnalysisReady = true;
  }

bool Packet::match(Packet* otherPacket)
  {
  if (!mAnalysisReady ||!otherPacket->mAnalysisReady)
    {
    return false; // calling at a time match is not possible.
    }
  vector<Element *>::iterator iterSelf;
  vector<Element *>::iterator iterOther;
  
  for (iterSelf = mElems.begin(), iterOther = otherPacket->mElems.begin();
       iterSelf != mElems.end() && iterOther != otherPacket->mElems.end();
       iterSelf++, iterOther++) // Only need to match for as much as the own packet (matching packet) is defined, therefore looping on self.
    {
    Element* elemSelf = *iterSelf;
    Element* elemOther = *iterOther;

    // there will be a protective typeid check in the match of every element, so no need to do it here.
    if (!elemSelf->match(elemOther)) 
      {
      return false; // RETURN STATEMENT IN LOOP!
      }
    // If there is a match, make sure to copy the VarAssigns
    }  

  if (iterSelf !=mElems.end()) // More elements in self then in the packet to match against => no match
    {
    return false; // RETURN STATEMENT
    }

  // The packet was matched ==> copy fields to variables as needed
  for (iterSelf = mElems.begin(), iterOther = otherPacket->mElems.begin();
       iterSelf != mElems.end() && iterOther != otherPacket->mElems.end();
       iterSelf++, iterOther++) // Only need to match for as much as the own packet (matching packet) is defined, therefore looping on self.
    {
    Element* elemSelf = *iterSelf;
    Element* elemOther = *iterOther;

    elemSelf->playVarAssigns(elemOther);

    }
  
  return true;
  }

void Packet::push_back(Element* elem)
  {
  mElems.push_back(elem);
  }

void Packet::setPort(Port* outport)
  {
  mOutPort = outport;
  }

bool Packet::tryComplete(bool final)
  {
  if (mRawPacket != NULL && !mHasVar)
    {
    return true;
    }
 
  // Going from top to bottom, because the elements themselves will go from 
  // bottom to top and enherit from upper layers...
  vector<Element *>::reverse_iterator riter;

  // First copy vars of all elements before trying complete: all layers must have proper values before trying autocomplete
  for (riter = mElems.rbegin();riter != mElems.rend();riter++)
    {
    Element* elem= *riter;
    bool hasVar;
    hasVar= elem->copyVar(); // If (non const) variables were used, take value now
    mHasVar = mHasVar || hasVar;
    }

  bool complete=true;
  for (riter = mElems.rbegin();(riter != mElems.rend() && complete);riter++)
    {
    Element* elem= *riter;
    complete = elem->tryComplete(mElems);
    if (final && !complete)
      {
      throw Exception ("Failed to auto-complete all parameters of: " + elem->getString() + " : " + elem->whatsMissing());
      }
    }

  if (complete)
    {
    if ((mRawPacket == NULL) || mHasVar)
      {
      vector<Element *>::iterator iter;
      mRawSize=0;
      for (iter = mElems.begin();iter != mElems.end();iter++)
        {
        Element* elem= *iter;
        mRawSize += elem->getSize();
        mRawSize += elem->getTailSize();
        }

      if (mAllocSize < mRawSize) // allocate or re-allocate
        {
        if (mRawPacket != NULL) // case mHasVar : variable content
          {
          delete mRawPacket; // free the current buffer. We need a larger one
          }
        mRawPacket = new uchar[mRawSize]; // No further optimisation: this should be rare cases
        mAllocSize = mRawSize;
        }
      //  uchar temp[1500]; // tempory. Need a system to predict size, so that the packet can never be too small...
      uchar* curPos = mRawPacket;
      for (iter = mElems.begin();iter != mElems.end();iter++)
        {
        Element* elem= *iter;
        curPos = elem->copyTo(curPos);
        }

      vector<Element *>::reverse_iterator riter;
      for (riter = mElems.rbegin();riter != mElems.rend();riter++)
        {
        Element* elem= *riter;
        curPos = elem->copyTail(curPos);
        }
      if ((ulong) (curPos-mRawPacket) != mRawSize)
        {
        stringstream msg;
        msg << "Packet size is not matching the packet content: known size: " 
          << mRawSize << " vs. raw data size: " << (ulong) (curPos-mRawPacket) << "For packet: " << endl
          << getString();        
        msg << flush;
        throw Exception(msg.str());
        }
      }
    }

  mBinaryReady = complete;
  return complete;
  }

void Packet::sendTo(Port& outport) throw (Exception)
  {
  bool complete;
  try
    {
    complete = tryComplete(true);
    }
  catch (Exception e)
    {
    stringstream msg;
    msg << "While trying to send packet: " << endl
      << getString() 
      << "  Had following error: " << endl
      << "  " << e.what() << endl;
    throw Exception(msg.str());
    }
  if (complete) //final attempt
    {
    if (mShaper != NULL)
      {
      mShaper->shape(mRawSize);
      }
    outport.send(mRawPacket,mRawSize);
    }
  else
    {
    throw Exception("Could not autoguess all of the packet: " + getString()); // Should not get here, but have got a more clear exception above
    }
  }

void Packet::sendTo(Port* outport) throw (Exception)
  {
  sendTo(*outport);
  }

void Packet::send() throw (Exception)
  {
  if (mOutPort != NULL)
    {
    sendTo(mOutPort);
    }
  else
    {
    throw Exception("Trying to send a packet, but don't know on what port to send it");
    // Should not occur as port is a mandatory parameter in the xml parsing
    }
  }

void Packet::sendNoShaper() throw (Exception)
  {
  Shaper* shaper = mShaper;
  mShaper = NULL;
  if (mOutPort != NULL)
    {
    sendTo(mOutPort);
    }
  else
    {
    throw Exception("Trying to send a packet, but don't know on what port to send it");
    // Should not occur as port is a mandatory parameter in the xml parsing
    }
  mShaper = shaper;
  }

void Packet::play() throw (Exception)
  {
  send();
  }

ulong Packet::getRawSize()
  {
  if (!mBinaryReady)
    {
    if (tryComplete(false))
      {
      return mRawSize;
      }
    }
  if (!mBinaryReady)
    {
    return 0; // 0 value is used for packet not ready to send
    }
  else
    {
    return mRawSize;
    }
  
  }

vector<Element*>::iterator Packet::begin()
  {
  return mElems.begin();
  }

vector<Element*>::iterator Packet::end()
  {
  return mElems.end();
  }

string Packet::getString()
  {
  stringstream retval;
  retval << "<packet";
  if (mOutPort != NULL)
    {
    retval << " port=\"" << mOutPort->getId() << "\"";
    }
  retval << ">" << endl;
  vector<Element *>::iterator iter;
  for (iter = mElems.begin();iter != mElems.end();iter++)
    {
    Element* elem= *iter;
    retval << "  " << elem->getString() << endl;
    }

  retval << "</packet>" << endl;
  return retval.str();
  }

void Packet::setShaper(Shaper* shaper)
  {
  mShaper = shaper;
  }

Shaper* Packet::getShaper()
  {
  return mShaper;
  }


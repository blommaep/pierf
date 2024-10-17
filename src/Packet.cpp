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
#include "SignatureElem.hpp"
#include <sstream>
#include <typeinfo>

Packet::Packet()
  : mOutPort(NULL), mRawPacket(NULL), mRawSize(0), mAllocSize(0), mAuto(eAutoEnherit),mBinaryReady(false),mAnalysisReady(false), mHasVar(false), mShaper(NULL), mTxSignaturePos(NULL)
  {
  }

Packet::Packet(char* id)
  : mOutPort(NULL), mRawPacket(NULL), mRawSize(0), mAllocSize(0), mId(id),mBinaryReady(false),mAnalysisReady(false), mHasVar(false), mShaper(NULL), mTxSignaturePos(NULL)
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

void Packet::setRawPacket(uchar* rawPacket, ulong32 rawSize) noexcept(false)
  {
  if (mBinaryReady)
    {
    throw Exception("Adding new binary in a packet that has already one. Code bug?");
    }

  mRawPacket = rawPacket;
  mRawSize = rawSize;
  mBinaryReady = true;
  }

void Packet::analyse() noexcept(false)
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

  if (!(mRawSize > 0))
    {
    throw Exception("Trying to analyse a packet of size 0???");
    }

  uchar* packetpos = mRawPacket;
  ulong32 remainingSize = mRawSize;

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

//    curelem->analyze_Tail(packetpos,remainingSize); Not useful to do analyse_Tail on eth, so just skipping

    while (remainingSize > 0 && nextelem != NULL && succeed)
      {
      push_back(curelem);
      curelem = nextelem;
      succeed = curelem->analyze_Head(packetpos,remainingSize);
      if (succeed)
        {
        succeed = curelem->analyze_Tail(packetpos,remainingSize); // must take from the back of the packet
        }
      if (succeed)
        {
        nextelem = curelem->analyze_GetNextElem();

        // special trick to distinghuish signature (from raw)
        if (remainingSize >= 8 && nextelem == NULL)
          {
          if (*packetpos == 0xCD and *(packetpos+1) == 0x40)
            {
            nextelem = new SignatureElem;
            }
          }
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

    // analyse_Tail used to be done by reverse iteration, but not anymore. (Virtual optimization as there is currently no Element that requires analyse_Tail) Kept as a sample
//    vector<Element *>::reverse_iterator riter;

//    for (riter = mElems.rbegin();(riter != mElems.rend() && succeed);riter++)
//      {
//      curelem = *riter;
//      curelem->analyze_Tail(packetpos,remainingSize);
//      }

    }

  mAnalysisReady = true;
  }

bool Packet::match(Packet* otherPacket) 
  {
  if (!mAnalysisReady ||!otherPacket->mAnalysisReady)
    {
    return false; // calling at a time match is not possible.
    }

  // self is supposed to be the user defined "match" packet, other is the captured packet
  vector<Element *>::iterator iterSelf;
  vector<Element *>::iterator iterOther;
  
  for (iterSelf = mElems.begin(), iterOther = otherPacket->mElems.begin();
       iterSelf != mElems.end() && iterOther != otherPacket->mElems.end();
       iterSelf++, iterOther++) // Only need to match for as much as the own packet (matching packet) is defined, therefore looping on self.
    {
    Element* elemSelf = *iterSelf;
    Element* elemOther = *iterOther;

    // there will be a protective typeid check in the match of every element, so no need to do it here.
    elemSelf->copyVar(); // Will update value based on variables if applicable (value-state=eVar)
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

bool Packet::compare(Packet* otherPacket, bool matchByString) // assumed to be executed by captured packet, otherPacket is the user defined "match" Packet.
  {
  if (!mBinaryReady)
    {
    throw Exception("Trying to do analysis while no binary packet was available. Code bug?");
    }

  if (!(mRawSize > 0))
    {
    throw Exception("Trying to analyse a packet of size 0???");
    }

  vector<Element *>::iterator iterSelf;
  vector<Element *>::iterator iterOther;

  uchar* packetpos = mRawPacket;
  ulong32 remainingSize = mRawSize;
  
  
  for (iterOther = otherPacket->mElems.begin();
       iterOther != otherPacket->mElems.end();
       iterOther++ ) // Only need to match for as much as the own packet (matching packet) is defined, therefore looping on self.
    {
    Element* elemOther = *iterOther;
    Element* elemSelf = elemOther->getNewBlank();
    push_back(elemSelf); // must be deleted with the Packet, so as a first thing, add it to the stack

    // tbd: for Raw: make sure somehow that also if size is automatically set, it has a correct value and only that size is captured
    if (typeid(*elemOther) == typeid(Raw))
      {
      if (!matchByString) // for string match, size can only be determined by explicit configuration. For binary match, make sure to capture only the relevant size, also if it is not explicitly configured
        {
        Raw* rawSelf = (Raw*) elemSelf;
        Raw* rawOther = (Raw*) elemOther;
        rawSelf->copySize(rawOther); // must not only copy value, but also value state
        }
      }

    if (!elemSelf->analyze_Head(packetpos,remainingSize))
      {
      return false; // analysis of an element fails if remainingSize is too small... In that case, there can't be a match.
       // RETURN STATEMENT IN LOOP!
      }

    if (!elemSelf->analyze_Tail(packetpos,remainingSize))
      {
      return false; // analysis of an element fails if remainingSize is too small... In that case, there can't be a match.
       // RETURN STATEMENT IN LOOP!
      }

    // tbd: this must move after the analyzetail, cf. below note
    elemOther->copyVar(); // Will update value based on variables if applicable (value-state=eVar)
    if (!elemOther->match(elemSelf)) 
      {
      return false; // RETURN STATEMENT IN LOOP!
      }
    }


  // Match is considered SUCCESS !!! if reached here
  // Must now execute the variable assigns
  for (iterSelf = mElems.begin(), iterOther = otherPacket->mElems.begin();
       iterSelf != mElems.end() && iterOther != otherPacket->mElems.end();
       iterSelf++, iterOther++) // If reached here, both packets should actually have exactly same number and type of elements.
    {
    Element* elemSelf = *iterSelf;
    Element* elemOther = *iterOther;

    elemOther->playVarAssigns(elemSelf);
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
        if (typeid(*elem) == typeid(SignatureElem))
          {
          mTxSignaturePos = (ulong32*) (curPos-6);
          }
        }

      vector<Element *>::reverse_iterator riter;
      for (riter = mElems.rbegin();riter != mElems.rend();riter++)
        {
        Element* elem= *riter;
        curPos = elem->copyTail(curPos);
        }
      if ((ulong32) (curPos-mRawPacket) != mRawSize)
        {
        stringstream msg;
        msg << "Packet size is not matching the packet content: known size: " 
          << mRawSize << " vs. raw data size: " << (ulong32) (curPos-mRawPacket) << "For packet: " << endl
          << getString();        
        msg << flush;
        throw Exception(msg.str());
        }
      }
    }

  mBinaryReady = complete;
  return complete;
  }

void Packet::sendTo(Port& outport) noexcept(false)
  {
  bool complete;

  try
    {
    complete = tryComplete(true);
    }
  catch (Exception& e)
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
    if (mCounter != NULL)
      {
      mCounter->increment(1, mRawSize);
      }
    if (mTxSignaturePos != NULL)
      {
      ulong32 tmp = ntohl32(*mTxSignaturePos);
      tmp++;
      *mTxSignaturePos = htonl32(tmp);
      }
    }
  else
    {
    throw Exception("Could not autoguess all of the packet: " + getString()); // Should not get here, but have got a more clear exception above
    }
  }

void Packet::sendTo(Port* outport) noexcept(false)
  {
  sendTo(*outport);
  }

void Packet::send() noexcept(false)
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

void Packet::sendNoShaper() noexcept(false)
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

void Packet::play() noexcept(false)
  {
  send();
  }

ulong32 Packet::getRawSize()
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

string Packet::getElementsString() const
  {
  stringstream retval;
  vector<Element *>::const_iterator iter;
  for (iter = mElems.begin();iter != mElems.end();iter++)
    {
    Element* elem= *iter;
    retval << "  " << elem->getString() << endl;
    }
  retval << flush;
  return retval.str();
  }

string Packet::getString() const
  {
  stringstream retval;
  retval << "<packet";
  if (mOutPort != NULL)
    {
    retval << " port=\"" << mOutPort->getId() << "\"";
    }
  retval << ">" << endl;
  vector<Element *>::const_iterator iter;
  for (iter = mElems.begin();iter != mElems.end();iter++)
    {
    Element* elem= *iter;
    retval << "  " << elem->getString() << endl;
    }

  retval << "</packet>" << endl << flush;
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

void Packet::setCounter(Counter* counter)
  {
  mCounter = counter;
  }


// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "Udp.hpp"
#include "IpHdr.hpp"
#include "Ipv6.hpp"
#include <sstream>
#include <string.h>
#include <iostream> 
#include <typeinfo>

Udp::Udp()
  :mChecksumIpRetrieved(false)
  {
  mSourcePort.displayDecimal();
  mDestPort.displayDecimal();
  mLength.displayDecimal();
  }

void Udp::parseAttrib(const char** attr, AutoObject* parent, bool checkMandatory, bool storeAsString) throw (Exception)
  {
  char* autoStr=NULL;
  int i=0;
  while (attr[i] != NULL)
    {
    if (!strcmp(attr[i],"sourceport"))
      {
      i++;
      setSourcePort(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"destport"))
      {
      i++;
      setDestPort(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"length"))
      {
      i++;
      setLength(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"checksum"))
      {
      i++;
      setChecksum(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"auto"))
      {
      i++;
      autoStr = (char*) attr[i++];
      }
    else
      {
      throw Exception("Unexepected attribute: " + string(attr[i]) + " in <udp> tag.");
      i++;
      }
    }

  setOrEnheritAuto(autoStr,parent);
  if (!isAuto() && checkMandatory)
    {
    string missing = "";

    if (!mSourcePort.hasValue())
      {
      missing += " sourceport";
      }
    if (!mDestPort.hasValue())
      {
      missing += " destport";
      }
    if (!mLength.hasValue())
      {
      missing += " length";
      }

    if (missing != "")
      {
      throw Exception("Missing mandatory attributes to <udp> tag: " + missing);
      }
    }
  
  }

void Udp::setSourcePort(const char* sourcePort, bool storeAsString) throw (Exception)
  {
  try
    {
    mSourcePort.setManual(sourcePort, storeAsString);
    }
  catch (Exception e)
    {
    throw Exception("Source port invalid: " + string(e.what()));
    }
  }

void Udp::setDestPort(const char* destPort, bool storeAsString) throw (Exception)
  {
  try
    {
    mDestPort.setManual(destPort, storeAsString);
    }
  catch (Exception e)
    {
    throw Exception("Destination port invalid: " + string(e.what()));
    }
  }

void Udp::setLength(const char* length, bool storeAsString) throw (Exception)
  {
  try
    {
    mLength.setManual(length, storeAsString);
    }
  catch (Exception e)
    {
    throw Exception("UDP length invalid: " + string(e.what()));
    }
  }


void Udp::setChecksum(const char* checkSum, bool storeAsString) throw (Exception)
  {
  try
    {
    mChecksum.setManual(checkSum, storeAsString);
    }
  catch (Exception e)
    {
    throw Exception("Checksum invalid: " + string(e.what()));
    }
  }


string Udp::getString() 
  {
  stringstream retval;
  retval << "<udp ";
  
  if (mSourcePort.isPrintable())
    {
    retval << "sourceport=\"" << mSourcePort.getString() << "\" ";
    }
  if (mDestPort.isPrintable())
    {
    retval << "destport=\"" << mDestPort.getString() << "\" ";
    }
  if (mLength.isPrintable())
    {
    retval << "length=\"" << mLength.getString() << "\" ";
    }
  if (mChecksum.isPrintable())  
    {
    retval << "checksum=\"" << mChecksum.getString() << "\" ";
    }

  retval << " />" << flush;
  return retval.str();
  }

bool Udp::getString(string& stringval, const char* fieldName)
  {
  if (!strcmp(fieldName, "sourceport"))
    {
    return mSourcePort.getString(stringval);
    }  
  if (!strcmp(fieldName, "destport"))
    {
    return mDestPort.getString(stringval);
    }  
  if (!strcmp(fieldName, "length"))
    {
    return mLength.getString(stringval);
    }  
  if (!strcmp(fieldName, "checksum"))
    {
    return mChecksum.getString(stringval);
    }  
  return false;
  }

ulong Udp::getSize()
  {
  return 8; // See the spec: 5 x 32 bit + options + 2 bytes for the ethertype
  }

ulong Udp::getTailSize()
  {
  return 0;
  }

bool Udp::copyVar() throw (Exception)
  {
  bool copy = false;
  bool res;
  res = mSourcePort.copyVar(); 
  copy = copy || res;
  res = mDestPort.copyVar();
  copy = copy || res;
  res = mLength.copyVar();
  copy = copy || res;
  res = mChecksum.copyVar();
  copy = copy || res;
  return copy;
  }

uchar* Udp::copyTo(uchar* toPtr)
  {
  uchar* ckStart = toPtr;
  toPtr = mSourcePort.copyTo(toPtr);
  toPtr = mDestPort.copyTo(toPtr);
  toPtr = mLength.copyTo(toPtr);
  mChecksumPos = toPtr;
  *toPtr++ = 0x0; 
  *toPtr++ = 0x0; 
  // Inserting the checksum is delayed untill the copyTail is called, so that we can calculate it over the entire content. Here, we only add the header part checksum
  if (!mChecksum.isManual())
    {
    mChecksum.reset();
    mChecksum.addPreCarry(mIpPseudoHdrChecksum);
    mChecksum.addToSum(ckStart,toPtr);
    }
  mContentStart = toPtr;
  return toPtr;
  }

uchar* Udp::copyTail(uchar* toPtr)
  {
  if (!mChecksum.isManual())
    {
    mChecksum.addToSum(mContentStart,toPtr);
    mChecksum.calculateCarry();
    mChecksum.wasDefaulted();
    }
  mChecksum.copyTo(mChecksumPos);
  return toPtr;
  }

bool Udp::analyze_Head(uchar*& fromPtr, ulong& remainingSize)
  {
  if (!mSourcePort.analyze(fromPtr,remainingSize)) return false;
  if (!mDestPort.analyze(fromPtr,remainingSize)) return false;
  if (!mLength.analyze(fromPtr,remainingSize)) return false;
  if (!mChecksum.analyze(fromPtr,remainingSize)) return false;
  return true;
  }

bool Udp::analyze_Tail(uchar*& fromPtr, ulong& remainingSize)
  {
  return true;
  }

Element* Udp::analyze_GetNextElem()
  {
  return NULL; // Would need to look at port for higher layer protocols, but they are not currently supported
  }

bool Udp::match(Element* other)
  {
  if (typeid(*other) != typeid(Udp))
    {
    return false;
    }
  Udp* otherUdp = (Udp*) other;

  //isPrintable: manual or captured. Only those two kinds must be matched don't care about defaults or autos
  if (!mSourcePort.match(otherUdp->mSourcePort))
    {
    return false;
    }
  if (!mDestPort.match(otherUdp->mDestPort))
    {
    return false;
    }
  if (!mLength.match(otherUdp->mLength))
    {
    return false;
    }
  if (!mChecksum.match(otherUdp->mChecksum))
    {
    return false;
    }

  return true;
  }

bool Udp::checkComplete()
  {
  if (!mSourcePort.hasValue())
    {
    return false;
    }
  if (!mDestPort.hasValue())
    {
    return false;
    }
  if (!mLength.hasValue())
    {
    return false;
    }
  if (!mChecksum.isManual() && !mIpPseudoHdrChecksum.hasValue()) // If not manual, it is generated, but need to retrieve the ip pseudo header checksum during trycomplete...
    {
    return false;
    }

  return true;
  }

// important assumption: tryComplete for Udp layer is only called if the higher layers are complete!
bool Udp::tryComplete(ElemStack& stack)
  {
  if (mChecksum.isManual())
    {
    mChecksumIpRetrieved = true; // Nothing was retrieved, but no need to do so...
    }

  if (!mChecksumIpRetrieved || isAuto())
    {
  // even in non auto, go through the stack to find the ip layer: need it for checksum
    enum CompleteState {eIdle,eFoundIp,eFoundSelf,eDone};
    CompleteState state = eIdle;
    ushort length = 8; // start by counting the size of the udp header itself
    bool calcLength = !mLength.isManual(); //calculate if it hasn't been set
    mIpPseudoHdrChecksum.reset();
    mChecksumIpRetrieved = false;
    if (!isAuto())
      {
      calcLength=false;
      }
    vector<Element*>::iterator iter;
    IpHdr* ip=NULL;
    Ipv6* ipv6=NULL;
    for (iter=stack.begin();iter != stack.end();iter++)
      {
      Element* elem = *iter;
      if (state == eIdle)
        {
        if (typeid(*elem) == typeid(IpHdr))
          {
          ip = (IpHdr*) elem; // we (may) need ip for checksum calculation
          state = eFoundIp;
          }
        else if (typeid(*elem) == typeid(Ipv6))
          {
          ipv6 = (Ipv6*) elem; // we (may) need ip for checksum calculation
          state = eFoundIp;
          }
        }
      else if (state == eFoundIp) // pseudo header detection!!
        {
        if (elem == this)
          {
          state = eFoundSelf;
          if (ip != NULL)
            {
            ip->tryComplete(stack); // protocol may need to be enherited by IP, so, try to complete it now
            if (!mChecksumIpRetrieved)
              {
              mChecksumIpRetrieved = ip->addPseudoHeaderChecksum(mIpPseudoHdrChecksum);
              // If IP header has variables, this flag will also be false: must recalculate every time
              }
            }
          else if (ipv6 != NULL)
            {
            ipv6->tryComplete(stack); // protocol may need to be enherited by IP, so, try to complete it now
            if (!mChecksumIpRetrieved)
              {
              mChecksumIpRetrieved = ipv6->addPseudoHeaderChecksum(mIpPseudoHdrChecksum);
              // If IP header has variables, this flag will also be false: must recalculate every time
              }
            }
          }
        else
          {
          state = eIdle; // it was not the ip layer for this udp
          ip = NULL;
          ipv6 = NULL;
          }
        }
      else // in all other states, need to get the length for calculating contentlength
        {
        if (!elem->tryComplete(stack))
          {
          calcLength=false;
          }
        if (calcLength)
          {
          length += elem->getSize();
          length += elem->getTailSize();
          }
        }
      }

    if (calcLength)
      {
      mLength.setAuto(length);
      }
    }


  return checkComplete();
  }

string Udp::whatsMissing()
  {
  if (!checkComplete())
    {
    string missing;

    if (!mSourcePort.hasValue())
      {
      missing += "sourceport ";
      }
    if (!mDestPort.hasValue())
      {
      missing += "destport ";
      }
    if (!mLength.hasValue())
      {
      missing += "length ";
      }

    if (!mIpPseudoHdrChecksum.hasValue() && !mChecksum.isManual())
      {
      missing += "checksum ";
      }

    return "Failed to complete the field(s): " + missing;
    }

  return "";
  }

Element* Udp::getNewBlank()
  {
  Udp* udp = new Udp();
  return (Element*) udp;
  }

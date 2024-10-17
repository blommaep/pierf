// Copyright (c) 2006-2011, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "Ipv6.hpp"
#include "Icmp.hpp"
#include "Igmp.hpp"
#include "IgmpV2.hpp"
#include "IgmpV3.hpp"
#include "Udp.hpp"
#include "Tcp.hpp"
#include "VarContainer.hpp"
#include <sstream>
#include <string.h>
#include <typeinfo>


Ipv6::Ipv6()
  {
  mVersion.setDefault((uchar)6);
  mVersion.displayDecimal(); 
  mVersion.setOffset(4);
  mTrafficClass.setOffset(4);
  mTrafficClass.setDefault((uchar)0); // former dscp
  mFlowLabel.setOffset(4);
  mFlowLabel.setDefault((ulong32)0);
  mHopLimit.displayDecimal(); // ttl
  mPayloadLength.displayDecimal();
  }

void Ipv6::parseAttrib(const char** attr, AutoObject* parent, bool checkMandatory, bool storeAsString) noexcept(false)
  {
  char* autoStr=NULL;
  int i=0;
  while (attr[i] != NULL)
    {
    if (!strcmp(attr[i],"version"))
      {
      i++;
      mVersion.setManual(attr[i++], storeAsString);      
      }
    else if (!strcmp(attr[i],"trafficClass"))
      {
      i++;
      mTrafficClass.setManual(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"flowLabel"))
      {
      i++;
      mFlowLabel.setManual(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"payloadLength"))
      {
      i++;
      mPayloadLength.setManual(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"nextHeader"))
      {
      i++;
      mNextHeader.setManual(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"hopLimit"))
      {
      i++;
      mHopLimit.setManual(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"from"))
      {
      i++;
      mFromIp.setManual(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"to"))
      {
      i++;
      mToIp.setManual(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"auto"))
      {
      i++;
      autoStr = (char*) attr[i++];
      }
    else
      {
      throw Exception("Unexepected attribute: " + string(attr[i]) + " in <iphdr> tag.");
      i++;
      }
    }

  setOrEnheritAuto(autoStr,parent);

  if (!isAuto() && checkMandatory)
    {
    string missing;

    if (!mNextHeader.hasValue())
      {
      missing += "nextHeader ";
      }
    if (!mHopLimit.hasValue())
      {
      missing += "hopLimit ";
      }
    if (!mFromIp.hasValue())
      {
      missing += "'from' ";
      }
    if (!mToIp.hasValue()) 
      {
      missing += "'to' ";
      }

    if (missing != "")
      {
      throw Exception("Missing mandatory attributes to <iphdr> tag: " + missing);
      }
    }

  }

bool Ipv6::addPseudoHeaderChecksum(ChecksumIp& cksum)
  {
  if (!mFromIp.hasValue())
    {
    return false;
    }
  if (!mToIp.hasValue())
    {
    return false;
    }
  if (!mNextHeader.hasValue())
    {
    return false;
    }
  if (!mPayloadLength.hasValue())
    {
    return false;
    }
  mFromIp.addToSum(cksum);
  mToIp.addToSum(cksum);
  cksum.addToSum(htons((ushort)mNextHeader.getValue()));
  cksum.addToSum(htons(mPayloadLength.getValue()));
  if (!cksum.hasValue())
    {
    cksum.wasDefaulted();
    }

  // return false also if the value can change and hence must be recalculated next time
  if (mFromIp.isVar())
    {
    return false;
    }
  if (mToIp.isVar())
    {
    return false;
    }
  if (mNextHeader.isVar())
    {
    return false;
    }
  if (mPayloadLength.isVar())
    {
    return false;
    }
  return true; // Everything calculated and fixed
  }


string Ipv6::getString() 
  {
  stringstream retval;
  retval << "<ipv6 ";
  
  if (mFromIp.isPrintable())
    {
    retval << "from=\"" << mFromIp.getConfigString() << "\" ";
    }
  if (mToIp.isPrintable())
    {
    retval << "to=\"" << mToIp.getConfigString() << "\" ";
    }
  if (mTrafficClass.isPrintable())
    {
    retval << "trafficClass=\"" << mTrafficClass.getConfigString() << "\" ";
    }
  if (mVersion.isPrintable())
    {
    retval << " version=\"" << mVersion.getConfigString() << "\"";
    }
  if (mFlowLabel.isPrintable())
    {
    retval << " flowLabel=\"" << mFlowLabel.getConfigString() << "\"";
    }
  if (mPayloadLength.isPrintable() != 0)
    {
    retval << " payloadLength=\"" << mPayloadLength.getConfigString() << "\"";
    }
  if (mNextHeader.isPrintable())
    {
    retval << " nextHeader=\"" << mNextHeader.getConfigString() << "\"";
    }
  if (mHopLimit.isPrintable() != 0)
    {
    retval << " hopLimit=\"" << mHopLimit.getConfigString() << "\"";
    }

  if (hasVarAssigns())
    {
    retval << " >" << endl << getVarAssignsString();
    retval << "  </ipv6>";
    }
  else
    {
    retval << " />";
    }

  retval << flush;
  return retval.str();
  }


bool Ipv6::getString(string& stringval, const char* fieldName)
  {
  if (!strcmp(fieldName,"from"))
    {
    return mFromIp.getString(stringval);
    }
  else if (!strcmp(fieldName,"to"))
    {
    return mToIp.getString(stringval);
    }
  else if (!strcmp(fieldName,"version"))
    {
    return mVersion.getString(stringval);
    }
  else if (!strcmp(fieldName,"trafficClass"))
    {
    return mTrafficClass.getString(stringval);
    }
  else if (!strcmp(fieldName,"flowLabel"))
    {
    return mFlowLabel.getString(stringval);
    }
  else if (!strcmp(fieldName,"payloadLength"))
    {
    return mPayloadLength.getString(stringval);
    }
  else if (!strcmp(fieldName,"nextHeader"))
    {
    return mNextHeader.getString(stringval);
    }
  else if (!strcmp(fieldName,"hopLimit"))
    {
    return mHopLimit.getString(stringval);
    }
  return false;
  }

// tbd: everything from here

ulong32 Ipv6::getSize()
  {
  return 40; 
  }

ulong32 Ipv6::getTailSize()
  {
  return 0;
  }

bool Ipv6::copyVar() noexcept(false)
  {
  bool copy = false;
  bool res;
  res = mFromIp.copyVar();
  copy = copy || res;
  res = mToIp.copyVar();
  copy = copy || res;
  res = mVersion.copyVar();
  copy = copy || res;
  res = mTrafficClass.copyVar();
  copy = copy || res;
  res = mFlowLabel.copyVar();
  copy = copy || res;
  res = mPayloadLength.copyVar();
  copy = copy || res;
  res = mNextHeader.copyVar();
  copy = copy || res;
  res = mHopLimit.copyVar();
  copy = copy || res;
  return copy;
  }

uchar* Ipv6::copyTo(uchar* toPtr)
  {
  mVersion.copyTo(toPtr); // do not advance toPtr
  toPtr = mTrafficClass.copyTo(toPtr);
  toPtr = mFlowLabel.copyTo(toPtr);
  toPtr = mPayloadLength.copyTo(toPtr);
  toPtr = mNextHeader.copyTo(toPtr);
  toPtr = mHopLimit.copyTo(toPtr);
  toPtr = mFromIp.copyTo(toPtr);
  toPtr = mToIp.copyTo(toPtr);
  return toPtr;
  }

uchar* Ipv6::copyTail(uchar* toPtr)
  {
  return toPtr;
  }

bool Ipv6::analyze_Head(uchar*& fromPtr, ulong32& remainingSize)
  {
  if (remainingSize < 40)
    {
    return false;
    }

  if (!mVersion.analyze(fromPtr,remainingSize)) return false;
  if (!mTrafficClass.analyze(fromPtr,remainingSize)) return false;
  if (!mFlowLabel.analyze(fromPtr,remainingSize)) return false;
  if (!mPayloadLength.analyze(fromPtr,remainingSize)) return false; 
  if (!mNextHeader.analyze(fromPtr,remainingSize)) return false;
  if (!mHopLimit.analyze(fromPtr,remainingSize)) return false;
  if (!mFromIp.analyze(fromPtr,remainingSize)) return false;
  if (!mToIp.analyze(fromPtr,remainingSize)) return false;
  return true;
  }

bool Ipv6::analyze_Tail(uchar*& fromPtr, ulong32& remainingSize)
  {
  return true;
  }

Element* Ipv6::analyze_GetNextElem()
  {
  if (mNextHeader.hasValue())
    {
    uchar protocol = mNextHeader.getValue();
    switch (protocol)
      {
      case 0x01:
        return new Icmp();
        break;
      case 0x02:
        return new Igmp();
        break;
      case 0x06:
        return new Tcp();
        break;
      case 0x11:
        return new Udp();
        break;
      // to be updated with new protocols
      default:
        ;
      }
    }
  return NULL;
  }

bool Ipv6::checkComplete()
  {
  if (!mFromIp.hasValue())
    {
    return false;
    }
  if (!mToIp.hasValue())
    {
    return false;
    }
  if (!mVersion.hasValue())
    {
    return false;
    }
  if (!mTrafficClass.hasValue())
    {
    return false;
    }
  if (!mFlowLabel.hasValue())
    {
    return false;
    }
  if (!mPayloadLength.hasValue())
    {
    return false;
    }
  if (!mNextHeader.hasValue())
    {
    return false;
    }
  if (!mHopLimit.hasValue())
    {
    return false;
    }

  return true;
  }

// important assumption: tryComplete for Ipv6 layer is only called if the higher layers are complete!
bool Ipv6::tryComplete(ElemStack& stack) //tbd: complete when NDP/MLD is implemented
  {
  if (isAuto())
    {
    enum CompleteState {eIdle,eFoundSelf,eFoundIgmp,eDone};
    CompleteState state = eIdle;
    ushort contentLength = 0;
    bool calcContentLength = mPayloadLength.needsAuto(); //calculate if it hasn't been set
    vector<Element*>::iterator iter;
    for (iter=stack.begin();iter != stack.end();iter++)
      {
      Element* elem = *iter;
      if (state == eIdle)
        {
        if (elem == this)
          {
          state = eFoundSelf;
          }
        }
      else // in all other states, need to get the length for calculating contentlength
        {
        if (!elem->checkComplete() && (typeid(*elem) != typeid(Udp)) && (typeid(*elem) != typeid(Tcp)))
          // Udp has fixed header length and udp needs the IP packet size itself to complete the checksum
          // Tcp doesn't have fixed header lenght, but can calculate header lenght before it is complete
          {
          calcContentLength=false;
          }
        if (calcContentLength)
          {
          contentLength += elem->getSize();
          contentLength += elem->getTailSize();
          }

        if (state == eFoundSelf)
          {
          if (typeid(*elem) == typeid(Icmp)) // unlikely, IPv6 has NDP
            {
            if (!mNextHeader.hasValue())
              {
              mNextHeader.setAuto(0x01);
              }
            }
          else if (typeid(*elem) == typeid(Udp))
            {
            if (!mHopLimit.hasValue())
              {
              mNextHeader.setAuto(128);
              }
            if (!mNextHeader.hasValue())
              {
              mNextHeader.setAuto(0x11);
              }
            state = eDone;
            }
          else if (typeid(*elem) == typeid(Tcp))
            {
            if (!mHopLimit.hasValue())
              {
              mHopLimit.setAuto(128);
              }
            if (!mNextHeader.hasValue())
              {
              mNextHeader.setAuto(0x06);
              }
            state = eDone;
            }
          else
            {
            state= eDone; // expecting strict stack order
            }
          }
        }
      }

    if (calcContentLength)
      {
      mPayloadLength.setAuto(contentLength);
      }

    return checkComplete();
    }

  // case: not auto and not complete. should never come here
  return false;
  }

string Ipv6::whatsMissing()
  {
  if (!checkComplete())
    {
    string missing;

    if (!mFromIp.hasValue())
      {
      missing += "from ";
      }
    if (!mToIp.hasValue())
      {
      missing += "to ";
      }
    if (!mVersion.hasValue())
      {
      missing += "version ";
      }
    if (!mTrafficClass.hasValue())
      {
      missing += "trafficClass ";
      }
    if (!mFlowLabel.hasValue())
      {
      missing += "flowLabel ";
      }
    if (!mPayloadLength.hasValue())
      {
      missing += "payloadLength ";
      }
    if (!mNextHeader.hasValue())
      {
      missing += "nextHeader ";
      }
    if (!mHopLimit.hasValue())
      {
      missing += "hopLimit ";
      }

    return "Failed to complete the field(s): " + missing;
    }

  return "";
  }

bool Ipv6::match(Element* other)
  {
  if (typeid(*other) != typeid(Ipv6))
    {
    return false;
    }
  Ipv6* otherIp = (Ipv6*) other;

  if (!mFromIp.match(otherIp->mFromIp))
    {
    return false;
    }
  if (!mToIp.match(otherIp->mToIp))
    {
    return false;
    }
  if (!mVersion.match(otherIp->mVersion))
    {
    return false;
    }
  if (!mTrafficClass.match(otherIp->mTrafficClass))
    {
    return false;
    }
  if (!mFlowLabel.match(otherIp->mFlowLabel))
    {
    return false;
    }
  if (!mPayloadLength.match(otherIp->mPayloadLength))
    {
    return false;
    }
  if (!mNextHeader.match(otherIp->mNextHeader))
    {
    return false;
    }
  if (!mHopLimit.match(otherIp->mHopLimit))
    {
    return false;
    }

  return true;  
  
  }

Element* Ipv6::getNewBlank()
  {
  Ipv6* ipv6 = new Ipv6();
  return (Element*) ipv6;
  }

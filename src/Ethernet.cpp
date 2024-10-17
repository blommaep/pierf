// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "Ethernet.hpp"
#include "Arp.hpp"
#include "Vlan.hpp"
#include "IpHdr.hpp"
#include "Ipv6.hpp"
#include "IgmpV2.hpp"
#include "Igmp.hpp"

#include <iostream> // for cout and cin
#include <fstream>
#include <sstream>
#include <typeinfo>
#include "typeinfo"
#include <string.h>

Ethernet::Ethernet()
  {
  }

Ethernet::Ethernet(MacAddress& from, MacAddress& to) throw (Exception)
  : mFrom(from), mTo(to) {} ;


void Ethernet::parseAttrib(const char** attr, AutoObject* parent, bool checkMandatory, bool storeAsString) throw (Exception)
  {
  char* autoStr=NULL;
  int i=0;
  while (attr[i] != NULL)
    {
    if (!strcmp(attr[i],"from"))
      {
      i++;
      mFrom.setManual(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"to"))
      {
      i++;
      mTo.setManual(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"ethertype"))
      {
      i++;
      mEthertype.setManual(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"auto"))
      {
      i++;
      autoStr = (char*) attr[i++];
      }
    else
      {
      throw Exception("Unexepected attribute: " + string(attr[i]) + " in <eth> tag.");
      i++;
      }
    }

  setOrEnheritAuto(autoStr,parent);
  if (!isAuto() && checkMandatory)
    {
    string missing = "";
    if (!mFrom.hasValue())
      {
      missing += "from ";
      }
    if (!mTo.hasValue())
      {
      missing += "to";
      }

    if (missing != "")
      {
      throw Exception("Missing mandatory attributes to <eth> tag: " + missing);
      }
    }
  }

void Ethernet::setFrom(MacAddress& from)
  {
  mFrom = from;
  }

void Ethernet::setTo(MacAddress& to)
  {
  mTo = to;
  }

void Ethernet::setFrom(const char* from)
  {
  mFrom.setManual(from, false);
  }

void Ethernet::setTo(const char* to)
  {
  mTo.setManual(to, false);
  }

void Ethernet::setEthertype(const char* ethertype) throw (Exception)
  {
  mEthertype.setManual(ethertype, false);
  }

bool Ethernet::copyVar() throw (Exception)
  {
  bool copy = false;
  bool res;
  res = mFrom.copyVar();
  copy = copy || res;
  res = mTo.copyVar();
  copy = copy || res;
  res = mEthertype.copyVar(); // Higher layer protocol
  copy = copy || res;
  return copy;
  }

bool Ethernet::hasVar()
  {
  if (mFrom.isVar()) return true;
  if (mTo.isVar()) return true;
  if (mEthertype.isVar()) return true;
  return false;
  }

uchar* Ethernet::copyTo(uchar* toPtr)
  {
  toPtr = mTo.copyTo(toPtr);
  toPtr = mFrom.copyTo(toPtr);
  toPtr = mEthertype.copyTo(toPtr);
  return toPtr;
  }

uchar* Ethernet::copyTail(uchar* toPtr)
  {
  return toPtr;
  }

bool Ethernet::analyze_Head(uchar*& fromPtr, ulong32& remainingSize)
  {
  if (!mTo.analyze(fromPtr,remainingSize)) return false;
  if (!mFrom.analyze(fromPtr,remainingSize)) return false;
  if (!mEthertype.analyze(fromPtr,remainingSize)) return false;
  return true;
  }

bool Ethernet::analyze_Tail(uchar*& fromPtr, ulong32& remainingSize)
  {
  return true;
  }

Element* Ethernet::analyze_GetNextElem()
  {
  if (mEthertype.hasValue())
    {
    ushort ethertype = mEthertype.getValue();
    if (ethertype == 0x0800)
      {
      return new IpHdr();
      }
    if (ethertype == 0x86DD)
      {
      return new Ipv6();
      }
    else if (ethertype == 0x0806)
      {
      return new Arp();
      }
    else if (ethertype == 0x8100)
      {
      return new Vlan();
      }
    }
  return NULL;
  }

bool Ethernet::match(Element* other)
  {
  if (typeid(*other) != typeid(Ethernet))
    {
    return false;
    }
  Ethernet* otherEth = (Ethernet*) other;

    {
    if (!mFrom.match(otherEth->mFrom))
      {
      return false;
      }
    if (!mTo.match(otherEth->mTo))
      {
      return false;
      }
    if (!mEthertype.match(otherEth->mEthertype))
      {
      return false;
      }
    }
  return true;  
  }

string Ethernet::getString()
  {
  stringstream retval;
  retval << "<eth ";
  if (mFrom.isPrintable())
    {
    retval << "from=\"" << mFrom.getConfigString() << "\" ";
    }
  if (mTo.isPrintable())
    {
    retval << "to=\"" << mTo.getConfigString() << "\" ";
    }
  if (mEthertype.isPrintable())
    {
    retval << "ethertype=\"" << mEthertype.getConfigString() << "\"";
    }

  if (hasVarAssigns())
    {
    retval << ">" << endl << getVarAssignsString();
    retval << "  </eth>";
    }
  else
    {
    retval << "/>";
    }
  
  retval << flush;
  return retval.str();
  }

bool Ethernet::getString(string& stringval, const char* fieldName)
  {
  if (!strcmp(fieldName, "from"))
    {
    return mFrom.getString(stringval);
    }
  if (!strcmp(fieldName, "to"))
    {
    return mTo.getString(stringval);
    }
  if (!strcmp(fieldName, "ethertype"))
    {
    return mEthertype.getString(stringval);
    }
  return false;
  }

ulong32 Ethernet::getSize()
  {
  return 14; // 2 x mac address + ethertype
  }

ulong32 Ethernet::getTailSize()
  {
  return 0;
  }

bool Ethernet::checkComplete()
  {
  return (mFrom.hasValue()) && (mTo.hasValue() && mEthertype.hasValue());
  }

bool Ethernet::tryComplete(ElemStack& stack)
  {
  if (isAuto())
    {
    enum CompleteState {eIdle,eFoundSelf,eFoundIp,eFoundIpv6,eFoundIgmp,eDone};
    CompleteState state = eIdle;
    vector<Element*>::iterator iter;
    for (iter=stack.begin();((iter != stack.end()) && (state != eDone));iter++)
      {
      Element* elem = *iter;
      if (state == eIdle)
        {
        if (elem == this)
          {
          state = eFoundSelf;
          }
        }
      else if (state == eFoundSelf)
        {
        if (typeid(*elem) == typeid(Arp))
          {
          Arp* arp = (Arp*) elem;
          if (mFrom.needsAuto())
            {
            mFrom.autoCopy(arp->getSourceMac()); // may still be incomplete if the arp's one was incomplete
            }
          if (mTo.needsAuto())
            {
            mTo.autoCopy(arp->getDestMac());
            }
          if (!mEthertype.hasValue())
            {
            mEthertype.setAuto(0x0806);
            }
          }
        else if (typeid(*elem) == typeid(Vlan))
          { // vlan irrelevant vs. upper layers auto guessing
          if (mEthertype.needsAuto())
            {
            Vlan* vlan = (Vlan*) elem;
            mEthertype.setAuto(vlan->getEthertype());
            }
          }
        else if (typeid(*elem) == typeid(IpHdr))
          { 
          if (!mEthertype.hasValue())
            {
            mEthertype.setAuto(0x0800);
            }
          state = eFoundIp;
          }
        else if (typeid(*elem) == typeid(Ipv6))
          { 
          if (!mEthertype.hasValue())
            {
            mEthertype.setAuto(0x86DD);
            }
//          state = eFoundIpv6;
          }
        else
          {
          state= eDone; // expecting strict stack order
          }
        }
      else if (state ==eFoundIp || state == eFoundIgmp)
        {
        if (typeid(*elem) == typeid(IgmpV2))
          {
          IgmpV2* igmp = (IgmpV2*) elem;
          if (mTo.needsAuto())
            {
            mTo.setAddressFromMcastIp(igmp->getDestIp().getAddress());
            }
          }
        else if (typeid(*elem) == typeid(Igmp))
          {
          state= eFoundIgmp; // expecting strict stack order
          }
        else
          {
          state= eDone; // expecting strict stack order
          }
        }
      }

    return checkComplete();
    }

  return false; // Should never come here
  }

string Ethernet::whatsMissing()
  {
  if (!checkComplete())
    {
    string missing;
    if (!mFrom.hasValue())
      {
      missing = "from ";
      }
    if (!mTo.hasValue())
      {
      missing = "to ";
      }
    if (!mEthertype.hasValue())
      {
      missing = "ethertype ";
      }
    return "Failed to complete the field(s): " + missing;
    }
  return "";  
  }

Element* Ethernet::getNewBlank()
  {
  Ethernet* ethernet = new Ethernet();
  return (Element*) ethernet;
  }


// Copyright (c) 2006-2011, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "Vlan.hpp"

#include <iostream> // for cout and cin
#include <fstream>
#include <sstream>
#include <string.h>
#include <stdlib.h>
#include "Arp.hpp"
#include "IpHdr.hpp"
#include "Ipv6.hpp"
#include "VarContainer.hpp"
#include <typeinfo>

void Vlan::stringToVal(const char* inString, int insertBefore) throw (Exception)
  {
  unsigned int i,j;

  vector<ushort>::iterator insertPos;
  
  if (insertBefore > (int) mVlans.size())
    {
    insertBefore = -1; // insert at the end
    }
  else if (insertBefore > 0)
    {
    insertPos = mVlans.begin();
    while (insertBefore)
      {
      insertPos++;
      insertBefore--;
      if (insertPos==mVlans.end())
        {
        throw Exception("Trying to insert a vlan in a stack with lower size. Insert position requested is: " + intToString(insertBefore));
        }
      }    
    }

  enum VlanState {eTag, ePriority, eReady, eDone};
  VlanState readWhat = eTag;
  char val[5];
  ushort newVlan;
 
  i=0; j=0;

  if (inString[i] == 0)
    {
    throw Exception("New vlan addition requested with empty string");
    readWhat=eDone; // TBD: throw exception instead
    }
  
  while (readWhat != eDone)
    {
    if (((inString[i] == '/') || (inString[i] == ':') || (inString[i] == 0)) && (readWhat == eTag))
      {
      val[j] = 0; // Make 0 terminated string
      newVlan = atoi(val);
      if (newVlan > 4095)
        {
        throw Exception("Vlan id too high: " + intToString(newVlan));
        }
      if (j == 0)
        {
        throw Exception("Empty vlan in vlan stack");
        }
      j = 0; // empty string again
      if (inString[i] == '/')
        {
        readWhat = ePriority; // will read priority next
        }
      else // no priority => push back the tag
        {
        readWhat = eReady;
        }
      }
    else if ((inString[i] == ':') || (inString[i] == 0) && (readWhat == ePriority))
      {
      val[j] = 0; // Make 0 terminated string
      ushort priority = atoi(val);
      if (priority > 7)
        {
        throw Exception("Vlan priority too high: " + intToString(priority));
        }
      priority = priority * 8192; // shift 13 bits
      newVlan += priority;
      j=0;
      readWhat = eReady;
      }
    else if ((inString[i] >= '0') && (inString[i] <= '9'))
      {
      val[j] = inString[i];
      j++;
      if (j >=5) // syntax error: vlan must be < 4094
        {
        throw Exception ("Vlan tag or vlan priority specified is > 9999");
        }
      }
    else
      {
      const char* wrongchar = inString+i;
      throw Exception("Unexpected character in vlan string: '" + string(wrongchar,1) + "'");
      }

    if (readWhat == eReady)
      {
      newVlan = htons(newVlan);
      if (insertBefore <0)
        {
        mVlans.push_back(newVlan);
        }
      else
        {
        insertPos = mVlans.insert(insertPos,newVlan); //Assuming insertPos remains valid and keeps pointing to the element to be inserted before.
        insertPos++; // next vlan tag to be inserted after the one inserted now.
        }
      readWhat = eTag;
      }

    if (inString[i] == 0)
      {
      readWhat=eDone;
      }    
    else
      {
      i++;
      }
    }
  }
  

Vlan::Vlan()
  {
  mVlanEthertype.setAuto(0x8100);
  mVlanString.setInputChar(true);
  }

Vlan::Vlan(char* inString) throw (Exception)
  {
  mVlanEthertype.setAuto(0x8100);
  stringToVal(inString,-1);
  mVlanString.setInputChar(true);
  }

Vlan::Vlan(string& inString) throw (Exception)
  {
  mVlanEthertype.setAuto(0x8100);
  stringToVal(inString.c_str(),-1);
  mVlanString.setInputChar(true);
  }

Vlan::~Vlan()
  {
  }

void Vlan::parseAttrib(const char** attr, AutoObject* parent, bool checkMandatory, bool storeAsString) throw (Exception)
  {
  char* autoStr=NULL;
  int i=0;
  while (attr[i] != NULL)
    {
    if (!strcmp(attr[i],"stack"))
      {
      i++;
      setStack(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"bodyEthertype"))
      {
      i++;
      mBodyEthertype.setManual(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"vlanEthertype"))
      {
      i++;
      mVlanEthertype.setManual(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"auto"))
      {
      i++;
      autoStr = (char*) attr[i++];
      }
    else
      {
      throw Exception("Unexepected attribute: " + string(attr[i]) + " in <vlans> tag.");
      i++;
      }
    }

  setOrEnheritAuto(autoStr,parent);
  
  if (!isAuto() && checkMandatory)
    {
    if (mVlans.size() == 0)
      {
      throw Exception("Missing mandatory attribute to tag <vlans> tag: 'stack'");
      }
    }
  }

string Vlan::getStackString()
  {
  stringstream retval;

  vector<ushort>::iterator iter;
  for ( iter = mVlans.begin(); iter != mVlans.end(); )  
    {
    ushort vlanId = *iter & 0xFFF; // remove priority by mask
    ushort priority = *iter / 0x2000 ;
    retval << vlanId;
    if (priority != 0)
      {
      retval << "/" << priority;
      }

    iter++; // !! iteration increment inside the loop

    if (iter != mVlans.end())
      {
      retval << ":"; // separation sign for next vlan in the stack
      }
    }

  retval << flush;
  return retval.str();
  }

string Vlan::getString()
  {
  stringstream retval;
  retval << "<vlans";

  if (mVlanString.size() > 0)
    {
    retval << " stack=\"" << mVlanString.getConfigString() << "\"";
    }

  if (mVlanEthertype.isPrintable())
    {
    retval << " vlanEthertype=\"" << mVlanEthertype.getConfigString() << "\"";
    }

  if (mBodyEthertype.isPrintable())
    {
    retval << " bodyEthertype=\"" << mBodyEthertype.getConfigString() << "\"";
    }
 
  if (hasVarAssigns())
    {
    retval << " >" << endl << getVarAssignsString();
    retval << "  </vlans>";
    }
  else
    {
    retval << " />";
    }

  retval << flush;
  return retval.str();
  }

bool Vlan::getString(string& stringval, const char* fieldName)
  {
  if (!strcmp(fieldName, "stack"))
    {
    return mVlanString.getString(stringval);
    }
  if (!strcmp(fieldName, "bodyEthertype"))
    {
    return mBodyEthertype.getString(stringval);
    }
  if (!strcmp(fieldName, "vlanEthertype"))
    {
    return mVlanEthertype.getString(stringval);
    }
  return false;
  }

void Vlan::setStack(const char* stack, bool storeAsString) throw (Exception)
  {
  mVlanString.setManual(stack, storeAsString);
  if (!mVlanString.isVar() && !mVlanString.isString())
    {
    mVlans.clear();
    stringToVal(mVlanString.getString().c_str(),-1);
    }
  }

void Vlan::setBodyEthertype(const char* ethertype) throw (Exception)
  {
  mBodyEthertype.setManual(ethertype, false);
  }

void Vlan::setVlanEthertype(const char* ethertype) throw (Exception)
  {
  mVlanEthertype.setManual(ethertype, false);
  }

ushort Vlan::getEthertype() // Gives the ethertype of the tag, so what must be used by the lower layer (normally ethernet itself)
  {
  if (mVlans.size() >0)
    {
    return mVlanEthertype.getValue();
    }
  else
    {
    return mBodyEthertype.getValue();
    }
  }

ulong32 Vlan::getSize()
  {
  return mVlans.size()*4; // 4 bytes per vlan tag
  }

ulong32 Vlan::getTailSize()
  {
  return 0;
  }

bool Vlan::copyVar() throw (Exception)
  {
  bool copy = false;
  bool res;
  res = mBodyEthertype.copyVar();
  copy = copy || res;
  res = mVlanEthertype.copyVar();
  copy = copy || res;
  if (mVlanString.copyVar())
    {
    mVlans.clear();
    stringToVal(mVlanString.getString().c_str(),-1);
    copy = true;
    }
  return true;
  }

uchar* Vlan::copyTo(uchar* toPtr)
  {
  vector<ushort>::iterator iter;

  for ( iter = mVlans.begin(); iter != mVlans.end(); )  // !! Increment within the loop
    {
    ushort *tmpPtr = (ushort*) toPtr;
    *tmpPtr = *iter;
    tmpPtr++;
    toPtr = (uchar*) tmpPtr;

    iter++; // !!!!!!!! INCREMENT WITHIN THE LOOP (only way to get an efficient and correct check

    if (iter != mVlans.end()) // another vlan following...
      {
      toPtr = mVlanEthertype.copyTo(toPtr);
      }
    else // the ethertype must be the ether type of what is inside the vlan 
      {
      toPtr = mBodyEthertype.copyTo(toPtr);
      }
    }
  
  return toPtr;
  }

uchar* Vlan::copyTail(uchar* toPtr)
  {
  return toPtr;
  }

bool Vlan::analyze_Head(uchar*& fromPtr, ulong32& remainingSize)
  {
  bool state = true;

  while (state)
    {
    if (remainingSize < 4) // too small
      {
      return false;
      }
    ushort vlan = ntohs(* (ushort*) fromPtr);
    mVlans.push_back(vlan);
    fromPtr += 2;
    remainingSize -= 2;
    ushort ethertype = ntohs(* (ushort*) fromPtr);
    if (ethertype != mVlanEthertype.getValue())
      {
      state = false;
      if (!mBodyEthertype.analyze(fromPtr,remainingSize)) return false;
      }
    else
      {
      fromPtr += 2;
      remainingSize -= 2;
      }
    }

  //tbd: clear vlan string before adding
  mVlanString.addString(getStackString());
  mVlanString.wasCaptured();
  return true; // Will also return true if no vlan was found. This is a bit strange, but the vlan layer allows any number of vlans (stacked). For now, keeping this concept.
  }

bool Vlan::analyze_Tail(uchar*& fromPtr, ulong32& remainingSize)
  {
  return true;
  }

Element* Vlan::analyze_GetNextElem()
  {
  if (mBodyEthertype.hasValue())
    {
    ushort ethertype = mBodyEthertype.getValue();
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

bool Vlan::checkComplete()
  {
  if (!mVlanString.hasValue())
    {
    return false;
    }
  if (!mBodyEthertype.hasValue())
    {
    return false;
    }
  return mVlanEthertype.hasValue();
  } 

bool Vlan::tryComplete(ElemStack& stack)
  {
  if (checkComplete())
    {
    return true;
    }
  else if (isAuto() && (!mBodyEthertype.hasValue()))
    {
    // to be completed!
    enum CompleteState {eIdle,eFoundSelf,eDone};
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
          if (!mBodyEthertype.hasValue())
            {
            mBodyEthertype.setAuto(0x0806);
            }
          }
        else if (typeid(*elem) == typeid(Vlan))
          { 
          if (!mBodyEthertype.hasValue())
            {
            Vlan* vlan = (Vlan*) elem;
            mBodyEthertype.setAuto(vlan->getEthertype());
            }
          }
        else if (typeid(*elem) == typeid(IpHdr))
          { 
          if (!mBodyEthertype.hasValue())
            {
            mBodyEthertype.setAuto(0x0800);
            }
          state = eDone;
          }
        else if (typeid(*elem) == typeid(Ipv6))
          { 
          if (!mBodyEthertype.hasValue())
            {
            mBodyEthertype.setAuto(0x86DD);
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
  return checkComplete(); // Vlan is always complete. Nothing to learn from other layers
  } 

string Vlan::whatsMissing()
  {
  return "";
  }

bool Vlan::match(Element* other)
  {
  if (typeid(*other) != typeid(Vlan))
    {
    return false;
    }
  Vlan* otherVlan = (Vlan*) other;

  if (!mBodyEthertype.match(otherVlan->mBodyEthertype))
    {
    return false;
    }
  if (!mVlanEthertype.match(otherVlan->mVlanEthertype))
    {
    return false;
    }

  if (!mVlanString.match(otherVlan->mVlanString))
    {
    // If match was needed but not found, we still try a vlan per vlan compare, just in case the string would not be exaclty ok
    if (mVlans.size() != otherVlan->mVlans.size())
      {
      return false;
      }

    //vlan stack cannot be sometihing auto/default. It must be set manually, so always check
    vector<ushort>::iterator iterSelf;
    vector<ushort>::iterator iterOther;
    iterOther = otherVlan->mVlans.begin();
    for ( iterSelf = mVlans.begin(); iterSelf != mVlans.end(); iterSelf++, iterOther++)  // !! Increment within the loop
      {
      if (*iterSelf != *iterOther)
        {
        return false;
        }
      }
    }

  return true;  
  }

Element* Vlan::getNewBlank()
  {
  Vlan* vlan = new Vlan();
  return (Element*) vlan;
  }

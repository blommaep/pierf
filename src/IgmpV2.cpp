// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "IgmpV2.hpp"
#include "VarContainer.hpp"

#include <sstream>
#include <typeinfo> 
#include <string.h> 


//// OVERLOADED CLASSES FOR DEDICATED FIELD INTERPRETATIONS ////

void IgmpV2Type::setManualFromValue(const char* inString) noexcept(false)
  {
  if (!strcmp(inString,"join") || !strcmp(inString,"report"))
    {
    Bitfield8::setManualFromValue((uchar)0x16);
    }
  else if (!strcmp(inString,"leave"))
    {
    Bitfield8::setManualFromValue((uchar)0x17);
    }
  else if (!strcmp(inString,"query"))
    {
    Bitfield8::setManualFromValue((uchar)0x11);
    }
  else
    {
    if (!strncmp(inString,"0x",2)) // hex Value
      {
      Bitfield8::setManualFromValue(inString); // allows setting a wrong msg type
      }
    else
      {
      throw Exception("Igmp v2 message type unknown: " + string(inString));
      }
    }
  }

void IgmpV2Type::setAuto(const char* inString) noexcept(false)
  {
  if (!strcmp(inString,"join") || !strcmp(inString,"report"))
    {
    Bitfield8::setAuto((uchar)0x16);
    }
  else if (!strcmp(inString,"leave"))
    {
    Bitfield8::setAuto((uchar)0x17);
    }
  else if (!strcmp(inString,"query"))
    {
    Bitfield8::setAuto((uchar)0x11);
    }
  else
    {
    if (!strncmp(inString,"0x",2)) // hex Value
      {
      Bitfield8::setAuto(inString); // allows setting a wrong msg type
      }
    else
      {
      throw Exception("Igmp v2 message type unknown: " + string(inString));
      }
    }
  }

void IgmpV2Type::setDefault(const char* inString) noexcept(false)
  {
  if (!strcmp(inString,"join") || !strcmp(inString,"report"))
    {
    Bitfield8::setDefault((uchar)0x16);
    }
  else if (!strcmp(inString,"leave"))
    {
    Bitfield8::setDefault((uchar)0x17);
    }
  else if (!strcmp(inString,"query"))
    {
    Bitfield8::setDefault((uchar)0x11);
    }
  else
    {
    if (!strncmp(inString,"0x",2)) // hex Value
      {
      Bitfield8::setDefault(inString); // allows setting a wrong msg type
      }
    else
      {
      throw Exception("Igmp v2 message type unknown: " + string(inString));
      }
    }
  }

string IgmpV2Type::getStringFromBinary() const
  {
  stringstream retval;

  switch (mData)
    {
    case 0x16:
      retval << "report";
      break;
    case 0x17:
      retval << "leave";
      break;
    case 0x11:
      retval << "query";
      break;
    default:
      retval << Bitfield8::getString();
    }

  retval << flush;
  return retval.str();
  }

bool IgmpV2Type::getStringFromBinary(string& stringval) const
  {
  if (hasValue())
    {
    stringval = getString();
    return true;
    }
  return false;
  }

/////////////////// IGMP V2 ITSELF ///////////////////////

IgmpV2::IgmpV2()
  {
  mResponseTime.setDefault((uchar)0);
  mResponseTime.displayDecimal();
  }

void IgmpV2::parseAttrib(const char** attr, AutoObject* parent, bool checkMandatory, bool storeAsString) noexcept(false)
  {
  char* autoStr=NULL;
  int i=0;
  while (attr[i] != NULL)
    {
    if (!strcmp(attr[i],"version"))
      {
      i++;
      i++; // script the tag and the value
      }
    else if (!strcmp(attr[i],"type"))
      {
      i++;
      setMsgType(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"to"))
      {
      i++;
      mMcastIp.setManual(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"responsetime"))
      {
      i++;
      mResponseTime.setManual(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"checksum"))
      {
      i++;
      mChecksum.setManual(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"auto"))
      {
      i++;
      autoStr = (char*) attr[i++];
      }
    else
      {
      throw Exception("Unexepected attribute: " + string(attr[i]) + " in <igmp> tag.");
      i++;
      }
    }


  setOrEnheritAuto(autoStr,parent);
  if (!isAuto() && checkMandatory)
    {
    string missing = "";

    if (!mMsgType.hasValue())
      {
      missing += "type ";
      }
    if (!mMcastIp.hasValue())
      {
      missing += "to ";
      }

    throw Exception("Missing mandatory attributes to <igmp> tag: " + missing);
    }

  }

void IgmpV2::setMsgType(const char* msgTypeStr, bool storeAsString) noexcept(false)
  {
  mMsgType.setManual(msgTypeStr, storeAsString);
  if (mMsgType.getValue() == 0x11)
    {
    mResponseTime.setDefault((uchar)0x64);
    }
  }

void IgmpV2::setMcastIp(const char* mcastIpStr) noexcept(false)
  {
  mMcastIp.setManual(mcastIpStr, false);
  }

void IgmpV2::setChecksumValue(const char* checksum) noexcept(false)
  {
  mChecksum.setManual(checksum, false);
  }

void IgmpV2::setResponseTime(const char* responseTime)
  {
  mResponseTime.setManual(responseTime, false);
  }

string IgmpV2::getTypeString()
  {
  return mMsgType.getString();
  }

string IgmpV2::getString()
  {
  stringstream retval;
  retval << "<igmp version=\"2\"";
  
  if (mMsgType.isPrintable())
    {  
    retval << " type=\"";
    retval << getTypeString();
    retval << "\"";

    if (mResponseTime.isPrintable()) // default for query
      {
      retval << " responsetime=\"" << mResponseTime.getConfigString();
      }
    
    retval << "\"";
    }

  if (mMcastIp.isPrintable())
    {
    retval << " to=\"" << mMcastIp.getConfigString() << "\"";
    }

  if (mChecksum.isPrintable())
    {
    retval << " checksum=\"" << mChecksum.getConfigString() << "\"";
    }
 
  if (hasVarAssigns())
    {
    retval << ">" << endl << getVarAssignsString();
    retval << "  </igmp>";
    }
  else
    {
    retval << "/>";
    }
 
  retval << flush;
  return retval.str();
  }

bool IgmpV2::getString(string& stringval, const char* fieldName)
  {
  if (!strcmp(fieldName, "version"))
    {
    stringval = "2";
    return true;
    }
  if (!strcmp(fieldName, "type"))
    {
    if (mMsgType.hasValue())
      {
      stringval = getTypeString();
      return true;
      }
    return false;
    }
  if (!strcmp(fieldName, "to"))
    {
    return mMcastIp.getString(stringval);
    }
  if (!strcmp(fieldName, "responsetime"))
    {
    return mResponseTime.getString(stringval);
    }
  if (!strcmp(fieldName, "checksum"))
    {
    return mChecksum.getString(stringval);
    }

  return false;
  }

ulong32 IgmpV2::getSize()
  {
  return 8;
  }

ulong32 IgmpV2::getTailSize()
  {
  return 0;
  }

bool IgmpV2::copyVar() noexcept(false)
  {
  bool copy = false;
  bool res;
  res = mMsgType.copyVar();
  copy = copy || res;
  res = mResponseTime.copyVar();
  copy = copy || res;
  res = mMcastIp.copyVar();
  copy = copy || res;
  res = mChecksum.copyVar();
  copy = copy || res;
  return copy;
  }

uchar* IgmpV2::copyTo(uchar* toPtr)
  {
  uchar* ckStart = toPtr; // start of checksum
  toPtr = mMsgType.copyTo(toPtr);
  toPtr = mResponseTime.copyTo(toPtr);

  // Keep space for checksum
  uchar* ckLoc = toPtr;
  *toPtr++ = 0;  
  *toPtr++ = 0;  

  toPtr = mMcastIp.copyTo(toPtr);

  if (!mChecksum.isManual())
    {
    mChecksum.calculate(ckStart,toPtr);
    }
  mChecksum.copyTo(ckLoc);
  return toPtr;
  }

uchar* IgmpV2::copyTail(uchar* toPtr)
  {
  return toPtr;
  }

bool IgmpV2::analyze_Head(uchar*& fromPtr, ulong32& remainingSize)
  {
  if (!mMsgType.analyze(fromPtr,remainingSize)) return false;
  if (!mResponseTime.analyze(fromPtr,remainingSize)) return false;
  if (!mChecksum.analyze(fromPtr,remainingSize)) return false;
  if (!mMcastIp.analyze(fromPtr,remainingSize)) return false;
  return true;
  }

bool IgmpV2::analyze_Tail(uchar*& fromPtr, ulong32& remainingSize)
  {
  return true;
  }

Element* IgmpV2::analyze_GetNextElem()
  {
  return NULL;
  }

bool IgmpV2::checkComplete()
  {
  return true; // nothing follows IGMP, so nothing to complete
  }

bool IgmpV2::tryComplete(ElemStack& stack)
  {
  return true; // nothing follows IGMP, so nothing to complete
  }

string IgmpV2::whatsMissing()
  {
  return "";
  }

IpAddress IgmpV2::getDestIp()
  {
  if (mMsgType.getValue() == 0x11) // query
    {
    if (mMcastIp.getAddress() == 0) //general query
      {
      IpAddress temp;
      temp.setAuto("224.0.0.01");
      return temp;
      }
    }
  return mMcastIp;
  }

bool IgmpV2::match(Element* other)
  {
  if (typeid(*other) != typeid(IgmpV2))
    {
    return false;
    }
  IgmpV2* otherIgmp = (IgmpV2*) other;

  if (!mMsgType.match(otherIgmp->mMsgType))
    {
    return false;
    }
  if (!mResponseTime.match(otherIgmp->mResponseTime))
    {
    return false;
    }
  if (!mMcastIp.match(otherIgmp->mMcastIp))
    {
    return false;
    }
  if (!mChecksum.match(otherIgmp->mChecksum))
    {
    return false;
    }

  return true;  
  }

Element* IgmpV2::getNewBlank()
  {
  IgmpV2* igmpV2 = new IgmpV2();
  return (Element*) igmpV2;
  }

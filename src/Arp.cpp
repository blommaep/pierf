// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "Arp.hpp"

#include <iostream> // for cout and cin
#include <fstream>
#include <sstream>
#include "VarContainer.hpp"
#include <string.h>
#include <typeinfo>

//// OVERLOADED CLASSES FOR DEDICATED FIELD INTERPRETATIONS ////

void ArpType::setManualFromValue(const char* inString) noexcept(false)
  {
  if (!strncmp(inString,"0x",2))
    {
    setManualFromValue(inString);
    }
  else if (!strcmp(inString,"req"))
    {
    setManualFromValue((ushort) 1);
    }
  else if (!strcmp(inString,"rep"))
    {
    setManualFromValue((ushort) 2);
    }
  else
    {
    throw Exception("Invalid value for attribute 'type' of tag <arp>");
    }
  }

void ArpType::setAuto(const char* inString) noexcept(false)
  {
  if (!strncmp(inString,"0x",2))
    {
    setAuto(inString);
    }
  else if (!strcmp(inString,"req"))
    {
    setAuto((ushort) 1);
    }
  else if (!strcmp(inString,"rep"))
    {
    setAuto((ushort) 2);
    }
  else
    {
    throw Exception("Invalid value for attribute 'type' of tag <arp>");
    }
  }

void ArpType::setDefault(const char* inString) noexcept(false)
  {
  if (!strncmp(inString,"0x",2))
    {
    setDefault(inString);
    }
  else if (!strcmp(inString,"req"))
    {
    setDefault((ushort) 1);
    }
  else if (!strcmp(inString,"rep"))
    {
    setDefault((ushort) 2);
    }
  else
    {
    throw Exception("Invalid value for attribute 'type' of tag <arp>");
    }
  }

string ArpType::getStringFromBinary() const
  {
  string retval;
  if (hasValue())
    {
    switch (mData)
      {
      case 1:
        retval = "req";
        break;
      case 2:
        retval = "rep";
        break;
      }
    }
  return retval;
  }

bool ArpType::getStringFromBinary(string& stringval) const
  {
  if (hasValue())
    {
    stringval = getString();
    return true;
    }
  return false;  
  }




/////////////////// IP HDR ITSELF ///////////////////////

Arp::Arp()
  {
  }

void Arp::parseAttrib(const char** attr, AutoObject* parent, bool checkMandatory, bool storeAsString) noexcept(false)
  {
  char* autoStr=NULL;
  int i=0;
  while (attr[i] != NULL)
    {
    if (!strcmp(attr[i],"fromMac"))
      {
      i++;
      mSourceMac.setManual(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"toMac"))
      {
      i++;
      mDestMac.setManual(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"fromIp"))
      {
      i++;
      mSourceIp.setManual(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"toIp"))
      {
      i++;
      mDestIp.setManual(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"type"))
      {
      i++;
      setType(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"auto"))
      {
      i++;
      autoStr = (char*) attr[i++];
      }
    else
      {
      throw Exception("Unexepected attribute: " + string(attr[i]) + " in <arp> tag.");
      i++;
      }
    }

  setOrEnheritAuto(autoStr, parent);

  if (!isAuto() && checkMandatory)
    {
    string missing = "";

    if (!mSourceMac.hasValue())
      {
      missing += "fromMac ";
      }
    if (!mSourceIp.hasValue())
      {
      missing += "fromIp ";
      }
    if (!mDestIp.hasValue())
      {
      missing += "toIp ";
      }
    if (mArpType.getValue() == 2 && !mDestMac.hasValue())
      {
      missing += "toMac";
      }

    if (missing != "")
      {
      }
    throw Exception ("Missing mandatory attributes to <arp> tag: " + missing);
    }
  }

void Arp::setType(const char* type, bool storeAsString) noexcept(false)
  {
  mArpType.setManual(type, storeAsString);

  if (!mDestMac.isManual())
    {
    if (mArpType.getValue() == 1)
      {
      mDestMac.setDefault("FF:FF:FF:FF:FF:FF");
      }
    }
  }

void Arp::setSourceMac(const char* sourceMac) noexcept(false)
  {
  mSourceMac.setManual(sourceMac, false);
  }

void Arp::setDestMac(const char* destMac) noexcept(false)
  {
  mDestMac.setManual(destMac, false);
  }

void Arp::setSourceIp(const char* sourceIp) noexcept(false)
  {
  mSourceIp.setManual(sourceIp, false);
  }

void Arp::setDestIp(const char* destIp) noexcept(false)
  {
  mDestIp.setManual(destIp, false);
  }

string Arp::getTypeString()
  {
  return mArpType.getString();
  }

string Arp::getString()
  {
  stringstream retval;
  retval << "<arp";
 
  if (mArpType.isPrintable())
    {
    retval << " type=\"" << mArpType.getConfigString() << "\"";
    }

  if (mSourceMac.isPrintable())
    {
    retval << " fromMac=\"" << mSourceMac.getConfigString() << "\"";
    }
  if (mDestMac.isPrintable())
    {
    retval << " toMac=\"" << mDestMac.getConfigString() << "\"";
    }
  if (mSourceIp.isPrintable())
    {
    retval << " fromIp=\"" << mSourceIp.getConfigString() << "\"";
    }
  if (mDestIp.isPrintable())
    {
    retval << " toIp=\"" << mDestIp.getConfigString() << "\"";
    }
   
  if (hasVarAssigns())
    {
    retval << " >" << endl << getVarAssignsString();
    retval << "  </arp>";
    }
  else
    {
    retval << " />";
    }
 
  retval << flush;
  return retval.str();
  }

bool Arp::getString(string& stringval, const char* fieldName)
  {
  if (!strcmp(fieldName, "fromMac"))
    {
    return mSourceMac.getString(stringval);
    }
  if (!strcmp(fieldName, "toMac"))
    {
    return mDestMac.getString(stringval);
    }
  if (!strcmp(fieldName, "fromIp"))
    {
    return mSourceIp.getString(stringval);
    }
  if (!strcmp(fieldName, "toIp"))
    {
    return mDestIp.getString(stringval);
    }
  if (!strcmp(fieldName, "type"))
    {
    if (mArpType.hasValue())
      {
      stringval = getTypeString();
      return true;
      }
    else
      {
      return false;
      }
    }
  return false;
  }

ulong32 Arp::getSize()
  {
  return 28; // feel free to count: 8 bytes header + 12 bytes for two mac@ + 8 bytes for 2 IP@
  }

ulong32 Arp::getTailSize()
  {
  return 0;
  }

bool Arp::copyVar() noexcept(false)
  {
  bool copy=false;
  bool res;
  res = mArpType.copyVar();
  copy = copy || res;
  res = mSourceMac.copyVar();
  copy = copy || res;
  res = mDestMac.copyVar();
  copy = copy || res;
  res = mSourceIp.copyVar();
  copy = copy || res;
  res = mDestIp.copyVar();
  copy = copy || res;
  return copy;
  }

unsigned char* Arp::copyTo(unsigned char* toPtr)
  {
  *(toPtr++)=0x0;  // Hw Type: Ethernet
  *(toPtr++)=0x01;
  *(toPtr++)=0x08; // Protocol Type: IP
  *(toPtr++)=0x00;
  *(toPtr++)=0x06; // Hw size: 6 (mac@ length)
  *(toPtr++)=0x04; // Protocol size : 4 (IP@ length)
  toPtr = mArpType.copyTo(toPtr);  // Opcode
  toPtr = mSourceMac.copyTo(toPtr);
  toPtr = mSourceIp.copyTo(toPtr);
  if (mArpType.getValue() == 1 && !mDestMac.isManual()) // Arp request
    {
    *(toPtr++) = 0x0; // target Mac => unknown (it is the arps function to ask it)
    *(toPtr++) = 0x0;
    *(toPtr++) = 0x0;
    *(toPtr++) = 0x0;
    *(toPtr++) = 0x0;
    *(toPtr++) = 0x0;
    }
  else // reply
    {
    toPtr = mDestMac.copyTo(toPtr);
    }
  toPtr = mDestIp.copyTo(toPtr);
  return toPtr;
  }

uchar* Arp::copyTail(uchar* toPtr)
  {
  return toPtr;
  }

bool Arp::analyze_Head(uchar*& fromPtr, ulong32& remainingSize)
  {
  if (remainingSize < 6)
    {
    return false;
    }
  if (*fromPtr++ != 0x0) return false;  // Hw Type: Ethernet
  if (*fromPtr++ != 0x01) return false;
  if (*fromPtr++ != 0x08) return false; // Protocol Type: IP
  if (*fromPtr++ != 0x00) return false;
  if (*fromPtr++ != 0x06) return false; // Hw size: 6 (mac@ length)
  if (*fromPtr++ != 0x04) return false; // Protocol size : 4 (IP@ length)

  remainingSize -= 6;

  if (!mArpType.analyze(fromPtr,remainingSize)) return false;

  if (!mSourceMac.analyze(fromPtr,remainingSize)) return false;
  if (!mSourceIp.analyze(fromPtr,remainingSize)) return false;
  if (!mDestMac.analyze(fromPtr,remainingSize)) return false; // Analyze it, whatever it may be. Should be FF:FF... for arp request
  if (!mDestIp.analyze(fromPtr,remainingSize)) return false;
  
  return true;
  }

bool Arp::analyze_Tail(uchar*& fromPtr, ulong32& remainingSize)
  {
  return true;
  }

Element* Arp::analyze_GetNextElem()
  {
  return NULL;
  }

bool Arp::checkComplete()
  {
  return true; 
  }

bool Arp::tryComplete(ElemStack& stack)
  {
  return true; // End element, nothing to complete
  }

string Arp::whatsMissing()
  {
  return "";
  }

const MacAddress& Arp::getSourceMac()
  {
  return mSourceMac;
  }

const MacAddress& Arp::getDestMac()
  {
  return mDestMac;
  }

bool Arp::match(Element* other)
  {
  if (typeid(*other) != typeid(Arp))
    {
    return false;
    }
  Arp* otherArp = (Arp*) other;

  if (!mArpType.match(otherArp->mArpType))
    {
    return false;
    }
  if (!mSourceMac.match(otherArp->mSourceMac))
    {
    return false;
    }
  if (!mDestMac.match(otherArp->mDestMac))
    {
    return false;
    }
  if (!mSourceIp.match(otherArp->mSourceIp))
    {
    return false;
    }
  if (!mDestIp.match(otherArp->mDestIp))
    {
    return false;
    }

  return true;  
  }

Element* Arp::getNewBlank()
  {
  Arp* arp =  new Arp();
  return (Element*) arp;
  }

// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "Icmp.hpp"

#include <sstream>
#include "VarContainer.hpp"
#include <string.h>
#include <typeinfo>


//// OVERLOADED CLASSES FOR DEDICATED FIELD INTERPRETATIONS ////

uchar IcmpType::stringToVal(const char* strType) throw (Exception)
  {
  uchar val;
  if (!strcmp(strType,"echoReply"))
    {
    val = 0;
    }
  else if (!strcmp(strType,"echoRequest"))
    {
    val = 8;
    }
  else if (!strcmp(strType,"destinationUnreachable"))
    {
    val = 3;
    }
  else if (!strcmp(strType,"sourceQuench"))
    {
    val = 4;
    }
  else if (!strcmp(strType,"redirect"))
    {
    val = 5;
    }
  else if (!strcmp(strType,"routerAdvertisement"))
    {
    val = 9;
    }
  else if (!strcmp(strType,"routerSolicitation"))
    {
    val = 10;
    }
  else if (!strcmp(strType,"timeExceeded"))
    {
    val = 11;
    }
  else if (!strcmp(strType,"parameterProblem"))
    {
    val = 12;
    }
  else if (!strcmp(strType,"timestampRequest"))
    {
    val = 13;
    }
  else if (!strcmp(strType,"timestampReply"))
    {
    val = 14;
    }
  else if (!strcmp(strType,"informationRequest"))
    {
    val = 15;
    }
  else if (!strcmp(strType,"informationReply"))
    {
    val = 16;
    }
  else if (!strcmp(strType,"addressMaskRequest"))
    {
    val = 17;
    }
  else if (!strcmp(strType,"addressMaskReply"))
    {
    val = 18;
    }
  else if (!strcmp(strType,"traceroute"))
    {
    val = 30;
    }
  else if (!strcmp(strType,"conversionError"))
    {
    val = 31;
    }
  else if (!strcmp(strType,"domainNameRequest"))
    {
    val = 37;
    }
  else if (!strcmp(strType,"domainNameReply"))
    {
    val = 38;
    }
  else 
    {
    int temp = textToLong(strType);
    if (temp < 0 || temp > 0xFF)
      {
      throw Exception("Invalid input string for what should be a 1 byte integer value");
      }
    val = (uchar) temp;
    }
  
  return val;  
  }

void IcmpType::setManualFromValue(const char* inString) throw (Exception)
  {
  Bitfield8::setManualFromValue(stringToVal(inString));
  }

void IcmpType::setAuto(const char* inString) throw (Exception)
  {
  Bitfield8::setAuto(stringToVal(inString));
  }

void IcmpType::setDefault(const char* inString) throw (Exception)
  {
  Bitfield8::setDefault(stringToVal(inString));
  }

string IcmpType::getStringFromBinary() const
  {
  stringstream retval;
  if (hasValue()) 
    {
    switch (mData)
      {
      case 0:
        retval << "echoReply";
        break;
      case 8:
        retval << "echoRequest";
        break;
      case 3:
        retval << "destinationUnreachable";
        break;
      case 4:
        retval << "sourceQuench";
        break;
      case 5:
        retval << "redirect";
        break;
      case 9:
        retval << "routerAdvertisement";
        break;
      case 10:
        retval << "routerSolicitation";
        break;
      case 11:
        retval << "timeExceeded";
        break;
      case 12:
        retval << "parameterProblem";
        break;
      case 13:
        retval << "timestampRequest";
        break;
      case 14:
        retval << "timestampReply";
        break;
      case 15:
        retval << "informationRequest";
        break;
      case 16:
        retval << "informationReply";
        break;
      case 17:
        retval << "addressMaskRequest";
        break;
      case 18:
        retval << "addressMaskReply";
        break;
      case 30:
        retval << "traceroute";
        break;
      case 31:
        retval << "conversionError";
        break;
      case 37:
        retval << "domainNameRequest";
        break;
      case 38:
        retval << "domainNameReply";
        break;
      default:
        retval << Bitfield8::getString();
      }
    }
  retval << flush;
  return retval.str();
  }

bool IcmpType::getStringFromBinary(string& stringval) const
  {
  if (hasValue())
    {
    stringval = getString();
    return true;
    }
  return false;
  }

/////////////////// ICMP ITSELF ///////////////////////

Icmp::Icmp()
  {
  }

void Icmp::parseAttrib(const char** attr, AutoObject* parent, bool checkMandatory, bool storeAsString) throw (Exception)
  {
  char* autoStr=NULL;
  int i=0;
  while (attr[i] != NULL)
    {
    if (!strcmp(attr[i],"type"))
      {
      i++;
      setType(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"code"))
      {
      i++;
      setCode(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"checksum"))
      {
      i++;
      setChecksum(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"identifier"))
      {
      i++;
      setId(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"sequencenr"))
      {
      i++;
      setSequenceNr(attr[i++]);
      }
    else if (!strcmp(attr[i],"offset"))
      {
      i++;
      setOffset(attr[i++]);
      }
    else if (!strcmp(attr[i],"nexthopMtu"))
      {
      i++;
      setNexthopMtu(attr[i++]);
      }
    else if (!strcmp(attr[i],"ipaddress"))
      {
      i++;
      setIpaddress(attr[i++]);
      }
    else if (!strcmp(attr[i],"advertisementCount"))
      {
      i++;
      setAdvertisementCount(attr[i++]);
      }
    else if (!strcmp(attr[i],"lifetime"))
      {
      i++;
      setLifetime(attr[i++]);
      }
    else if (!strcmp(attr[i],"pointer"))
      {
      i++;
      setPointer(attr[i++]);
      }
    else if (!strcmp(attr[i],"auto"))
      {
      i++;
      autoStr = (char*) attr[i++];
      }
    else
      {
      throw Exception("Unexepected attribute: " + string(attr[i]) + " in <icmp> tag.");
      i++;
      }
    }

  setOrEnheritAuto(autoStr,parent);

  // tbd : different checks on the other fields depending on the type...
  if (!isAuto() && checkMandatory)
    {
    string missing;

    if (!mType.hasValue())
      {
      missing += "type ";
      }

    if (!isAuto()) // manual => all mandatory
      {
      if (missing != "")
        {
        throw Exception("Missing mandatory attributes to <icmp> tag: " + missing);
        }
      }
    }
  }

void Icmp::setType(const char* strType, bool storeAsString) throw (Exception)
  {
  try
    {
    mType.setManual(strType, storeAsString);
    }
  catch (Exception e)
    {
    throw Exception("Type value invalid: " + string(e.what()));
    }

  // setting the fields of the variable field, depending on the type
  // this assures proper feedback to the user, what field he forgot to specify
  switch (mType.getValue())
    {
    case 0: // echo request/reply
    case 8:
    case 13: // timestamp request/reply
    case 14:
    case 15: // information request/reply
    case 16:
    case 17: // address mask request/reply
    case 18:
    case 37: // domain name request/reply
    case 38:
      mCode.setDefault((uchar)0);
      mSpecificData.setShort("identifier", 0, (ushort) 0, FlexField32::eHowDefault);
      mSpecificData.setShort("sequencenr", 1, (ushort) 0, FlexField32::eHowDefault);
      break;
    case 3: // destination unreachable
      mSpecificData.setShort("unused", 0, (ushort) 0, FlexField32::eHowDefault);
      mSpecificData.setShort("nexthopMtu", 1, (ushort) 0, FlexField32::eHowUndef);
      break;
    case 10: // router solicitation
      mCode.setDefault((uchar)0); // fallthrough explicitly used here
    case 4: // source quench
    case 11: // time exceeded
      mSpecificData.setLong("unused", (ulong) 0, FlexField32::eHowDefault);
      break;
    case 5: // redirect
      mSpecificData.setLong("ipaddress", (ulong) 0, FlexField32::eHowUndef);
      break;
    case 9: // router advertisement
      mCode.setDefault((uchar)0);
      mSpecificData.setChar("advertisementCount", 0, (uchar) 0, FlexField32::eHowUndef);
      mSpecificData.setChar("addressEntrySize", 1, (uchar) 2, FlexField32::eHowDefault);
      mSpecificData.setShort("lifetime", 1, (ushort) 0, FlexField32::eHowUndef);
      break;
    case 12: // Parameter problem
      mSpecificData.setChar("pointer", 0, (uchar) 0, FlexField32::eHowUndef);
      mSpecificData.setChar("unused", 1, (uchar) 2, FlexField32::eHowDefault);
      mSpecificData.setShort("unused", 1, (ushort) 0, FlexField32::eHowDefault);
      break;
      
    case 30: // traceroute
      mSpecificData.setShort("identifier", 0, (ushort) 0, FlexField32::eHowDefault);
      mSpecificData.setShort("unused", 1, (ushort) 0, FlexField32::eHowDefault);
      break;
    case 31: // conversion error
      mSpecificData.setLong("offset", (ulong)  0, FlexField32::eHowUndef);
      break;
    }
  }

void Icmp::setCode(const char* strCode, bool storeAsString) throw (Exception)
  {
  try
    {
    mCode.setManual(strCode, storeAsString);
    }
  catch (Exception e)
    {
    throw Exception("Code value invalid: " + string(e.what()));
    }
  }

void Icmp::setChecksum(const char* strChecksum, bool storeAsString) throw (Exception)
  {
  try
    {
    mChecksum.setManual(strChecksum, storeAsString);
    }
  catch (Exception e)
    {
    throw Exception("Checksum value invalid: " + string(e.what()));
    }
  }

void Icmp::setId(const char* strId, bool storeAsString) throw (Exception)
  {
  if (storeAsString)
    {
    // tbd: currently FlexField not really supporting strings and match
    }
  else
    {
    switch (mType.getValue())
      {
      case 0: // echo request/reply
      case 8:
      case 13: // timestamp request/reply
      case 14:
      case 15: // information request/reply
      case 16:
      case 17: // address mask request/reply
      case 18:
      case 30: // traceroute 
      case 37: // domain name request/reply
      case 38:
        break;
      default:
        throw Exception("Identifier specified for an icmp type that does not support it: " + intToString(mType.getValue()));
      }
    try
      {
      mSpecificData.setShort("identifier", 0, strId, FlexField32::eHowManual);
      }
    catch (Exception e)
      {
      throw Exception("Id value invalid: " + string(e.what()));
      }
    }
  }

void Icmp::setSequenceNr(const char* strSeq) throw (Exception)
  {
  switch (mType.getValue())
    {
    case 0: // echo request/reply
    case 8:
    case 13: // timestamp request/reply
    case 14:
    case 15: // information request/reply
    case 16:
    case 17: // address mask request/reply
    case 18:
    case 37: // domain name request/reply
    case 38:
      break;
    default:
      throw Exception("Sequencenr specified for an icmp type that does not support it: " + intToString(mType.getValue()));
    }
  try
    {
    mSpecificData.setShort("sequencenr", 1, strSeq, FlexField32::eHowManual);
    }
  catch (Exception e)
    {
    throw Exception("Sequencenr value invalid: " + string(e.what()));
    }
  }

void Icmp::setOffset(const char* strOffset) throw (Exception)
  {
  switch (mType.getValue())
    {
    case 31: // conversion error
      break;
    default:
      throw Exception("Offset specified for an icmp type that does not support it: " + intToString(mType.getValue()));
    }
  try
    {
    mSpecificData.setLong("offset", strOffset, FlexField32::eHowManual);
    }
  catch (Exception e)
    {
    throw Exception("Offset value invalid: " + string(e.what()));
    }
  }

void Icmp::setNexthopMtu(const char* strMtu) throw (Exception)
  {
  switch (mType.getValue())
    {
    case 3: // destination unreachable
      break;
    default:
      throw Exception("NexthopMtu specified for an icmp type that does not support it: " + intToString(mType.getValue()));
    }
  try
    {
    mSpecificData.setShort("nexthopMtu", 1, strMtu, FlexField32::eHowManual);
    }
  catch (Exception e)
    {
    throw Exception("Next hop MTU value invalid: " + string(e.what()));
    }
  }

void Icmp::setIpaddress(const char* strIp) throw (Exception)
  {
  switch (mType.getValue())
    {
    case 5: // redirect
      break;
    default:
      throw Exception("Ipaddress specified for an icmp type that does not support it: " + intToString(mType.getValue()));
    }
  try
    {
    mSpecificData.setLong("ipaddress", strIp, FlexField32::eHowManual);
    }
  catch (Exception e)
    {
    throw Exception("IP Address value invalid: " + string(e.what()));
    }
  }

void Icmp::setAdvertisementCount(const char* strCount) throw (Exception)
  {
  switch (mType.getValue())
    {
    case 9: // router advertisement
      break;
    default:
      throw Exception("Advertisemnt count specified for an icmp type that does not support it: " + intToString(mType.getValue()));
    }
  try
    {
    mSpecificData.setChar("advertisementCount", 0, strCount, FlexField32::eHowManual);
    }
  catch (Exception e)
    {
    throw Exception("Advertisement count value invalid: " + string(e.what()));
    }
  }

void Icmp::setAddressEntrySize(const char* strSize) throw (Exception)
  {
  switch (mType.getValue())
    {
    case 9: // router advertisement
      break;
    default:
      throw Exception("Address Entry Size specified for an icmp type that does not support it: " + intToString(mType.getValue()));
    }
  try
    {
    mSpecificData.setChar("addressEntrySize", 1, strSize, FlexField32::eHowManual);
    }
  catch (Exception e)
    {
    throw Exception("Address Entry Size value invalid: " + string(e.what()));
    }
  }

void Icmp::setLifetime(const char* strLifetime) throw (Exception)
  {
  switch (mType.getValue())
    {
    case 9: // router advertisement
      break;
    default:
      throw Exception("Lifetime specified for an icmp type that does not support it: " + intToString(mType.getValue()));
    }
  try
    {
    mSpecificData.setShort("lifetime", 1, strLifetime, FlexField32::eHowManual);
    }
  catch (Exception e)
    {
    throw Exception("Lifetime value invalid: " + string(e.what()));
    }
  }

void Icmp::setPointer(const char* strPointer) throw (Exception)
  {
  switch (mType.getValue())
    {
    case 12: // Parameter problem
      break;
    default:
      throw Exception("Pointer (icmp) specified for an icmp type that does not support it: " + intToString(mType.getValue()));
    }
  try
    {
    mSpecificData.setChar("pointer", 0, strPointer, FlexField32::eHowManual);
    }
  catch (Exception e)
    {
    throw Exception("ICMP Pointer value invalid: " + string(e.what()));
    }
  }

string Icmp::getTypeString()
  {
  return mType.getString();
  }

string Icmp::getString()
  {
  stringstream retval;
  retval << "<icmp";
  if (mType.isPrintable())
    {
    retval << " type=\"";
    retval << getTypeString();
    retval << "\"";
    }

  if (mCode.isPrintable())
    {
    retval << " code=\"" << mCode.getString() << "\"";
    }

  if (mChecksum.isPrintable())
    {
    retval << " checksum=\"" << mChecksum.getString() << "\"";
    }
  
  retval << mSpecificData.getString();
  retval << " />" << flush;
  return retval.str();
  }

bool Icmp::getString(string& stringval, const char* fieldName)
  {
  if (!strcmp(fieldName, "type"))
    {
    if (mType.hasValue())
      {
      stringval = getTypeString();
      return true;
      }
    else
      {
      return false;
      }
    }
  else if (!strcmp(fieldName, "code"))
    {
    return mCode.getString(stringval);
    }
  else if (!strcmp(fieldName, "checksum"))
    {
    return mChecksum.getString(stringval);
    }
  else
    {
    return mSpecificData.getString(stringval, fieldName);
    }
  return false; // execution should never reach here
  }

ulong Icmp::getSize()
  {
  return 8;
  }

ulong Icmp::getTailSize()
  {
  return 0;
  }

bool Icmp::copyVar() throw (Exception)
  {
  bool copy = false;
  bool res;
  res = mType.copyVar();
  copy = copy || res;
  res = mCode.copyVar();
  copy = copy || res;
  res = mChecksum.copyVar();
  copy = copy || res;
  res = mSpecificData.copyVar();
  copy = copy || res;
  return copy;
  }

uchar* Icmp::copyTo(uchar* toPtr)
  {
  uchar* ckStart = toPtr;
  toPtr = mType.copyTo(toPtr);
  toPtr = mCode.copyTo(toPtr);
  mChecksumPos = toPtr; // position to insert checksum
  *toPtr++ = 0;
  *toPtr++ = 0;
  toPtr = mSpecificData.copyTo(toPtr);
  mContentStart = toPtr; // used for checksum calculation
  if (!mChecksum.isManual())
    {
    mChecksum.reset(); // always clear again
    mChecksum.addToSum(ckStart,toPtr);
    }
  return toPtr;
  }

uchar* Icmp::copyTail(uchar* toPtr)
  {
  if (!mChecksum.isManual())
    {
    mChecksum.addToSum(mContentStart,toPtr);
    mChecksum.calculateCarry();
    }  
  mChecksum.copyTo(mChecksumPos);
  return toPtr;
  }

bool Icmp::analyze_Head(uchar*& fromPtr, ulong& remainingSize)
  {
  if (!mType.analyze(fromPtr,remainingSize)) return false;
  if (!mCode.analyze(fromPtr,remainingSize)) return false;
  if (!mChecksum.analyze(fromPtr,remainingSize)) return false;
  if (!mSpecificData.analyze(fromPtr,remainingSize)) return false;

  // Set the proper names for the specificData fields
  switch (mType.getValue())
    {
    case 0: // echo request/reply
    case 8:
    case 13: // timestamp request/reply
    case 14:
    case 15: // information request/reply
    case 16:
    case 17: // address mask request/reply
    case 18:
    case 37: // domain name request/reply
    case 38:
      mSpecificData.setShort("identifier", 0, mSpecificData.getShort(0), FlexField32::eHowCaptured);
      mSpecificData.setShort("sequencenr", 1, mSpecificData.getShort(1), FlexField32::eHowCaptured);
      break;
    case 3: // destination unreachable
      mSpecificData.setShort("unused", 0, (ushort) 0, FlexField32::eHowUndef);
      mSpecificData.setShort("nexthopMtu", 1, mSpecificData.getShort(1), FlexField32::eHowCaptured);
      break;
    case 10: // router solicitation
    case 4: // source quench
    case 11: // time exceeded
      mSpecificData.setLong("unused", (ulong) 0, FlexField32::eHowUndef);
      break;
    case 5: // redirect
      mSpecificData.setLong("ipaddress", mSpecificData.getLong(), FlexField32::eHowCaptured);
      break;
    case 9: // router advertisement
      mSpecificData.setChar("advertisementCount", 0, mSpecificData.getChar(0), FlexField32::eHowCaptured);
      mSpecificData.setChar("addressEntrySize", 1, mSpecificData.getChar(1), FlexField32::eHowCaptured);
      mSpecificData.setShort("lifetime", 1, mSpecificData.getShort(1), FlexField32::eHowCaptured);
      break;
    case 12: // Parameter problem
      mSpecificData.setChar("pointer", 0, mSpecificData.getChar(0), FlexField32::eHowCaptured);
      mSpecificData.setChar("unused", 1, (uchar) 0, FlexField32::eHowUndef);
      mSpecificData.setShort("unused", 1, mSpecificData.getShort(1), FlexField32::eHowCaptured);
      break;
      
    case 30: // traceroute
      mSpecificData.setShort("identifier", 0, mSpecificData.getShort(0), FlexField32::eHowCaptured);
      mSpecificData.setShort("unused", 1, (ushort) 0, FlexField32::eHowUndef);
      break;
    case 31: // conversion error
      mSpecificData.setLong("offset", mSpecificData.getLong(), FlexField32::eHowCaptured);
      break;
    default:
      mSpecificData.setLong("unused", (ulong) 0, FlexField32::eHowUndef);
      break; // wrong type. Can't interprete
    }

  return true;
  }

bool Icmp::analyze_Tail(uchar*& fromPtr, ulong& remainingSize)
  {
  return true;
  }

Element* Icmp::analyze_GetNextElem()
  {
  return NULL;
  }

bool Icmp::checkComplete()
  {
  if (!mType.hasValue())
    {
    return false;
    }
  if (!mCode.hasValue())
    {
    return false;
    }
  if (!mSpecificData.hasValue())
    {
    return false;
    }
  return true; // nothing follows IGMP, so nothing to complete
  }

bool Icmp::tryComplete(ElemStack& stack)
  {
  return checkComplete(); // nothing follows IGMP, so nothing to complete
  }

string Icmp::whatsMissing()
  {
  return "";
  }

bool Icmp::match(Element* other)
  {
  if (typeid(*other) != typeid(Icmp))
    {
    return false;
    }
  Icmp* otherIcmp = (Icmp*) other;

  if (!mType.match(otherIcmp->mType))
    {
    return false;
    }
  if (!mCode.match(otherIcmp->mCode))
    {
    return false;
    }
  if (!mChecksum.match(otherIcmp->mChecksum))
    {
    return false;
    }
  if (!mSpecificData.match(otherIcmp->mSpecificData))
    {
    return false;
    }

  return true;  
  }

Element* Icmp::getNewBlank()
  {
  Icmp* icmp = new Icmp();
  return (Element*) icmp;
  }

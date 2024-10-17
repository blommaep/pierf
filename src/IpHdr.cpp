// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "IpHdr.hpp"
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


IpHdr::IpHdr()
  {
  mDscp.setOffset(2);
  mDscp.setDefault((uchar)0);
  mFlags.setDefault((uchar)0);
  mFlags.setOffset(5); // flagss fill the first three bits of the byte
  mHeaderLength.setOffset(0); 
  mFragmentOffset.setDefault((ushort)0);
  mTtl.displayDecimal();
  mContentLength.displayDecimal();
  mPacketId.displayDecimal();
  }

void IpHdr::parseAttrib(const char** attr, AutoObject* parent, bool checkMandatory, bool storeAsString) noexcept(false)
  {
  char* autoStr=NULL;
  int i=0;
  while (attr[i] != NULL)
    {
    if (!strcmp(attr[i],"from"))
      {
      i++;
      mFromIp.setManual(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"to"))
      {
      i++;
      mToIp.setManual(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"dscp"))
      {
      i++;
      mDscp.setManual(attr[i++], storeAsString);  
      }
    else if (!strcmp(attr[i],"protocol"))
      {
      i++;
      mProtocol.setManual(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"contentlength"))
      {
      i++;
      setContentLength(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"options"))
      {
      i++;
      setOptions(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"ttl"))
      {
      i++;
      setTtl(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"flags"))
      {
      i++;
      setFlags(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"packetid"))
      {
      i++;
      setPacketId(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"fragmentoffset"))
      {
      i++;
      setFragmentOffset(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"checksum"))
      {
      i++;
      setChecksumVal(attr[i++], storeAsString);
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

    if (!mFromIp.hasValue())
      {
      missing += "from ";
      }
    if (!mToIp.hasValue())
      {
      missing += "to ";
      }
    if (!mProtocol.hasValue())
      {
      missing += "protocol ";
      }
    if (!mContentLength.hasValue())
      {
      missing += "contentlength ";
      }
    if (!mOptions.hasValue())
      {
      missing += "options ";
      }
    if (!mTtl.hasValue())
      {
      missing += "'ttl'";
      }
    if (!mPacketId.hasValue()) 
      {
      missing += "'packetid'";
      }

    if (missing != "")
      {
      throw Exception("Missing mandatory attributes to <iphdr> tag: " + missing);
      }
    }

  }

void IpHdr::setFrom(const char* fromIp) noexcept(false)
  {
  mFromIp.setManual(fromIp, false);
  }

void IpHdr::setTo(const char* toIp) noexcept(false)
  {
  mToIp.setManual(toIp, false);
  }

void IpHdr::setDscp(const char* dscpStr) noexcept(false)
  {
  mDscp.setManual(dscpStr, false);  
  }

void IpHdr::setContentLength(ushort contentLength)
  {
  mContentLength.setManualFromValue(contentLength);
  }

void IpHdr::setContentLength(const char* contentLengthStr, bool storeAsString) noexcept(false)
  {
  try
    {
    mContentLength.setManual(contentLengthStr, storeAsString);
    }
  catch (Exception& e)
    {
    throw Exception("Content length value invalid: " + string(e.what()));
    }
  }

void IpHdr::setTtl(const char* ttl, bool storeAsString) noexcept(false)
  {
  try
    {
    mTtl.setManual(ttl, storeAsString);
    }
  catch (Exception& e)
    {
    throw Exception("Invalid TTL: "+ string(e.what()));
    }
  }

void IpHdr::setProtocol(uchar protocol)
  {
  mProtocol.setManualFromValue(protocol);
  }

void IpHdr::setProtocol(const char* protocolStr) noexcept(false)
  {
  mProtocol.setManual(protocolStr, false);
  }

void IpHdr::setPacketId(ushort packetId)
  {
  mPacketId.setManualFromValue(packetId);
  }

void IpHdr::setPacketId(const char* packetId, bool storeAsString) noexcept(false)
  {
  try
    {
    mPacketId.setManual(packetId, storeAsString);
    }
  catch (Exception& e)
    {
    throw Exception("Invalid fragmentid: " + string(e.what()));
    }
  }

void IpHdr::setFlags(const char* flags, bool storeAsString) noexcept(false)
  {
  try
    {
    mFlags.setManual(flags, storeAsString);
    }
  catch (Exception& e)
    {
    throw Exception("Invalid flags value: " + string(e.what()));
    }
  }

void IpHdr::setFragmentOffset(ushort fragmentOffset) noexcept(false)
  {
  try
    {
    mFragmentOffset.setManualFromValue(fragmentOffset);
    }
  catch (Exception& e)
    {
    throw Exception("IP Header: invalid fragment offset: " + string(e.what()));
    }
  }

void IpHdr::setFragmentOffset(const char* fragmentOffset, bool storeAsString) noexcept(false)
  {
  if (storeAsString)
    {
    mFragmentOffset.setManual(fragmentOffset, storeAsString);
    }
  else
    {
    ushort temp;
    try
      {
      temp = atos((char*) fragmentOffset);
      }
    catch (Exception& e)
      {
      throw Exception("Invalid fragmentoffset: " + string(e.what()));
      }
    setFragmentOffset(temp);
    }
  }

void IpHdr::setOptions(const char* optionStr, bool storeAsString) noexcept(false)
  {
  mOptions.resetString();
  mOptions.setManual(optionStr, storeAsString);
  if (mOptions.size() > 40) // total IP header must be < 60 bytes
    {
    throw Exception("Adding the option to the IP header makes this field too large (> 40 bytes): " + string(optionStr));    
    }
  }

void IpHdr::setChecksumVal(const char* checksum, bool storeAsString) noexcept(false)
  {
  try
    {
    mChecksum.setManual(checksum, storeAsString);
    }
  catch (Exception& e)
    {
    throw Exception("Invalid checksum value: " + string(e.what()));
    }
  }

bool IpHdr::addPseudoHeaderChecksum(ChecksumIp& cksum)
  {
  if (!mFromIp.hasValue())
    {
    return false;
    }
  if (!mToIp.hasValue())
    {
    return false;
    }
  if (!mProtocol.hasValue())
    {
    return false;
    }
  if (!mHeaderLength.hasValue())
    {
    return false;
    }
  if (!mContentLength.hasValue())
    {
    return false;
    }
  cksum.addToSum(mFromIp.getAddress());
  cksum.addToSum(mToIp.getAddress());
  cksum.addToSum(htons((ushort)mProtocol.getValue()));
  cksum.addToSum(htons(mContentLength.getValue()));
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
  if (mProtocol.isVar())
    {
    return false;
    }
  if (mContentLength.isVar())
    {
    return false;
    }
  return true; // Everything calculated and fixed
  }

string IpHdr::getString() //tbd: some fields missing
  {
  stringstream retval;
  retval << "<iphdr ";
  
  if (mFromIp.isPrintable())
    {
    retval << "from=\"" << mFromIp.getConfigString() << "\" ";
    }
  if (mToIp.isPrintable())
    {
    retval << "to=\"" << mToIp.getConfigString() << "\" ";
    }
  if (mProtocol.isPrintable())
    {
    retval << "protocol=\"";
    retval << mProtocol.getConfigString();
    retval << "\"";
    }
  if (mContentLength.isPrintable())  
    {
    retval << " contentlength=\"" << mContentLength.getConfigString() << "\"";
    }

  if (mPacketId.isPrintable())
    {
    retval << " packetid=\"" << mPacketId.getConfigString() << "\"";
    }

  if (mTtl.isPrintable())
    {
    retval << " ttl=\"" << mTtl.getConfigString() << "\"";
    }

  if (mOptions.isPrintable() != 0)
    {
    retval << " options=\"" << mOptions.getConfigString() << "\"";
    }

  if (hasVarAssigns())
    {
    retval << " >" << endl << getVarAssignsString();
    retval << "  </iphdr>";
    }
  else
    {
    retval << " />";
    }

  retval << flush;
  return retval.str();
  }

bool IpHdr::getString(string& stringval, const char* fieldName)
  {
  if (!strcmp(fieldName,"from"))
    {
    return mFromIp.getString(stringval);
    }
  else if (!strcmp(fieldName,"to"))
    {
    return mToIp.getString(stringval);
    }
  else if (!strcmp(fieldName,"headerlength"))
    {
    return mHeaderLength.getString(stringval);
    }
  else if (!strcmp(fieldName,"dscp"))
    {
    return mDscp.getString(stringval);
    }
  else if (!strcmp(fieldName,"contentlength"))
    {
    return mContentLength.getString(stringval);
    }
  else if (!strcmp(fieldName,"packetid"))
    {
    return mPacketId.getString(stringval);
    }
  else if (!strcmp(fieldName,"fragmentoffset"))
    {
    return mFragmentOffset.getString(stringval);
    }
  else if (!strcmp(fieldName,"ttl"))
    {
    return mTtl.getString(stringval);
    }
  else if (!strcmp(fieldName,"flags"))
    {
    return mFlags.getString(stringval);
    }
  else if (!strcmp(fieldName,"protocol"))
    {
    return mProtocol.getString(stringval);
    }
  else if (!strcmp(fieldName,"options"))
    {
    return mOptions.getString(stringval);
    }
  else if (!strcmp(fieldName,"checksum"))
    {
    return mChecksum.getString(stringval);
    }
  return false;
  }

ulong32 IpHdr::getSize()
  {
  ushort optionsPlusPadding = mOptions.size();
  optionsPlusPadding = ((optionsPlusPadding + 3) / 4) * 4; //round up to the next multiple of 4
  return 20+optionsPlusPadding; // See the spec: 5 x 32 bit + options
  }

ulong32 IpHdr::getTailSize()
  {
  return 0;
  }

bool IpHdr::copyVar() noexcept(false)
  {
  bool copy = false;
  bool res;
  res = mFromIp.copyVar();
  copy = copy || res;
  res = mToIp.copyVar();
  copy = copy || res;
  res = mHeaderLength.copyVar();
  copy = copy || res;
  res = mDscp.copyVar();
  copy = copy || res;
  res = mContentLength.copyVar();
  copy = copy || res;
  res = mPacketId.copyVar();
  copy = copy || res;
  res = mFragmentOffset.copyVar();
  copy = copy || res;
  res = mFlags.copyVar();
  copy = copy || res;
  res = mTtl.copyVar();
  copy = copy || res;
  res = mProtocol.copyVar();
  copy = copy || res;
  res = mOptions.copyVar();
  copy = copy || res;
  res = mChecksum.copyVar();
  copy = copy || res;
  return copy;
  }

uchar* IpHdr::copyTo(uchar* toPtr)
  {
  // Ethertype: IP (for the ease of stacking, ethertype is included in the higher protocol.
  uchar* ckStart = toPtr;
  *toPtr = 0x40 ; // First four bytes: ipV4 id
  toPtr = mHeaderLength.copyTo(toPtr);
  *toPtr = 0; // The other 2 bits are unused => clear
  toPtr = mDscp.copyTo(toPtr);
  ushort* tmpPtr = (ushort *) toPtr;  
  *tmpPtr++ = htons(getTotalLength());
  toPtr = (uchar*) tmpPtr;
  toPtr = mPacketId.copyTo(toPtr);
  mFlags.copyTo(toPtr); // don't advance pointer: is only 3 bits, need to add fragmentoffset...
  toPtr = mFragmentOffset.copyTo(toPtr);
  toPtr = mTtl.copyTo(toPtr);
  toPtr = mProtocol.copyTo(toPtr);
  uchar* ckPos = toPtr;
  *toPtr++ = 0x0; 
  *toPtr++ = 0x0; 
  toPtr = mFromIp.copyTo(toPtr);
  toPtr = mToIp.copyTo(toPtr);
  toPtr = mOptions.copyTo(toPtr);
  // padding in case options are not rounded properly.
  uchar padding = (mOptions.size() % 4);
  while (padding--) // ip header must have a size of a multiple of 32 bits.
    {
    *toPtr++ = 0xFF;
    }
  if (!mChecksum.isManual())
    {
    mChecksum.calculate(ckStart,toPtr);
    }
  mChecksum.copyTo(ckPos);
//  setChecksum(ckPos,ckStart,toPtr);
  return toPtr;
  }

uchar* IpHdr::copyTail(uchar* toPtr)
  {
  return toPtr;
  }

bool IpHdr::analyze_Head(uchar*& fromPtr, ulong32& remainingSize)
  {
  ulong32 startingSize = remainingSize; // Need this to calculate the header size
  if (remainingSize <1)
    {
    return false;
    }
  if (((*fromPtr) & 0xF0) != 0x40)
    {
    return false; // This is not an Ip header
    }

  if (!mHeaderLength.analyze(fromPtr,remainingSize)) return false;
  fromPtr++;
  remainingSize--;
  if (!mDscp.analyze(fromPtr,remainingSize)) return false;
  fromPtr++; // DSCP field is only 6 bit and therefore doesn't increment fromPtr...
  remainingSize--;

  if (remainingSize >= 2)
    {
    ushort totalLength = ntohs(* (ushort*) fromPtr);
    mContentLength.setAuto(totalLength - mHeaderLength.getValue() * 4);
    mContentLength.wasCaptured();
    fromPtr += 2;
    remainingSize -= 2;
    }
  
  
  if (!mPacketId.analyze(fromPtr,remainingSize)) return false;
  if (!mFlags.analyze(fromPtr,remainingSize)) return false; // don't advance pointer: is only 3 bits, need to add fragmentoffset...
  if (!mFragmentOffset.analyze(fromPtr,remainingSize)) return false;
  fromPtr += 2; // mFragmentOffset  is 13 bit => 2 bytes alltogether
  remainingSize -= 2;
  if (!mTtl.analyze(fromPtr,remainingSize)) return false;
  if (!mProtocol.analyze(fromPtr,remainingSize)) return false;
  if (!mChecksum.analyze(fromPtr,remainingSize)) return false;
  if (!mFromIp.analyze(fromPtr,remainingSize)) return false;
  if (!mToIp.analyze(fromPtr,remainingSize)) return false;

  // finding end of option
  uchar* optionType = fromPtr;
  ulong32 optionSize = (mHeaderLength.getValue()*4) - (startingSize - remainingSize); // signed, to allow <0, eases the loop; (startingSize - remaininSize is the part of the header already analysed, the remaining is option and padding
  long remainingHdrSize = ulong32(optionSize);

  while (*optionType != 0 && remainingHdrSize > 0)
    {
    if (*optionType == 1) // fixed length of 1 byte, NOP
      {
      optionType++;
      remainingHdrSize--;
      }
    else
      {
      optionType++; // now it is actually pointing to the option length
      remainingHdrSize -= *optionType; 
      optionType += (*optionType - 1); // already advanced 1, so advance length-1 : length includes option type and length fields
      }
    }
  if ((remainingHdrSize < 0))
    {
    return false; // inproper length in option field
    }
  
  optionSize -= (ulong32) remainingHdrSize; // remainingHdrSize now is in fact the paddig size
  if (!mOptions.analyze(fromPtr,remainingSize,optionSize)) return false;

  // padding is currently ignored
  fromPtr += remainingHdrSize;
  remainingSize -= remainingHdrSize;

  return true;
  }

bool IpHdr::analyze_Tail(uchar*& fromPtr, ulong32& remainingSize)
  {
  return true;
  }

Element* IpHdr::analyze_GetNextElem()
  {
  if (mProtocol.hasValue())
    {
    uchar protocol = mProtocol.getValue();
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
      // to be completed
      default:
        ;
      }
    }
  return NULL;
  }

bool IpHdr::checkComplete()
  {
  if (!mFromIp.hasValue())
    {
    return false;
    }
  if (!mToIp.hasValue())
    {
    return false;
    }
  if (!mDscp.hasValue())
    {
    return false;
    }
  if (!mContentLength.hasValue())
    {
    return false;
    }
  if (!mPacketId.hasValue())
    {
    return false;
    }
  if (!mFragmentOffset.hasValue())
    {
    return false;
    }
  if (!mFlags.hasValue())
    {
    return false;
    }
  if (!mTtl.hasValue())
    {
    return false;
    }
  if (!mProtocol.hasValue())
    {
    return false;
    }
  if (!mOptions.hasValue())
    {
    return false;
    }
  if (!mHeaderLength.hasValue())
    {
    return false;
    }

  return true;
  }

// important assumption: tryComplete for IpHdr layer is only called if the higher layers are complete!
bool IpHdr::tryComplete(ElemStack& stack)
  {
  if (isAuto())
    {
    enum CompleteState {eIdle,eFoundSelf,eFoundIgmp,eDone};
    CompleteState state = eIdle;
    ushort contentLength = 0;
    bool calcContentLength = mContentLength.needsAuto(); //calculate if it hasn't been set
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
          if (typeid(*elem) == typeid(Icmp)) // layers can't change due to var => hasValue check is sufficient
            {
            if (!mProtocol.hasValue())
              {
              mProtocol.setAuto(0x01);
              }
            if (!mTtl.hasValue())
              {
              mTtl.setAuto(32);
              }
            if (!mOptions.hasValue()) 
              {
              mOptions.setAuto(""); 
              }
            }
          else if (typeid(*elem) == typeid(Igmp))
            {
            state = eFoundIgmp;
            }
          else if (typeid(*elem) == typeid(Udp))
            {
            if (!mTtl.hasValue())
              {
              mTtl.setAuto(128);
              }
            if (!mProtocol.hasValue())
              {
              mProtocol.setAuto(0x11);
              }
            if (!mOptions.hasValue()) 
              {
              mOptions.setAuto(""); 
              }
            state = eDone;
            }
          else if (typeid(*elem) == typeid(Tcp))
            {
            if (!mTtl.hasValue())
              {
              mTtl.setAuto(128);
              }
            if (!mProtocol.hasValue())
              {
              mProtocol.setAuto(0x06);
              }
            if (!mOptions.hasValue()) 
              {
              mOptions.setAuto(""); 
              }
            state = eDone;
            }
          else
            {
            state= eDone; // expecting strict stack order
            }
          }
        else if (state == eFoundIgmp)
          {
          if (typeid(*elem) == typeid(IgmpV2))
            {
            IgmpV2* igmp = (IgmpV2*) elem;
            if (mToIp.needsAuto())
              {
              mToIp = igmp->getDestIp(); // may still be incomplete if the arp's one was incomplete
              mToIp.wasAutoSet();
              }
            if (!mTtl.hasValue())
              {
              mTtl.setAuto(1);
              }
            if (!mProtocol.hasValue())
              {
              mProtocol.setAuto(0x02);
              }
            if (!mOptions.hasValue()) 
              {
              mOptions.setAuto("94:04:00:00"); // set a router alert
              }
            }
          else if (typeid(*elem) == typeid(IgmpV3))
            {
            IgmpV3* igmp = (IgmpV3*) elem;
            if (mToIp.needsAuto())
              {
              mToIp = igmp->getDestIp(); // may still be incomplete if the arp's one was incomplete
              mToIp.wasAutoSet();
              }
            if (!mTtl.hasValue())
              {
              mTtl.setAuto(1);
              }
            if (!mProtocol.hasValue())
              {
              mProtocol.setAuto(0x02);
              }
            if (!mOptions.hasValue()) 
              {
              mOptions.setAuto("94:04:00:00"); // set a router alert
              }
            }
          else
            {
            state = eDone;
            }
          }
        }
      }

    if (calcContentLength)
      {
      mContentLength.setAuto(contentLength);
      }

    if (!mHeaderLength.isManual()) // header length is defaulted. But the default needs to be recalculated each time to cover case of variable fields
      {
      if (mOptions.hasValue())
        {
        uchar ownLength = 20;
        ownLength += (uchar) mOptions.size();
        uchar padding = (ownLength%4);
        ownLength += padding;
        ownLength = ownLength / 4;
        mHeaderLength.setDefault(ownLength);
        }
      }

    return checkComplete();
    }

  // case: not auto and not complete. should never come here
  return false;
  }

string IpHdr::whatsMissing()
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
    if (!mContentLength.hasValue())
      {
      missing += "contentlength ";
      }
    if (!mPacketId.hasValue())
      {
      missing += "packetid ";
      }
    if (!mTtl.hasValue())
      {
      missing += "ttl ";
      }
    if (!mProtocol.hasValue())
      {
      missing += "protocol ";
      }
    if (!mOptions.hasValue())
      {
      missing += "options ";
      }

    return "Failed to complete the field(s): " + missing;
    }

  return "";
  }

ushort IpHdr::getTotalLength()
  {
  if (!mHeaderLength.hasValue())
    {
    return 0;
    }
  if (!mContentLength.hasValue())
    {
    return 0;
    }
  ushort totallength = mContentLength.getValue();
  totallength += mHeaderLength.getValue()*4;
  return totallength;
  }

bool IpHdr::match(Element* other)
  {
  if (typeid(*other) != typeid(IpHdr))
    {
    return false;
    }
  IpHdr* otherIp = (IpHdr*) other;

  if (!mFromIp.match(otherIp->mFromIp))
    {
    return false;
    }
  if (!mToIp.match(otherIp->mToIp))
    {
    return false;
    }
  if (!mHeaderLength.match(otherIp->mHeaderLength))
    {
    return false;
    }
  if (!mDscp.match(otherIp->mDscp))
    {
    return false;
    }
  if (!mContentLength.match(otherIp->mContentLength))
    {
    return false;
    }
  if (!mPacketId.match(otherIp->mPacketId))
    {
    return false;
    }
  if (!mFragmentOffset.match(otherIp->mFragmentOffset))
    {
    return false;
    }
  if (!mFlags.match(otherIp->mFlags))
    {
    return false;
    }
  if (!mTtl.match(otherIp->mTtl))
    {
    return false;
    }
  if (!mProtocol.match(otherIp->mProtocol))
    {
    return false;
    }
  if (!mOptions.match(otherIp->mOptions))
    {
    return false;
    }
  if (!mChecksum.match(otherIp->mChecksum))
    {
    return false;
    }

  return true;  
  
  }

Element* IpHdr::getNewBlank()
  {
  IpHdr* ipHdr = new IpHdr();
  return (Element*) ipHdr;
  }


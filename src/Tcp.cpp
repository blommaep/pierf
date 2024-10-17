// Copyright (c) 2006-2011, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "Tcp.hpp"
#include "IpHdr.hpp"
#include "Ipv6.hpp"
#include <sstream>
#include <string.h>
#include <iostream> 
#include <typeinfo>

//// OVERLOADED CLASSES FOR DEDICATED FIELD INTERPRETATIONS ////

uchar TcpFlags::stringToValue(const char* inString) noexcept(false)
  {
  uchar tempFlags = 0;
  char* currentPos = (char*) inString;
  while (*currentPos != 0)
    {
    if (!strncmp(currentPos, "syn", 3))
      {
      tempFlags |= 0x2;
      currentPos += 3;
      }
    else if (!strncmp(currentPos, "fin", 3))
      {
      tempFlags |= 0x1;
      currentPos += 3;
      }
    else if (!strncmp(currentPos, "rst", 3)) //reset
      {
      tempFlags |= 0x4;
      currentPos += 3;
      }
    else if (!strncmp(currentPos, "psh", 3)) //push
      {
      tempFlags |= 0x8;
      currentPos += 3;
      }
    else if (!strncmp(currentPos, "ack", 3)) //acknowledgement
      {
      tempFlags |= 0x10;
      currentPos += 3;
      }
    else if (!strncmp(currentPos, "urg", 3)) //urgent
      {
      tempFlags |= 0x20;
      currentPos += 3;
      }
    else if (!strncmp(currentPos, "ecn", 3)) //ecn-echo
      {
      tempFlags |= 0x40;
      currentPos += 3;
      }
    else if (!strncmp(currentPos, "con", 3)) //congestion
      {
      tempFlags |= 0x80;
      currentPos += 3;
      }
    else 
      {
      throw Exception("Unexpected string specified as tcp flags: " + string(currentPos));
      }

    if (*currentPos == '+') // Advance if another flag is following
      {
      currentPos++;
      }
    }

  return tempFlags;
  }

void TcpFlags::setManualFromValue(const char* inString) noexcept(false)
  {
  if (!strncmp(inString, "0x", 2)) // Hex number => interpete as is
    {
    Bitfield8::setManualFromValue(inString);
    }
  else
    {
    uchar tempFlags = stringToValue(inString);
    Bitfield8::setManualFromValue(tempFlags);
    }
  }

void TcpFlags::setAuto(const char* inString) noexcept(false)
  {
  if (!strncmp(inString, "0x", 2)) // Hex number => interpete as is
    {
    Bitfield8::setAuto(inString);
    }
  else
    {
    uchar tempFlags = stringToValue(inString);
    Bitfield8::setAuto(tempFlags);
    }
  }

void TcpFlags::setDefault(const char* inString) noexcept(false)
  {
  if (!strncmp(inString, "0x", 2)) // Hex number => interpete as is
    {
    Bitfield8::setDefault(inString);
    }
  else
    {
    uchar tempFlags = stringToValue(inString);
    Bitfield8::setDefault(tempFlags);
    }
  }

string TcpFlags::getStringFromBinary() const
  {
  bool firstPrinted=false;
  string result;

  if ((mData & 0x1) > 0)
    {
    result += "fin";
    firstPrinted = true;
    }
  if ((mData & 0x2) > 0)
    {
    if (firstPrinted)
      {
      result += "+";
      }
    result += "syn";
    firstPrinted = true;
    }
  if ((mData & 0x4) > 0)
    {
    if (firstPrinted)
      {
      result += "+";
      }
    result += "rst";
    firstPrinted = true;
    }
  if ((mData & 0x8) > 0)
    {
    if (firstPrinted)
      {
      result += "+";
      }
    result += "psh";
    firstPrinted = true;
    }
  if ((mData & 0x10) > 0)
    {
    if (firstPrinted)
      {
      result += "+";
      }
    result += "ack";
    firstPrinted = true;
    }
  if ((mData & 0x20) > 0)
    {
    if (firstPrinted)
      {
      result += "+";
      }
    result += "urg";
    firstPrinted = true;
    }
  if ((mData & 0x40) > 0)
    {
    if (firstPrinted)
      {
      result += "+";
      }
    result += "ecn";
    firstPrinted = true;
    }
  if ((mData & 0x80) > 0)
    {
    if (firstPrinted)
      {
      result += "+";
      }
    result += "con";
    firstPrinted = true;
    }
  return result;
  }

bool TcpFlags::getStringFromBinary(string& stringval) const
  {
  if (hasValue())
    {
    stringval = getString();
    return true;
    }
  return false;
  }



/////////////////// TCP HDR ITSELF ///////////////////////

Tcp::Tcp()
  :mChecksumIpRetrieved(false)
  {
  mSourcePort.displayDecimal();
  mDestPort.displayDecimal();
  mHeaderLength.displayDecimal();
  mHeaderLength.setOffset(4);
  mAckNr.setDefault(ulong32(0)); // Common default: when no ack, default is 0. 
  mSeqNr.displayDecimal();
  mAckNr.displayDecimal();
  mWindowSize.setDefault(ushort(0xFFFF)); // Typically used default
  mWindowSize.displayDecimal();
  mFlags.setDefault(uchar(0x0));
  mUrgentPointer.setDefault(ushort(0x0)); // Rarely used, default=0
  mOptions.setDefault("");
  }

void Tcp::parseAttrib(const char** attr, AutoObject* parent, bool checkMandatory, bool storeAsString) noexcept(false)
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
    else if (!strcmp(attr[i],"sequencenr"))
      {
      i++;
      try
        {
        mSeqNr.setManual(attr[i++], storeAsString);
        }
      catch (Exception& e)
        {
        throw Exception ("Invalid Sequence number: "+ string(e.what()));
        }
      }
    else if (!strcmp(attr[i],"acknowledgenr"))
      {
      i++;
      try
        {
        mAckNr.setManual(attr[i++], storeAsString);
        }
      catch (Exception& e)
        {
        throw Exception ("Invalid Acknowledgement number: "+ string(e.what()));
        }
      }
    else if (!strcmp(attr[i],"headerlength"))
      {
      i++;
      setLength(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"flags"))
      {
      i++;
      mFlags.setManual(attr[i++], storeAsString);
      }
    else if (!strcmp(attr[i],"windowsize"))
      {
      i++;
      try
        {
        mWindowSize.setManual(attr[i++], storeAsString);
        }
      catch (Exception& e)
        {
        throw Exception ("Invalid window size: "+ string(e.what()));
        }
      }
    else if (!strcmp(attr[i],"urgentpointer"))
      {
      i++;
      try
        {
        mUrgentPointer.setManual(attr[i++], storeAsString);
        }
      catch (Exception& e)
        {
        throw Exception ("Invalid window size: "+ string(e.what()));
        }
      }
    else if (!strcmp(attr[i],"options"))
      {
      i++;
      try
        {
        setOptions(attr[i++], storeAsString);
        }
      catch (Exception& e)
        {
        throw Exception ("Invalid options field: "+ string(e.what()));
        }
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
      throw Exception("Unexepected attribute: " + string(attr[i]) + " in <tcp> tag.");
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
    if (!mSeqNr.hasValue())
      {
      missing += " sequencenr";
      }
    if (!mHeaderLength.hasValue())
      {
      missing += " headerlength";
      }

    if (missing != "")
      {
      throw Exception("Missing mandatory attributes to <tcp> tag:" + missing);
      }
    }
  
  }

void Tcp::setSourcePort(const char* sourcePort, bool storeAsString) noexcept(false)
  {
  try
    {
    mSourcePort.setManual(sourcePort, storeAsString);
    }
  catch (Exception& e)
    {
    throw Exception("Source port invalid: " + string(e.what()));
    }
  }

void Tcp::setDestPort(const char* destPort, bool storeAsString) noexcept(false)
  {
  try
    {
    mDestPort.setManual(destPort, storeAsString);
    }
  catch (Exception& e)
    {
    throw Exception("Destination port invalid: " + string(e.what()));
    }
  }

void Tcp::setLength(const char* length, bool storeAsString) noexcept(false)
  {
  try
    {
    mHeaderLength.setManual(length, storeAsString);
    }
  catch (Exception& e)
    {
    throw Exception("UDP length invalid: " + string(e.what()));
    }
  }


void Tcp::setChecksum(const char* checkSum, bool storeAsString) noexcept(false)
  {
  try
    {
    mChecksum.setManual(checkSum, storeAsString);
    }
  catch (Exception& e)
    {
    throw Exception("Checksum invalid: " + string(e.what()));
    }
  }

void Tcp::setOptions(const char* optionStr, bool storeAsString) noexcept(false)
  {
  mOptions.resetString();
  mOptions.setManual(optionStr, storeAsString);
  }

string Tcp::getString() 
  {
  stringstream retval;
  retval << "<tcp ";
  
  if (mSourcePort.isPrintable())
    {
    retval << "sourceport=\"" << mSourcePort.getConfigString() << "\" ";
    }
  if (mDestPort.isPrintable())
    {
    retval << "destport=\"" << mDestPort.getConfigString() << "\" ";
    }
  if (mSeqNr.isPrintable())
    {
    retval << "sequencenr=\"" << mSeqNr.getConfigString() << "\" ";
    }
  if (mAckNr.isPrintable())
    {
    retval << "acknowledgenr=\"" << mAckNr.getConfigString() << "\" ";
    }
  if (mHeaderLength.isPrintable())
    {
    retval << "headerlength=\"" << mHeaderLength.getConfigString() << "\" ";
    }
  if (mFlags.isPrintable())
    {
    retval << "flags=\"" << mFlags.getConfigString() << "\" ";
    }
  if (mWindowSize.isPrintable())
    {
    retval << "windowsize=\"" << mWindowSize.getConfigString() << "\" ";
    }
  if (mUrgentPointer.isPrintable())
    {
    retval << "urgentpointer=\"" << mUrgentPointer.getConfigString() << "\" ";
    }
  if (mOptions.isPrintable())
    {
    retval << "options=\"" << mOptions.getConfigString() << "\" ";
    }
  if (mChecksum.isPrintable())  
    {
    retval << "checksum=\"" << mChecksum.getConfigString() << "\" ";
    }

  if (hasVarAssigns())
    {
    retval << ">" << endl << getVarAssignsString();
    retval << "  </tcp>";
    }
  else
    {
    retval << "/>";
    }

  retval << flush;
  return retval.str();
  }

bool Tcp::getString(string& stringval, const char* fieldName)
  {
  if (!strcmp(fieldName, "sourceport"))
    {
    return mSourcePort.getString(stringval);
    }  
  if (!strcmp(fieldName, "destport"))
    {
    return mDestPort.getString(stringval);
    }  
  if (!strcmp(fieldName, "sequencenr"))
    {
    return mSeqNr.getString(stringval);
    }  
  if (!strcmp(fieldName, "acknowledgenr"))
    {
    return mAckNr.getString(stringval);
    }  
  if (!strcmp(fieldName, "headerlength"))
    {
    return mHeaderLength.getString(stringval);
    }  
  if (!strcmp(fieldName, "flags"))
    {
    return mFlags.getString(stringval);
    }  
  if (!strcmp(fieldName, "windowsize"))
    {
    return mWindowSize.getString(stringval);
    }  
  if (!strcmp(fieldName, "urgentpointer"))
    {
    return mUrgentPointer.getString(stringval);
    }  
  if (!strcmp(fieldName, "options"))
    {
    return mOptions.getString(stringval);
    }  
  if (!strcmp(fieldName, "checksum"))
    {
    return mChecksum.getString(stringval);
    }  
  return false;
  }

ulong32 Tcp::getSize()
  {
  ushort optionsPlusPadding = mOptions.size();
  optionsPlusPadding = ((optionsPlusPadding + 3) / 4) * 4; //round up to the next multiple of 4
  return 20+optionsPlusPadding; // See the spec: 5 x 32 bit + options
  }

ulong32 Tcp::getTailSize()
  {
  return 0;
  }

bool Tcp::copyVar() noexcept(false)
  {
  bool copy = false;
  bool res;
  res = mSourcePort.copyVar(); 
  copy = copy || res;
  res = mDestPort.copyVar();
  copy = copy || res;
  res = mSeqNr.copyVar();
  copy = copy || res;
  res = mAckNr.copyVar();
  copy = copy || res;
  res = mHeaderLength.copyVar();
  copy = copy || res;
  res = mFlags.copyVar();
  copy = copy || res;
  res = mWindowSize.copyVar();
  copy = copy || res;
  res = mUrgentPointer.copyVar();
  copy = copy || res;
  res = mOptions.copyVar();
  copy = copy || res;
  res = mChecksum.copyVar();
  copy = copy || res;
  return copy;
  }

uchar* Tcp::copyTo(uchar* toPtr)
  {
  uchar* ckStart = toPtr;
  toPtr = mSourcePort.copyTo(toPtr);
  toPtr = mDestPort.copyTo(toPtr);
  toPtr = mSeqNr.copyTo(toPtr);
  toPtr = mAckNr.copyTo(toPtr);
  *toPtr = 0;
  toPtr = mHeaderLength.copyTo(toPtr);
  toPtr = mFlags.copyTo(toPtr);
  toPtr = mWindowSize.copyTo(toPtr);

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

  mContentStart = toPtr; // Include all of the remaining header too
  toPtr = mUrgentPointer.copyTo(toPtr);
  toPtr = mOptions.copyTo(toPtr);
  uchar padding = (mOptions.size() % 4);
  while (padding--) // ip header must have a size of a multiple of 32 bits.
    {
    *toPtr++ = 0x00;
    }

  return toPtr;
  }

uchar* Tcp::copyTail(uchar* toPtr)
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

bool Tcp::analyze_Head(uchar*& fromPtr, ulong32& remainingSize)
  {
  ulong32 startingSize = remainingSize; // Need this to calculate the header size
  if (!mSourcePort.analyze(fromPtr,remainingSize)) return false;
  if (!mDestPort.analyze(fromPtr,remainingSize)) return false;
  if (!mSeqNr.analyze(fromPtr,remainingSize)) return false;
  if (!mAckNr.analyze(fromPtr,remainingSize)) return false;
  if (!mHeaderLength.analyze(fromPtr,remainingSize)) return false;
  fromPtr++; // The next for bits are ignored...
  remainingSize--;
  if (!mFlags.analyze(fromPtr,remainingSize)) return false;
  if (!mWindowSize.analyze(fromPtr,remainingSize)) return false;
  if (!mChecksum.analyze(fromPtr,remainingSize)) return false;
  if (!mUrgentPointer.analyze(fromPtr,remainingSize)) return false;
  ulong32 optionSize = (mHeaderLength.getValue()*4) - (startingSize - remainingSize); // signed, to allow <0, eases the loop; (startingSize - remaininSize is the part of the header already analysed, the remaining is option and padding
  //tbd: complete option detection and any further option and other field functions
  if (!mOptions.analyze(fromPtr,remainingSize,optionSize)) return false;

  return true;
  }

bool Tcp::analyze_Tail(uchar*& fromPtr, ulong32& remainingSize)
  {
  return true;
  }

Element* Tcp::analyze_GetNextElem()
  {
  return NULL; // Would need to look at port for higher layer protocols, but they are not currently supported
  }

bool Tcp::match(Element* other)
  {
  if (typeid(*other) != typeid(Tcp))
    {
    return false;
    }
  Tcp* otherTcp = (Tcp*) other;

  //isPrintable: manual or captured. Only those two kinds must be matched don't care about defaults or autos
  if (!mSourcePort.match(otherTcp->mSourcePort))
    {
    return false;
    }
  if (!mDestPort.match(otherTcp->mDestPort))
    {
    return false;
    }
  if (!mSeqNr.match(otherTcp->mSeqNr))
    {
    return false;
    }
  if (!mAckNr.match(otherTcp->mAckNr))
    {
    return false;
    }
  if (!mHeaderLength.match(otherTcp->mHeaderLength))
    {
    return false;
    }
  if (!mFlags.match(otherTcp->mFlags))
    {
    return false;
    }
  if (!mWindowSize.match(otherTcp->mWindowSize))
    {
    return false;
    }
  if (!mUrgentPointer.match(otherTcp->mUrgentPointer))
    {
    return false;
    }
  if (!mOptions.match(otherTcp->mOptions))
    {
    return false;
    }
  if (!mChecksum.match(otherTcp->mChecksum))
    {
    return false;
    }

  return true;
  }

bool Tcp::checkComplete()
  {
  if (!mSourcePort.hasValue())
    {
    return false;
    }
  if (!mDestPort.hasValue())
    {
    return false;
    }
  if (!mSeqNr.hasValue())
    {
    return false;
    }
  if (!mAckNr.hasValue())
    {
    return false;
    }
  if (!mHeaderLength.hasValue())
    {
    return false;
    }
  if (!mFlags.hasValue())
    {
    return false;
    }
  if (!mWindowSize.hasValue())
    {
    return false;
    }
  if (!mUrgentPointer.hasValue())
    {
    return false;
    }
  if (!mOptions.hasValue())
    {
    return false;
    }
  if (!mChecksum.isManual() && !mIpPseudoHdrChecksum.hasValue()) // If not manual, it is generated, but need to retrieve the ip pseudo header checksum during trycomplete...
    {
    return false;
    }

  return true;
  }

// important assumption: tryComplete for Tcp layer is only called if the higher layers are complete!
bool Tcp::tryComplete(ElemStack& stack)
  {
  if (mChecksum.isManual())
    {
    mChecksumIpRetrieved = true; // Nothing was retrieved, but no need to do so...
    }

  // fix header length default depending on options size
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
    else
      {
      mHeaderLength.setDefault(5);
      }
    }


  if (!mChecksumIpRetrieved || isAuto())
    {
  // even in non auto, go through the stack to find the ip layer: need it for checksum
    enum CompleteState {eIdle,eFoundIp,eFoundIpv6,eFoundSelf,eDone};
    CompleteState state = eIdle;
    mIpPseudoHdrChecksum.reset();
    mChecksumIpRetrieved = false;
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
        if (typeid(*elem) == typeid(Ipv6))
          {
          ipv6 = (Ipv6*) elem; // we (may) need ip for checksum calculation
          state = eFoundIpv6;
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
          }
        else
          {
          state = eIdle; // it was not the ip layer for this tcp
          ip = NULL;
          }
        }
      else if (state == eFoundIpv6) // pseudo header detection!!
        {
        if (elem == this)
          {
          state = eFoundSelf;
          if (ipv6 != NULL)
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
          state = eIdle; // it was not the ip layer for this tcp
          ipv6 = NULL;
          }
        }
      }
    }


  return checkComplete();
  }

string Tcp::whatsMissing()
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
    if (!mSeqNr.hasValue())
      {
      missing += "sequencenr ";
      }
    if (!mAckNr.hasValue())
      {
      missing += "acknowledgenr ";
      }
    if (!mHeaderLength.hasValue())
      {
      missing += "headerlength ";
      }
    if (!mWindowSize.hasValue())
      {
      missing += "windowsize ";
      }
    if (!mUrgentPointer.hasValue())
      {
      missing += "urgentpointer ";
      }
    if (!mIpPseudoHdrChecksum.hasValue() && !mChecksum.isManual())
      {
      missing += "checksum ";
      }

    return "Failed to complete the field(s): " + missing;
    }

  return "";
  }

Element* Tcp::getNewBlank()
  {
  Tcp* tcp = new Tcp();
  return (Element*) tcp;
  }

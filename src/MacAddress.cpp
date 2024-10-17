// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "MacAddress.hpp"

#include <iostream> // for cout and cin
#include <iomanip>
#include <fstream>
#include <sstream>

#include "ByteString.hpp"


void MacAddress::stringToVal(const char* inString) throw (Exception)
  {
  ByteString convert;
  try
    {
    convert.addString(inString);
    }
  catch (Exception e)
    {
    stringstream msg;
    msg << "Invalid mac address: " << inString << " :" << endl << "  " << e.what() << endl;
    throw (Exception(msg.str()));
    }
    
  if (convert.size() == 6)
    {
    convert.copyTo(mAddress);
    }
  else
    {
    throw Exception("Invalid mac address. Size is not 6 Bytes.");
    }
  }
  
MacAddress::MacAddress()
  {
  int i;
  for (i=0;i<6;i++)
    {
    mAddress[i] = 0xFF;
    }
  }

void MacAddress::setDefault(const char* inString) throw (Exception)
  {
  stringToVal(inString);
  wasDefaulted();
  }

void MacAddress::setManualFromValue(const char* inString) throw (Exception)
  {
  stringToVal(inString);
  wasManuallySet();
  }

void MacAddress::setAuto(const char* inString) throw (Exception)
  {
  stringToVal(inString);
  wasAutoSet();
  }

void MacAddress::autoCopy(const MacAddress& copyFrom)
  {
  if (copyFrom.hasValue())
    {
    for (int i=0;i<6;i++)
      {
      mAddress[i] = copyFrom.mAddress[i];
      }

    wasAutoSet();
    }
  }

void MacAddress::setAddressFromMcastIp(ulong mcastIp) throw (Exception)
  {
  // by ethernet spec, igmp/ip/ethernet will get assigned a multicast mac address
  // in the range 01:00:5E:00:00:00 till 01:00:5E:7F:FF:FF.
  // Within that range, the mapping of IP addresses to mac addresses is a simple
  // copy of the last bits
  // Note: not checking if the ip-address is within multicast range!
  typedef union 
    {
    uchar chars[4];
    ulong whole;
    } u_fourBytes;

  u_fourBytes temp;
  temp.whole=mcastIp;

  mAddress[0] = 0x01;
  mAddress[1] = 0x00;
  mAddress[2] = 0x5E;
  mAddress[3] = temp.chars[1] & 0x7F; // 1 bit less then a byte
  mAddress[4] = temp.chars[2];
  mAddress[5] = temp.chars[3];
  wasAutoSet(); // asked to calculate, so certainly auto
  }

string MacAddress::getStringFromBinary() const
  {
  stringstream retval;
  retval.setf(ios::uppercase); // print hex in uppercase
  retval.setf(ios::hex,ios::basefield); // print hex in uppercase
  retval.fill('0');

  int i;
  for (i=0;i<5;i++)
    {
    retval << setw(2) << (int) mAddress[i] << ":";
    }
  retval << setw(2) << (int) mAddress[5] << flush;
  return retval.str();
  }

bool MacAddress::getStringFromBinary(string& stringval) const
  {
  if (hasValue())
    {
    stringval = getStringFromBinary();
    return true;
    }
  return false;  
  }

uchar* MacAddress::copyTo(uchar* toPtr)
  {
  int i;
  for (i=0;i<6;i++)
    {
    toPtr[i] = mAddress[i];
    }
  return toPtr+6;
  }

bool MacAddress::analyze(uchar*& fromPtr, ulong& remainingSize)
  {
  if (remainingSize >= 6)
    {
    int i;
    for (i=0;i<6;i++)
      {
      mAddress[i] = fromPtr[i];
      }
    fromPtr += 6;
    remainingSize -= 6;
    wasCaptured();
    return true;
    }
  return false;
  }

bool MacAddress::match(const MacAddress& other)
  {
  if (isComparable() && other.hasValue())
    {
    if (isString() || other.isString())
      {
      return matchByString(other);
      }
    else
      {
      for (int i=0;i<6;i++)
        {
        if (mAddress[i] != other.mAddress[i])
          {
          return false;
          }
        }
      }
    }
  //else
  return true;
  }


// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "Bitfield8.hpp"

#include <iostream> // for cout and cin
#include <iomanip>
#include <fstream>
#include <sstream>


Bitfield8::Bitfield8()
  : mOffset(0), mDisplayType(eHex)
  {
  }

void Bitfield8::setOffset(uchar offset) throw (Exception)
  {
  if (offset > 7)
    {
    throw Exception("Invalid offset for an 8 bit field. Maximum offset is 7");
    }
  mOffset = offset;
  }

void Bitfield8::stringToVal(const char* inString) throw (Exception)
  {
  int temp = textToLong(inString);
  if (temp < 0 || temp > 0xFF)
    {
    throw Exception("Invalid input string for what should be a 1 byte integer value");
    }
  setVal((uchar)temp);
  }

void Bitfield8::setVal(ushort val) throw (Exception)
  {
  mData = val;
  // no exception/checks. This is merely here for easing enherited classes to overload the setVal
  }

void Bitfield8::setDefault(const char* inString) throw (Exception)
  {
  stringToVal(inString);
  wasDefaulted();
  }

void Bitfield8::setManualFromValue(const char* inString) throw (Exception)
  {
  stringToVal(inString);
  wasManuallySet();
  }

void Bitfield8::setAuto(const char* inString) throw (Exception)
  {
  stringToVal(inString);
  wasAutoSet();
  }

void Bitfield8::setDefault(const uchar inValue)
  {
  mData = inValue;
  wasDefaulted();
  }

void Bitfield8::setManualFromValue(const uchar inValue)
  {
  mData = inValue;
  wasManuallySet();
  }
   
void Bitfield8::setAuto(const uchar inValue)
  {
  mData = inValue;
  wasAutoSet();
  }

void Bitfield8::displayDecimal()
  {
  mDisplayType = eDec;
  }

void Bitfield8::displayChar()
  {
  mDisplayType = eChar;
  }

string Bitfield8::getStringFromBinary() const
  {
  stringstream retval;

  switch (mDisplayType)
    {
    case eHex:
      retval.setf(ios::uppercase); // print hex in uppercase
      retval.setf(ios::hex,ios::basefield); // print hex in uppercase
      retval.fill('0');
      retval << "0x" << setw(2) << (int) mData << flush;
      break;
    case eDec:
      retval << (int) mData << flush;
      break;
    case eChar:
      retval << mData << flush;
      break;
    }
  return retval.str();
  }

bool Bitfield8::getStringFromBinary(string& stringval) const
  {
  if (hasValue())
    {
    stringval = getStringFromBinary();
    return true;
    }
  return false;
  }

uchar Bitfield8::getValue()
  {
  return mData;
  }

bool Bitfield8::operator==(unsigned int value)
  {
  return ((unsigned int) mData == value);
  }

bool Bitfield8::operator!=(unsigned int value)
  {
  return ((unsigned int) mData != value);
  }

uchar* Bitfield8::copyTo(uchar* toPtr)
  {
  if (mOffset == 0)
    {
    *toPtr++ = mData;
    return toPtr;
    }
  else // remind: in network order, offset is reverse: in arithmetic, MSB is left bit, cf Bitfield4 for full more clarific 
    {
    // add zero's at the proper place
    ushort temp = (ushort) mData;
    temp = htons(temp << (8-mOffset));

    ushort* tmp = (ushort*) toPtr;
    
    // Two masks are needed to mask out the part of what is there that needs to be kept
    ushort mask1 = htons(0x00FF >> mOffset);
    ushort mask2 = htons(0xFF00 << (8-mOffset));
    mask1 |= mask2;
    *tmp &= mask1;

    // Now insert our value
    *tmp |= temp;

    toPtr++; //Advance always only 1 byte
    return toPtr; 
    }
  }

bool Bitfield8::analyze(uchar*& fromPtr, ulong32& remainingSize)
  {
  if (mOffset==0)
    {
    if (remainingSize < 1)
      {
      return false;
      }
    
    mData = *fromPtr;
    }
  else
    {
    if (remainingSize < 2)
      {
      return false;
      }
    
    ushort temp = ntohs(* ((ushort*)fromPtr));
    temp >>= (8-mOffset); // shift mOffset, so that it becomes the base value
    temp = temp & 0x00FF; // now, keep only the lower bits.
    mData = (uchar) temp;
    }

  fromPtr++; // always 1 byte
  remainingSize--;
  wasCaptured();
  return true;
  }

bool Bitfield8::match(Bitfield8& other)
  {
  if (isPrintable() && other.hasValue())
    {
    if (isString() || other.isString())
      {
      return matchByString(other);
      }
    
    if (mData != other.mData)
      {
      return false;
      }
    }
  //else
  return true;
  
  }


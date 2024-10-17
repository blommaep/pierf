// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "Bitfield32.hpp"

#include <iostream> // for cout and cin
#include <iomanip>
#include <fstream>
#include <sstream>


Bitfield32::Bitfield32()
  : mDisplayType(eHex)
  {}

void Bitfield32::stringToVal(const char* inString) throw (Exception)
  {
  ulong temp = textToLong(inString);
  setVal((ulong) temp);
  }
  
void Bitfield32::setVal(ulong val) throw (Exception)
  {
  mData = val;
  // no exception/checks. This is merely here for easing enherited classes to overload the setVal
  }

void Bitfield32::setDefault(const char* inString) throw (Exception)
  {
  stringToVal(inString);
  wasDefaulted();
  }

void Bitfield32::setManualFromValue(const char* inString) throw (Exception)
  {
  stringToVal(inString);
  wasManuallySet();
  }

void Bitfield32::setAuto(const char* inString) throw (Exception)
  {
  stringToVal(inString);
  wasAutoSet();
  }

void Bitfield32::setDefault(const ulong inValue) throw (Exception)
  {
  setVal(inValue);
  wasDefaulted();
  }

void Bitfield32::setManualFromValue(const ulong inValue) throw (Exception)
  {
  setVal(inValue);
  wasManuallySet();
  }
   
void Bitfield32::setAuto(const ulong inValue) throw (Exception)
  {
  setVal(inValue);
  wasAutoSet();
  }

void Bitfield32::displayDecimal()
  {
  mDisplayType = eDec;
  }

void Bitfield32::displayChar()
  {
  mDisplayType = eChar;
  }

string Bitfield32::getStringFromBinary() const
  {
  stringstream retval;

  switch (mDisplayType)
    {
    case eHex:
      retval.setf(ios::uppercase); // print hex in uppercase
      retval.setf(ios::hex,ios::basefield); // print hex in uppercase
      retval.fill('0');
      retval << "0x" << setw(4) << (int) mData << flush;
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

bool Bitfield32::getStringFromBinary(string& stringval) const
  {
  if (hasValue())
    {
    stringval = getStringFromBinary();
    return true;
    }
  return false;
  }
  
ulong Bitfield32::getValue()
  {
  return mData;
  }

bool Bitfield32::operator==(unsigned int value)
  {
  return ((unsigned int) mData == value);
  }

bool Bitfield32::operator!=(unsigned int value)
  {
  return ((unsigned int) mData != value);
  }

uchar* Bitfield32::copyTo(uchar* toPtr)
  {
  ulong* tmp = (ulong*) toPtr;
  *tmp++ = htonl(mData);
  return (uchar*) tmp;
  }

bool Bitfield32::analyze(uchar*& fromPtr, ulong& remainingSize)
  {
  if (remainingSize < 4)
    {
    return false;
    }
  
  mData = ntohl(* (ulong*) fromPtr);
  fromPtr += 4;
  remainingSize -= 4;
  wasCaptured();
  return true;
  }

bool Bitfield32::match(Bitfield32& other)
  {
  if (isComparable() && other.hasValue())
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

// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "Bitfield16.hpp"

#include <iostream> // for cout and cin
#include <iomanip>
#include <fstream>
#include <sstream>


Bitfield16::Bitfield16()
  : mDisplayType(eHex)
  {}

void Bitfield16::stringToVal(const char* inString) noexcept(false)
  {
  ulong32 temp = textToLong(inString);
  if (temp < 0 || temp > 0xFFFF)
    {
    throw Exception("Invalid input string for what should be a 2 byte integer value");
    }
  setVal((ushort) temp);
  }
  
void Bitfield16::setVal(ushort val) noexcept(false)
  {
  mData = val;
  // no exception/checks. This is merely here for easing enherited classes to overload the setVal
  }

void Bitfield16::setDefault(const char* inString) noexcept(false)
  {
  stringToVal(inString);
  wasDefaulted();
  }

void Bitfield16::setManualFromValue(const char* inString) noexcept(false)
  {
  stringToVal(inString);
  wasManuallySet();
  }

void Bitfield16::setAuto(const char* inString) noexcept(false)
  {
  stringToVal(inString);
  wasAutoSet();
  }

void Bitfield16::setDefault(const ushort inValue) noexcept(false)
  {
  setVal(inValue);
  wasDefaulted();
  }

void Bitfield16::setManualFromValue(const ushort inValue) noexcept(false)
  {
  setVal(inValue);
  wasManuallySet();
  }
   
void Bitfield16::setAuto(const ushort inValue) noexcept(false)
  {
  setVal(inValue);
  wasAutoSet();
  }

void Bitfield16::displayDecimal()
  {
  mDisplayType = eDec;
  }

void Bitfield16::displayChar()
  {
  mDisplayType = eChar;
  }

string Bitfield16::getStringFromBinary() const
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

bool Bitfield16::getStringFromBinary(string& stringval) const
  {
  if (hasValue())
    {
    stringval = getStringFromBinary();
    return true;
    }
  return false;
  }
  
ushort Bitfield16::getValue()
  {
  return mData;
  }

bool Bitfield16::operator==(unsigned int value)
  {
  return ((unsigned int) mData == value);
  }

bool Bitfield16::operator!=(unsigned int value)
  {
  return ((unsigned int) mData != value);
  }

uchar* Bitfield16::copyTo(uchar* toPtr)
  {
  ushort* tmp = (ushort*) toPtr;
  *tmp++ = htons(mData);
  return (uchar*) tmp;
  }

bool Bitfield16::analyze(uchar*& fromPtr, ulong32& remainingSize)
  {
  if (remainingSize < 2)
    {
    return false;
    }
  
  mData = ntohs(* (ushort*) fromPtr);
  fromPtr += 2;
  remainingSize -= 2;
  wasCaptured();
  return true;
  }

bool Bitfield16::match(Bitfield16& other)
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

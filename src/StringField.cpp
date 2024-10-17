// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "StringField.hpp"

#include <iostream> // for cout and cin
#include <iomanip>
#include <fstream>
#include <sstream>

StringField::~StringField()
  {
  }
  
void StringField::setDefault(const char* inString) throw (Exception)
  {
  resetString();
  addString(inString);
  wasDefaulted();
  }

void StringField::setManualFromValue(const char* inString) throw (Exception)
  {
  resetString();
  addString(inString);
  wasManuallySet();
  }

void StringField::setAuto(const char* inString) throw (Exception)
  {
  resetString();
  addString(inString);
  wasAutoSet();
  }

string StringField::getString()
  {
  return ByteString::getString();
  }

bool StringField::getString(string& stringval)
  {
  if (hasValue())
    {
    stringval = ByteString::getString();
    return true;
    }
  return false;
  }

uchar* StringField::copyTo(uchar* toPtr)
  {
  return ByteString::copyTo(toPtr);
  }

uchar* StringField::copyTo(uchar* toPtr, ulong maxSize) // copy max. maxSize bytes
  {
  return ByteString::copyTo(toPtr, maxSize);
  }

bool StringField::analyze(uchar*& fromPtr, ulong& remainingSize) // consumes all of remainingSize
  {
  bool result = analyze(fromPtr,remainingSize,remainingSize);
  return result;
  }

bool StringField::analyze(uchar*& fromPtr, ulong& remainingSize, ulong fieldSize) // consumes the specified amount of bytes
  {
  if (fieldSize > remainingSize)
    {
    return false;
    }
  
  addBytes(fromPtr,fieldSize);
  remainingSize -= fieldSize;
  fromPtr += fieldSize;  
  wasCaptured();
  return true;
  }

bool StringField::match(StringField& other)
  {
  if (isPrintable() && other.hasValue())
    {
    if (!ByteString::match(other))
      {
      return false;
      }
    }
  //else
  return true;
  
  }


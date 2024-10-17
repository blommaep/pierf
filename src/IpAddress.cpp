// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "IpAddress.hpp"

#include <iostream> // for cout and cin
#include <fstream>
#include <sstream>
//#include <asm/bytorder.h>

void IpAddress::stringToVal(const char* inString) throw (Exception)
  {
  unsigned int i=0,j=0,k=0;
  mAddress.whole=0;
  char byte[4];

  while (i<16)
    {
    if (inString[i] == '.' || inString[i] == 0)
      {
      byte[k]=0;
      int value = atoi(byte);
      if (value > 255)
        {
        throw Exception("Reading a value > 255 in IP address: " + string(byte));
        }
      mAddress.chars[j]= (uchar) value;
      j++;
      k=0;
      if (inString[i] == 0)
        {
        i=16;
        }
      }
    else
      {
      byte[k] = inString[i];
      k++;
      if (k>3)
        {
        throw Exception("Reading a value > 255 in IP address: " + string(inString));
        }
      }
    i++;
    }   
  if (k>0)
    {
    throw Exception("IP address string too long. Must be four bytes (0-255) separated by . : " + string(inString));
    } 
  else if (j != 4)
    {
    throw Exception("Invalid IP address: failed to read exactly four bytes (0-255) separated by dots: " + string(inString));
    }
  }
  
IpAddress::IpAddress()
  {
  mAddress.whole=0;
  }

void IpAddress::setDefault(const char* inString) throw (Exception)
  {
  stringToVal(inString);
  wasManuallySet();
  }

void IpAddress::setManualFromValue(const char* inString) throw (Exception)
  {
  stringToVal(inString);
  wasManuallySet();
  }

void IpAddress::setAuto(const char* inString) throw (Exception)
  {
  stringToVal(inString);
  wasManuallySet();
  }

ulong IpAddress::getAddress()
  {
  return mAddress.whole;
  }

string IpAddress::getString()
  {
  stringstream retval; //creates an ostringstream object
  
  for (int i=0;i<3;i++)
    {
    retval << (unsigned int) mAddress.chars[i] << ".";
    }
  retval << (unsigned int) mAddress.chars[3] << flush;

  return(retval.str()); //returns the string form of the stringstream object
  }

bool IpAddress::getString(string& stringval)
  {
  if (hasValue())
    {
    stringval = getString();
    return true;
    }
  return false;
  }

uchar* IpAddress::copyTo(uchar* toPtr)
  {
  return copyTo((ulong*) toPtr);
  }

uchar* IpAddress::copyTo(ulong* toPtr)
  {
  *toPtr = mAddress.whole;
  toPtr++;
  return (uchar*) toPtr;
  }

bool IpAddress::analyze(uchar*& fromPtr, ulong& remainingSize)
  {
  if (remainingSize >= 4)
    {
    mAddress.whole = * ((ulong*) fromPtr);
    wasCaptured();
    remainingSize -= 4;
    fromPtr += 4;
    return true;
    }
  return false;
  }

bool IpAddress::match(IpAddress& other)
  {
  if (isPrintable() && other.hasValue())
    {
    if (mAddress.whole != other.mAddress.whole)
      {
      return false;
      }
    }
  //else
  return true;
  }


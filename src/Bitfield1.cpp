// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "Bitfield1.hpp"

#include <iostream> // for cout and cin
#include <iomanip>
#include <fstream>
#include <sstream>


Bitfield1::Bitfield1()
  :mOffset(0)
  {}

void Bitfield1::setVal(ushort val) noexcept(false)
  {
  if (val > 0x0001)
    {
    throw Exception("Invalid input value: value too high");
    }
  mData = val;
  }

void Bitfield1::setOffset(uchar offset) noexcept(false)
  {
  if (offset > 7)
    {
    throw Exception("Invalid offset for a 1 bit field. Maximum offset is 7");
    }
  mOffset = offset;
  }

uchar* Bitfield1::copyTo(uchar* toPtr)
  {
  // add zero's at the proper place
  uchar temp = mData << mOffset;

  // Two masks are needed to mask out the part of what is there that needs to be kept
  ushort mask1 = 0xFE << mOffset;
  ushort mask2 = 0xEF >> (7-mOffset);
  ushort mask = mask1 | mask2;
  *toPtr &= mask;

  // Now insert our value
  *toPtr |= temp;

  toPtr++;
  return toPtr;
  }


bool Bitfield1::analyze(uchar*& fromPtr, ulong32& remainingSize)
  {
  if (remainingSize < 1)
    {
    return false;
    }
  
  uchar temp = *fromPtr;
  temp >>= mOffset; // shift mOffset, so that it becomes the base value
  mData = temp & 0x01; // now, keep only the lower 4 bits.

  wasCaptured();
  return true;
  }


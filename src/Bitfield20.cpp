// Copyright (c) 2010, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "Bitfield20.hpp"

#include <iostream> // for cout and cin
#include <iomanip>
#include <fstream>
#include <sstream>


Bitfield20::Bitfield20()
  : mOffset(0)
  {
  }

void Bitfield20::setVal(ulong32 val) throw (Exception)
  {
  if (val > 0x000FFFFF)
    {
    throw Exception("Invalid input value: value too high");
    }
  mData = val;
  }

void Bitfield20::setOffset(uchar offset) throw (Exception)
  {
  if (offset > 7)
    {
    throw Exception("Invalid offset for a 13 bit field. Maximum offset is 7");
    }
  mOffset = offset;
  }

uchar* Bitfield20::copyTo(uchar* toPtr)
  {
  // add zero's at the proper place
  
  // Two masks are needed to mask out the part of what is there that needs to be kept
  if (mOffset > 4) // must be spread over 4 bytes
    {
    ulong32 temp = htonl32(mData << (12-mOffset));
    ulong32 mask1 = htonl32(0x00000FFF >> mOffset);
    ulong32 mask2 = htonl32(0xFFF00000 << (12-mOffset));
    mask1 |= mask2;

    ulong32* tmp = (ulong32*) toPtr;
    *tmp &= mask1;

    // Now insert our value
    *tmp |= temp;

    return toPtr+3; // last byte incomplete
    }
  else // must do byte per byte to assure not to write out of boundary
    {
    ulong32 temp = htonl32(mData << (12-mOffset));
    uchar *tmp = (uchar*) &temp; // here serves as source
    uchar mask1 = (uchar) 0xF0 << (4-mOffset);
    uchar mask2 = (uchar) 0x0F >> mOffset;

    *toPtr &=mask1;
    *toPtr |= *tmp;
    toPtr++; tmp++;
    *toPtr = *tmp; // second byte is always full byte
    toPtr++; tmp++;
    *toPtr &= mask2;
    *toPtr |= *tmp;
    if (mOffset == 4) // 3rd byte also finished
      {
      toPtr++;
      }
    return toPtr; 
    }
  }

bool Bitfield20::analyze(uchar*& fromPtr, ulong32& remainingSize)
  {
  if (remainingSize < 3)
    {
    return false;
    }
  if (mOffset > 4)
    {
    if (remainingSize < 4)
      {
      return false;
      }

    ulong32 temp = ntohl32(* ((ulong32*)fromPtr));
    temp >>= (12-mOffset); // shift mOffset, so that it becomes the base value
    mData = temp & 0x000FFFFF; // now, keep only the lower bits.
    fromPtr += 4;
    }
  else
    {
    ulong32 temp = 0;
    uchar* tmp = (uchar*) &temp;
    *tmp = *fromPtr;
    tmp++; fromPtr++;
    *tmp = *fromPtr;
    tmp++; fromPtr++;
    *tmp = *fromPtr;
    temp = ntohl32(temp);
    temp >>= (12-mOffset); // becomes base value
    mData = temp & 0x000FFFFF; // now, keep only the lower bits.
    if (mOffset == 4)
      {
      fromPtr++; // can move to next byte: third byte is also finished.
      }
    }

  wasCaptured();
  return true;  
  }


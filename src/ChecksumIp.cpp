// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "ChecksumIp.hpp"

ChecksumIp::ChecksumIp()
  : mPreCarry(0)
  {
  mData = 0;
  }

void ChecksumIp::addToSum(uchar* start, uchar* stop)
  {
  // Checksum is 1's complement. 
  // The advantage: little/big endian does not matter.
  // The algorithm: add as usual, but if result is e.g. 12345, add the 1 to the 2345.
  //                then take FFFF-x and that's it.

  unsigned short *loop=(unsigned short*) start;
  unsigned short *end=(unsigned short*) (stop-1); // Minus preventing to take one byte too much when the number of bytes is odd

  // In steps of sixteen bits
  while (loop<end)
    {
    mPreCarry += *loop;
    loop++;
    }

  if (loop == end) // odd number of bytes, add last byte
    {
    mPreCarry += htons(ushort(*(stop-1)));
    }

  }


void ChecksumIp::addToSum(ulong32 val)
  {
  uchar* tmp = (uchar*) &val;
  addToSum(tmp,tmp+4);
  }

void ChecksumIp::addToSum(ushort val)
  {
  mPreCarry += val;
  }

void ChecksumIp::calculateCarry()
  {
  while (mPreCarry >= 0x10000)
    {
    mPreCarry = (mPreCarry & 0xFFFF) + (mPreCarry / 0x10000);
    }

  mPreCarry = 0xFFFF - mPreCarry;
  mData = (unsigned short) mPreCarry;
  mData = ntohs(mData);
  }

void ChecksumIp::calculate(uchar* start, uchar* stop)
  {
  if (!isManual())
    {
    reset();
    addToSum(start,stop);
    calculateCarry();
    wasDefaulted();
    }
  }

void ChecksumIp::addPreCarry(ChecksumIp& other)
  {
  mPreCarry += other.mPreCarry;
  }

void ChecksumIp::reset()
  {
  mData = 0;
  mPreCarry = 0;
  }


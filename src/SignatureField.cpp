// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "SignatureField.hpp"

#include <iostream> // for cout and cin
#include <iomanip>
#include <fstream>
#include <sstream>


SignatureField::SignatureField()
  :mRxSignatureFound(false)
  {
  mData=0;
  }

uchar* SignatureField::copyTo(uchar* toPtr)
  {
  *toPtr++ = 0xCD;
  *toPtr++ = 0x40;
  ulong* tmp = (ulong*) toPtr;
  *tmp++ = htonl(mData);
  toPtr = (uchar*) tmp;
  *toPtr++ = 0xCD;
  *toPtr++ = 0x40;

  mData++; // increment the counter
  return toPtr;
  }

bool SignatureField::analyze(uchar*& fromPtr, ulong& remainingSize)
  {
  if (remainingSize < 8)
    {
    return false;
    }
 
  mRxSignatureFound = true;
  ushort signaturePreamble = ntohs(* (ushort*) fromPtr);
  if (signaturePreamble != 0xCD40)
    {
    mRxSignatureFound = false;
    return true; // if no signature detected, don't advance pointer, consider it an empty element
    }
  ushort signaturePostamble = ntohs(* (ushort*) (fromPtr+6));
  if (signaturePostamble != 0xCD40)
    {
    mRxSignatureFound = false;
    return true; // if no signature detected, don't advance pointer, consider it an empty element
    }

  fromPtr += 2; // else: next comes the counter value

  mData = ntohl(* (ulong*) fromPtr);
  fromPtr += 6;
  remainingSize -= 8;
  wasCaptured();
  return true;
  }

bool SignatureField::match(SignatureField& other)
  {
  if (other.hasValue())
    {
    if (mData != other.mData)
      {
      stringstream mismatch;
      mismatch << "Expect: " << (int) mData << " (" << getString() << "), ";
      mismatch << "Rx: " << (int) other.mData << " (" << other.getString() << ")";
      mismatch << flush;
      mMismatch = mismatch.str();
      mData = other.mData +1; // fix it
      return false;
      }
    }
  //else
  mData++; // increment for next match
  return true;
  }

bool SignatureField::copyVar() throw (Exception)
  {
  return true;
  }
  
bool SignatureField::isVar() const
  {
  return true;
  }



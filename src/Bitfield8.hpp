// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#ifndef BITFIELD8_HPP__
#define BITFIELD8_HPP__

#include "generics.hpp"
#include "Exception.hpp"
#include "Field.hpp"
#include <string>

using namespace std;

class Bitfield8: public Field
  {
  protected:
    uchar mData;
    uchar mOffset; // in case the 8 bits are not aligned with byte boundaries
    enum DisplayType {eHex, eDec, eChar}; //Hex, Decimal or ascii char display
    DisplayType mDisplayType;
    void stringToVal(const char* inString) noexcept(false);
    void setVal(ushort val) noexcept(false);
  public:
    Bitfield8();
    void setOffset(uchar offset) noexcept(false);
    void setDefault(const char* inString) noexcept(false);
    void setManualFromValue(const char* inString) noexcept(false);
    void setAuto(const char* inString) noexcept(false);
    void setDefault(const uchar inValue);
    void setManualFromValue(const uchar inValue);
    void setAuto(const uchar inValue);
    void displayDecimal();
    void displayChar();
    string getStringFromBinary() const;
    bool getStringFromBinary(string& stringval) const;
    uchar getValue();
    bool operator==(unsigned int value);
    bool operator!=(unsigned int value);
    uchar* copyTo(uchar* toPtr);
    bool analyze(uchar*& fromPtr, ulong32& remainingSize); 
    bool match(Bitfield8& other);
  };

#endif

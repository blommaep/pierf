// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#ifndef FLEXFIELD32_HPP__
#define FLEXFIELD32_HPP__

#include "generics.hpp"
#include "Exception.hpp"
#include "Field.hpp"
#include <string>

using namespace std;

// A flexfield is a field that is always there, but can be subdivided (or not) in different subfields that can be set individually and can have different names/meanings. Sample usage: see ICMP
class FlexField32: public Field
  {
  protected:
    union Flex32
      {
      ulong32  uLong;
      ushort uShort[2];
      uchar  uChar[4];
      };
    string mFieldNames[4];
    Var* mVars[4];
    struct ValueStates
      {
      ValueState mShort[2];
      ValueState mChar[4];
      };
    ValueStates mValueStates;
    Flex32 mData;
    void stringToVal(const char* inString) noexcept(false);
    uchar stringToChar(const char* val) noexcept(false);
    ushort stringToShort(const char* val) noexcept(false);
  public:
    FlexField32();
    enum How {eHowUndef, eHowDefault, eHowManual, eHowAuto, eHowCaptured};
    void setDefault(const char* inString) noexcept(false);
    void setManualFromValue(const char* inString) noexcept(false);
    void setAuto(const char* inString) noexcept(false);
    void setName(const char* inString);
    void setLong(const char* fieldname, ulong32 val, How how);
    void setLong(const char* fieldname, const char* val, How how) noexcept(false);
    void setShort(const char* fieldname, int pos, ushort val, How how) noexcept(false);
    void setShort(const char* fieldname, int pos, const char* val, How how) noexcept(false);
    void setChar(const char* fieldname, int pos, uchar val, How how) noexcept(false);
    void setChar(const char* fieldname, int pos, const char* val, How how) noexcept(false);
    uchar getChar(int pos);
    ushort getShort(int pos);
    ulong32 getLong();
    bool hasValue();
    string getStringFromBinary() const;
    bool getStringFromBinary(string& stringval) const;
    string getString() const;
    bool getString(string& stringval, const string& fieldname) const;
    uchar* copyTo(uchar* toPtr);
    bool analyze(uchar*& fromPtr, ulong32& remainingSize);     
    bool match(FlexField32& other);
    bool copyVar() noexcept(false);    
  protected:
    ValueState how2ValueState(How how);
  };

#endif

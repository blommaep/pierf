// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#ifndef STRINGFIELD_HPP__
#define STRINGFIELD_HPP__

#include "generics.hpp"
#include "Exception.hpp"
#include "Field.hpp"
#include "ByteString.hpp"
#include <string>

using namespace std;

class StringField: public Field, public ByteString
  {
  protected:
    void stringToVal(const char* inString) noexcept(false);
  public:
    virtual ~StringField();
    void setDefault(const char* inString) noexcept(false);
    void setManualFromValue(const char* inString) noexcept(false);
    void setAuto(const char* inString) noexcept(false);
    string getString() const; // return string, no checks
    string getStringFromBinary() const;
    bool getString(string& stringval) const;
    bool getStringFromBinary(string& stringval) const;    
    uchar* copyTo(uchar* toPtr);
    uchar* copyTo(uchar* toPtr, ulong32 maxSize); // copy max. maxSize bytes
    bool analyze(uchar*& fromPtr, ulong32& remainingSize); // consumes all of remainingSize
    bool analyze(uchar*& fromPtr, ulong32& remainingSize, ulong32 fieldSize); // consumes the specified amount of bytes
    bool match(StringField& other);
    };

#endif

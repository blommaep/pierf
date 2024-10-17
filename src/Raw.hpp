// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#ifndef RAW_HPP__
#define RAW_HPP__

#include "generics.hpp"
#include <string>
#include "Element.hpp"
#include "StringField.hpp"
#include "Bitfield16.hpp"

using namespace std;

class Raw: public Element
  {
  private:
    StringField mData;

    // Although they are not really fields, they are set by the user in the tag and impact the element content, so they need to be tracked as fields
    Bitfield16 mSize;
    StringField mFiller;
    Bitfield16 mType;
    bool mDataFieldEntered;
  public:
    Raw();
    void parseAttrib(const char** attr, AutoObject* parent, bool checkMandatory, bool storeAsString) throw (Exception);
    void addString(const char* inString) throw (Exception);
    void addString(const string& inString) throw (Exception);
    void setSize(ulong32 size);
    void setSize(const char* size);
    void setType(const char* type) throw (Exception);
    void setFiller(const char* filler);
    string getString();
    bool getString(string& stringval, const char* fieldName);
    ulong32 getSize();
    ulong32 getTailSize();
    bool copyVar() throw (Exception);
    uchar* copyTo(uchar* toPtr);
    uchar* copyTail(uchar* toPtr);
    bool analyze_Head(uchar*& fromPtr, ulong32& remainingSize);
    bool analyze_Tail(uchar*& fromPtr, ulong32& remainingSize);
    Element* analyze_GetNextElem();
    bool checkComplete();
    bool tryComplete(ElemStack& stack);
    string whatsMissing();
    bool match(Element* other);
    Element* getNewBlank();
    void copySize(Raw* fromRaw); // for packet match by compare method
  };

#endif

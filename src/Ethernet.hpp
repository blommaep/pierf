// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#ifndef ETHERNET_HPP__
#define ETHERNET_HPP__

#include "generics.hpp"
#include "MacAddress.hpp"
#include "Bitfield16.hpp"
#include "Element.hpp"
#include "Exception.hpp"

using namespace std;

class Ethernet: public Element
  {
  private:
    MacAddress mFrom;
    MacAddress mTo;
    Bitfield16 mEthertype; // Higher layer protocol
    bool checkComplete();
  public:
    Ethernet();
    Ethernet(MacAddress& from, MacAddress& to) noexcept(false);
    void parseAttrib(const char** attr, AutoObject* parent, bool checkMandatory, bool storeAsString) noexcept(false);
    // set interfaces only kept for backward compatibility. parseAttrib is the only function relevant for xml parsing and will also assure string save
    void setFrom(MacAddress& from);
    void setTo(MacAddress& to);
    void setFrom(const char* from);
    void setTo(const char* to);
    void setEthertype(const char* ethertype) noexcept(false);
    string getString();
    bool getString(string& stringval, const char* fieldName);
    ulong32 getSize();
    bool copyVar() noexcept(false);
    bool hasVar();
    uchar* copyTo(uchar* toPtr);
    uchar* copyTail(uchar* toPtr);
    bool analyze_Head(uchar*& fromPtr, ulong32& remainingSize);
    bool analyze_Tail(uchar*& fromPtr, ulong32& remainingSize);
    Element* analyze_GetNextElem();
    bool match(Element* other);
    ulong32 getTailSize();
    bool tryComplete(ElemStack& stack);
    string whatsMissing();
    Element* getNewBlank();
  };

#endif

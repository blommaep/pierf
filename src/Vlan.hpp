// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#ifndef VLAN_HPP__
#define VLAN_HPP__

#include "generics.hpp"
#include <string>
#include <vector>
#include "Element.hpp"
#include "Exception.hpp"
#include "Bitfield16.hpp"
#include "StringField.hpp"

using namespace std;

class Vlan: public Element
  {
  private:
    void stringToVal(const char* inString, int insertBefore) noexcept(false); // -1 to insert at the end
    vector<ushort> mVlans;
    StringField mVlanString;
    Bitfield16 mBodyEthertype;
    Bitfield16 mVlanEthertype;

    string getStackString();
  public:
    Vlan();
    Vlan(char* inString) noexcept(false);
    Vlan(string& inString) noexcept(false);    
    ~Vlan();
    void parseAttrib(const char** attr, AutoObject* parent, bool checkMandatory, bool storeAsString) noexcept(false);
    void setStack(const char* stack, bool storeAsString) noexcept(false);
    // setEthertype functions only kept for backward compatibility
    void setBodyEthertype(const char* ethertype) noexcept(false);
    void setVlanEthertype(const char* ethertype) noexcept(false);
    ushort getEthertype(); // Gives the ethertype of the tag, so what must be used by the lower layer (normally ethernet itself)
    string getString();
    bool getString(string& stringval, const char* fieldName);
    ulong32 getSize();
    ulong32 getTailSize();
    bool copyVar() noexcept(false);
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
  };

#endif

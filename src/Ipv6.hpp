// Copyright (c) 2010, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#ifndef IPV6_HPP__
#define IPV6_HPP__

#include "generics.hpp"
#include <string>
#include "Element.hpp"
#include "StringField.hpp"
#include "Ipv6Address.hpp"
#include "IpProtocol.hpp"
#include "Bitfield4.hpp"
#include "Bitfield8.hpp"
#include "Bitfield20.hpp"
#include "Bitfield16.hpp"
#include "IpProtocol.hpp"
#include "ChecksumIp.hpp"
#include "Exception.hpp"

using namespace std;

// Currently very basic, to be extended with introduction of other protocols that run on top

class Ipv6: public Element
  {
  private:
    Bitfield4 mVersion;
    Bitfield8 mTrafficClass; // dscp
    Bitfield20 mFlowLabel;
    Bitfield16 mPayloadLength; // ipv6 contentlength
    IpProtocol mNextHeader; //compatible with protocol field in ipv4
    Bitfield8 mHopLimit; // former ttl
    Ipv6Address mFromIp;
    Ipv6Address mToIp;

  public:
    Ipv6();
    void parseAttrib(const char** attr, AutoObject* parent, bool checkMandatory, bool storeAsString) throw (Exception);
    bool addPseudoHeaderChecksum(ChecksumIp& cksum);
    string getString();
    bool getString(string& stringval, const char* fieldName);
    ulong32 getSize();
    ulong32 getTailSize();
    bool copyVar() throw (Exception);
    uchar* copyTo(unsigned char* toPtr);
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

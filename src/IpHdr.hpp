// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#ifndef IPHDR_HPP__
#define IPHDR_HPP__

#include "generics.hpp"
#include <string>
#include "Element.hpp"
#include "StringField.hpp"
#include "IpAddress.hpp"
#include "IpProtocol.hpp"
#include "Bitfield3.hpp"
#include "Bitfield4.hpp"
#include "Bitfield6.hpp"
#include "Bitfield8.hpp"
#include "Bitfield13.hpp"
#include "Bitfield16.hpp"
#include "ChecksumIp.hpp"
#include "Exception.hpp"

using namespace std;

// Currently very basic, to be extended with introduction of other protocols that run on top

class IpHdr: public Element
  {
  private:
    IpAddress mFromIp;
    IpAddress mToIp;
    Bitfield4 mHeaderLength;
    Bitfield6 mDscp;
    Bitfield16 mContentLength; // size of everything underneath.
    Bitfield16 mPacketId;
    Bitfield13 mFragmentOffset;
    Bitfield3 mFlags;
    Bitfield8 mTtl;
    IpProtocol mProtocol;
    StringField mOptions;
    ChecksumIp mChecksum;

    ushort getTotalLength();
  public:
    IpHdr();
    void parseAttrib(const char** attr, AutoObject* parent, bool checkMandatory, bool storeAsString) noexcept(false);
    void setFrom(const char* fromIp) noexcept(false);
    void setTo(const char* toIp) noexcept(false);
    void setDscp(uchar dscp) noexcept(false);
    void setDscp(const char* dscpStr) noexcept(false);
    void setContentLength(ushort contentLength);
    void setContentLength(const char* contentLengthStr, bool storeAsString) noexcept(false);
    void setFlags(const char* flags, bool storeAsString) noexcept(false);
    void setPacketId(ushort packetId);
    void setPacketId(const char* packetId, bool storeAsString) noexcept(false);
    void setFragmentOffset(ushort fragmentOffset) noexcept(false);
    void setFragmentOffset(const char* fragmentOffset, bool storeAsString) noexcept(false);
    void setTtl(const char* ttl, bool storeAsString) noexcept(false);
    void setProtocol(uchar protocol);
    void setProtocol(const char* protocolStr) noexcept(false);
    void setOptions(const char* optionStr, bool storeAsString) noexcept(false);
    void setChecksumVal(const char* checksum, bool storeAsString) noexcept(false);
    bool addPseudoHeaderChecksum(ChecksumIp& cksum);
    string getString();
    bool getString(string& stringval, const char* fieldName);
    ulong32 getSize();
    ulong32 getTailSize();
    bool copyVar() noexcept(false);
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

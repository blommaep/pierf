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
#include "Bitfield3.hpp"
#include "Bitfield4.hpp"
#include "Bitfield6.hpp"
#include "Bitfield8.hpp"
#include "Bitfield13.hpp"
#include "Bitfield16.hpp"
#include "ChecksumIp.hpp"
#include "Exception.hpp"

using namespace std;

// Some fields have dedicated input parsing/output (print). Overload them here

class IpProtocol: public Bitfield8 
  {
  public:
    void setManualFromValue(const char* inString) throw (Exception);
    void setAuto(const char* inString) throw (Exception);
    void setDefault(const char* inString) throw (Exception);
    void setDefault(const uchar inValue)
      {
      Bitfield8::setDefault(inValue);
      }
    void setManualFromValue(const uchar inValue)
      {
      Bitfield8::setManualFromValue(inValue);
      }
    void setAuto(const uchar inValue)
      {
      Bitfield8::setAuto(inValue);
      }
    string getString();
    bool getString(string& stringval);
  };

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
    IpHdr(char* fromIp, char* toIp) throw (Exception);
    void parseAttrib(const char** attr, AutoObject* parent, bool checkMandatory) throw (Exception);
    void setFrom(const char* fromIp) throw (Exception);
    void setTo(const char* toIp) throw (Exception);
    void setDscp(uchar dscp) throw (Exception);
    void setDscp(const char* dscpStr) throw (Exception);
    void setContentLength(ushort contentLength);
    void setContentLength(const char* contentLengthStr) throw (Exception);
    void setFlags(const char* flags) throw (Exception);
    void setPacketId(ushort packetId);
    void setPacketId(const char* packetId) throw (Exception);
    void setFragmentOffset(ushort fragmentOffset) throw (Exception);
    void setFragmentOffset(const char* fragmentOffset) throw (Exception);
    void setTtl(const char* ttl) throw (Exception);
    void setProtocol(uchar protocol);
    void setProtocol(const char* protocolStr) throw (Exception);
    void setOptions(const char* optionStr) throw (Exception);
    void setChecksumVal(const char* checksum) throw (Exception);
    bool addPseudoHeaderChecksum(ChecksumIp& cksum);
    string getString();
    bool getString(string& stringval, const char* fieldName);
    ulong getSize();
    ulong getTailSize();
    bool copyVar() throw (Exception);
    uchar* copyTo(unsigned char* toPtr);
    uchar* copyTail(uchar* toPtr);
    bool analyze_Head(uchar*& fromPtr, ulong& remainingSize);
    bool analyze_Tail(uchar*& fromPtr, ulong& remainingSize);
    Element* analyze_GetNextElem();
    bool checkComplete();
    bool tryComplete(ElemStack& stack);
    string whatsMissing();
    bool match(Element* other);
  };

#endif

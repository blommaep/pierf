// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#ifndef UDP_HPP__
#define UDP_HPP__

#include "generics.hpp"
#include <string>
#include "Element.hpp"
#include "ChecksumIp.hpp"
#include "Exception.hpp"

using namespace std;

// Currently very basic, to be extended with introduction of other protocols that run on top

class Udp: public Element
  {
  private:
    Bitfield16 mSourcePort; 
    Bitfield16 mDestPort;
    Bitfield16 mLength;
    ChecksumIp mChecksum;
    ChecksumIp mIpPseudoHdrChecksum; // The part of the checksum that is calculated from ip pseudo header (stupidly enough, since ip has its own checksum)
    bool mChecksumIpRetrieved; // calculating the checksum goes in two steps: one part comes from the IP layer (pseudo header)...
    uchar* mChecksumPos; // position to insert checksum
    uchar* mContentStart; // used for checksum calculation
  public:
    Udp();
    void parseAttrib(const char** attr, AutoObject* parent, bool checkMandatory, bool storeAsString) noexcept(false);
    void setSourcePort(const char* sourcePort, bool storeAsString) noexcept(false);
    void setDestPort(const char* destPort, bool storeAsString) noexcept(false);
    void setLength(const char* length, bool storeAsString) noexcept(false);
    void setChecksum(const char* checkSum, bool storeAsString) noexcept(false);
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

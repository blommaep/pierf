// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#ifndef PACKET_HPP__
#define PACKET_HPP__

#include "generics.hpp"
#include <string>
#include <vector>
#include "Element.hpp"
#include "Exception.hpp"
#include "PlayStep.hpp"
#include "Shaper.hpp"
#include "Counter.hpp"

class Port;

using namespace std;

class Packet: public PlayStep
  {
  private:
    ElemStack mElems;
    Port* mOutPort; // default port
    uchar* mRawPacket;
    ulong32 mRawSize; // size of the raw packet
    ulong32 mAllocSize; // size of allocated buffer. May be larger then the actual raw size (in case of dynamic content)
    string mId; // The name given to the packet
    AutoComplete mAuto;
    bool mBinaryReady;
    bool mAnalysisReady;
    bool mHasVar; // copyVar needed every time
    Shaper* mShaper;
    Counter* mCounter;
    ulong32* mTxSignaturePos;
  public:
    Packet();
    Packet(char* id);
    ~Packet();
    // State sets and checks
    void setBinaryReady(bool ready);
    void setAnalysisReady(bool ready);
    bool getBinaryReady();
    bool getAnalysisReady();
    void setRawPacket(uchar* rawPacket, ulong32 rawSize) throw (Exception);
    void analyse() throw (Exception);
    bool match(Packet* otherPacket); // backward compatible, always binary match
    // Element and send tasks
    bool compare(Packet* otherPacket, bool matchByString); // no mixed mode currently, may need to merge these again later...?
    void push_back(Element* elem);
    bool tryComplete(bool final);
    void setPort(Port* outport);
    void sendTo(Port& outport) throw (Exception);
    void sendTo(Port* outport) throw (Exception);
    ulong32 getRawSize();
    void send() throw (Exception);
    void sendNoShaper() throw (Exception);
    void play() throw (Exception);
    void setCounter(Counter* counter);
    vector<Element*>::iterator begin();
    vector<Element*>::iterator end();
    string getString() const;
    string getElementsString() const;
    void setShaper(Shaper* shaper);
    Shaper* getShaper();
  };

#endif

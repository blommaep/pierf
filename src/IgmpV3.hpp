// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#ifndef IGMPV3_HPP__
#define IGMPV3_HPP__

#include "generics.hpp"
#include <string>
#include <vector>
#include "Element.hpp"
#include "IpAddress.hpp"
#include "Bitfield8.hpp"
#include "Bitfield1.hpp"
#include "Bitfield3.hpp"
#include "ChecksumIp.hpp"
#include "IgmpGroupRec.hpp"

using namespace std;

class IgmpV3: public Element
  {
  private:
    Bitfield8 mType;
    Bitfield8 mResponseTime;
    ChecksumIp mChecksum;

    // for the query only
    Bitfield1 mSFlag;
    Bitfield3 mQrv;
    Bitfield8 mQqic;
    IpAddress mMcastIp;
    Bitfield16 mNrRecords; // nrSources or nrGroupRecords
    vector<IpAddress> mSourceList; // for query
    vector<IgmpGroupRec*> mGroupList; // for report

    string getTypeString();
  public:
    IgmpV3();
    ~IgmpV3();
    void parseAttrib(const char** attr, AutoObject* parent, bool checkMandatory, bool storeAsString) throw (Exception);
    void setType(const char* type, bool storeAsString) throw (Exception);
    void setResponseTime(const char* responseTime) throw (Exception);
    void setChecksum(const char* checksum) throw (Exception);
    void setSFlag(const char* sflag, bool storeAsString) throw (Exception);
    void setQrv(const char* qrv, bool storeAsString) throw (Exception);
    void setQqic(const char* qqic, bool storeAsString) throw (Exception);
    void setMcastIp(const char* mcastIp, bool storeAsString) throw (Exception);
    void addSource(const char* srcIp, bool storeAsString) throw (Exception);
    IgmpGroupRec* addGroupRecord(const char* type, const char* mcastIp, bool storeAsString) throw (Exception);
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
    IpAddress getDestIp();
    uchar getType();
    bool match(Element* other);
    Element* getNewBlank();
  };

#endif

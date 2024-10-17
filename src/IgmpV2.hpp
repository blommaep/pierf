// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#ifndef IGMPV2_HPP__
#define IGMPV2_HPP__

#include "generics.hpp"
#include <string>
#include <vector>
#include "Element.hpp"
#include "IpAddress.hpp"
#include "Bitfield8.hpp"
#include "ChecksumIp.hpp"

using namespace std;

//// OVERLOADED CLASSES FOR DEDICATED FIELD INTERPRETATIONS ////

class IgmpV2Type: public Bitfield8
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

/////////////////// IGMP V2 ITSELF ///////////////////////

class IgmpV2: public Element
  {
  private:
    IgmpV2Type mMsgType;
    Bitfield8 mResponseTime;
    IpAddress mMcastIp;
    ChecksumIp mChecksum;

    string getTypeString();
  public:
    IgmpV2();
    void parseAttrib(const char** attr, AutoObject* parent, bool checkMandatory) throw (Exception);
    void setMsgType(const char* msgTypeStr) throw (Exception);
    void setMcastIp(const char* mcastIpStr) throw (Exception);
    void setChecksumValue(const char* checksum) throw (Exception);
    void setResponseTime(const char* responseTime);
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
    bool match(Element* other);
  };

#endif

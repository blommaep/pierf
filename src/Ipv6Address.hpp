// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#ifndef IPV6ADDRESS_HPP__
#define IPV6ADDRESS_HPP__

#include "generics.hpp"
#include "Exception.hpp"
#include "Field.hpp"
#include "ChecksumIp.hpp"
#include <string>

using namespace std;

class Ipv6Address: public Field
  {
  private:
    uchar mBytes[16];
    void stringToVal(const char* inString) throw (Exception);
  public:
    Ipv6Address();
    void setDefault(const char* inString) throw (Exception);
    void setManualFromValue(const char* inString) throw (Exception);
    void setAuto(const char* inString) throw (Exception);
    string getStringFromBinary() const;
    bool getStringFromBinary(string& stringval) const;
    uchar* copyTo(uchar* toPtr);
    bool analyze(uchar*& fromPtr, ulong& remainingSize); 
    bool match(Ipv6Address& other);
    void addToSum(ChecksumIp& cksum);
  };

#endif

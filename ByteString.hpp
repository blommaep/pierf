// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#ifndef BYTESTRING_HPP__
#define BYTESTRING_HPP__

#include "generics.hpp"
#include <string>
#include <vector>
#include "Exception.hpp"

using namespace std;

class ByteString: public vector<uchar>
  {
  private:
    void stringToVal(const char* inString) throw (Exception); // -1 to insert at the end
    bool mParseStrict;
    bool mInputChar; // true to store the input directly, as a text string, rather then to interprete it in a binary way.

  public:
    ByteString();
    ByteString(const char* inString) throw (Exception);
    ByteString(const string& inString) throw (Exception);
    void addString(const char* inString) throw (Exception);
    void addString(const string& inString) throw (Exception);
    void addBytes(const uchar* inBytes, ulong nrBytes);
    void resetString();
    string getString();
    uchar* copyTo(uchar* toPtr);
    uchar* copyTo(uchar* toPtr, ulong maxSize); // copy max. maxSize bytes
    void setStrict(bool strict);
    void setInputChar(bool asChar); 
    bool match(ByteString& other);
  };

#endif

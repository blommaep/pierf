// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#ifndef FIELDSTACK_HPP__
#define FIELDSTACK_HPP__

#include "generics.hpp"
#include <string>
#include <vector>
#include "Field.hpp"
#include "Port.hpp"
#include "Exception.hpp"

using namespace std;

class FieldRecord 
  {
  public:
    enum FieldAttrib {eHasDefault, eLastDummy};
    FieldRecord(Field* field, const char* fieldName);
    void setAttrib(FieldAttrib attrib);
    void clearAttrib(FieldAttrib attrib);
    Field* getField();
    bool hasAttrib(FieldAttrib attrib);
  private:
    Field* mField;
    string mName;
    bool mAttribs[eLastDummy];
  };

class FieldStack
  {
  private:
    vector<FieldRecord*> mRecords;
  public:
    FieldStack();
    ~FieldStack();
    FieldRecord* addField(Field* field, const char* fieldName);
  };

#endif

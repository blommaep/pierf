// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#ifndef ELEMENT_HPP__
#define ELEMENT_HPP__

#include "generics.hpp"
#include "ElemStack.hpp"
#include "AutoObject.hpp"
#include "VarAssignStep.hpp"
#include <string>

using namespace std;

class VarAssignSeq: public vector<VarAssignStep*>
  {
  };

class Element: public AutoObject
  {
  protected:
    AutoComplete mAuto;
    VarAssignSeq* mVarAssignSeq; // For assigning field values to variables
  public:
    Element();
    virtual ~Element();      
    virtual void parseAttrib(const char** attr, AutoObject* parent, bool checkMandatory, bool storeAsString) noexcept(false) = 0;
    virtual bool copyVar() noexcept(false) = 0; // Copy head (in network order)
    virtual uchar* copyTo(uchar* toPtr) = 0; // Copy head (in network order)
    virtual uchar* copyTail(uchar* toPtr) = 0; // Copy tail, if there is any
    virtual bool analyze_Head(uchar*& fromPtr, ulong32& remainingSize) = 0; // Analyze incoming packet, header part
    virtual bool analyze_Tail(uchar*& fromPtr, ulong32& remainingSize) = 0; // Analyze incoming packet, tailer part: analyzeTail must no take its input from fromPotr, but backward from fromPtr+remainingSize (- analyzed size). It must equally decrement remainingSize.
    virtual Element* analyze_GetNextElem() = 0; // After analyze_Head,give the next Element (layer). Return NULL if none could be identified
    virtual string getString() = 0; // Get the xml string representing the elem
    virtual bool getString(string& stringval, const char* fieldName) = 0; // return the value in string format
    virtual ulong32 getSize() = 0; // Size of the head part in the actual packet in bytes
    virtual ulong32 getTailSize() = 0; // Size of the tail part in the actual packet in bytes
    virtual bool checkComplete() = 0; // Confirm if the Element is complete
    virtual bool tryComplete(ElemStack& stack) = 0; // Try to complete the element
    virtual string whatsMissing() = 0; // Print what fields are missing
//    virtual bool match(Element* other) = 0; // Match the fields with an other element (of the same type)
    virtual bool match(Element* other) = 0; // Match the fields with an other element (of the same type)
    virtual Element* getNewBlank() = 0; // Must return a new Element of the same type (e.g Ethernet must return Ethernet* (pointing to a brand new Ethernet instance),...)
    VarAssignStep* getNewVarAssignStep();
    void playVarAssigns(Element* receivedElem); 
    bool hasVarAssigns() const;
    string getVarAssignsString() const;
  };

#endif

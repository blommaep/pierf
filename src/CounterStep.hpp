// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#ifndef COUNTERSTEP_HPP__
#define COUNTERSTEP_HPP__

#include "generics.hpp"
#include "PlayStep.hpp"
#include "ReceiveStep.hpp"
#include "Counter.hpp"

using namespace std;

class CounterStep: public PlayStep
  {
  private:
    ReceiveStep* mReceiveStep; // The receive of which this mirror is part. Needed because only the ReceiveStep knows what packet was received...
    Counter* mCounter;
    enum CounterAction {eIncrement, eReset, eHold, eCont, eReport};
    CounterAction mAction;
    ulong32 mValue; 
    ulong32 mBytesValue;
    Var* mActionVar;
  public:
    CounterStep();
    ~CounterStep();
    void setReceiveStep(ReceiveStep* receiveStep);
    void setCounter(Counter* counter);
    void setAction(char* action);
    void setAction(CounterAction action);
    void setValue(const char* value) throw (Exception); // May be used with reset/increment, default=0 resp. 1
    void setBytesValue(const char* bytesValue) throw (Exception);
    void setVar(const char* varName) throw (Exception); // Alternative to fixed value, use var
    void play();
    string getString() const;
  };

#endif

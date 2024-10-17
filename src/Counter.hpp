// Copyright (c) 2006-2011, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#ifndef COUNTER_HPP__
#define COUNTER_HPP__

#include "generics.hpp"
#include <vector>

#include "Var.hpp"
#include <sys/time.h>

using namespace std;

class Counter: public Var
  {
  private:
    ulong32 mCount;
    unsigned long long mByteCount;
    struct timeval mStartTime;
    struct timeval mStopTime;
    struct timeval mTotalTime;
    bool mRunning;
    void doTimeClick(); // Do the actual update of mTotalTime
  public:
    Counter(char* name);
    Counter(string& name);
    ~Counter();
    void reset();
    void reset(ulong32 newCount);
    void increment(ulong32 incr, ulong32 sizeIncr);
    void overrule(ulong32 nrPackets, ulong32 totalSize);
    void overruleXl(ulong32 nrPackets, unsigned long long totalSize);
    void timeClick(); //update mTotalTime and restart with a new StartTime
    void timeClick(struct timeval pktTime); //idem, but using the packet capture time, provided as argument
    void hold();
    void hold(struct timeval pktTime);
    void cont();
    void cont(struct timeval pktTime);
    ulong32 getCount();
    ulong32 getByteCount();
    ulong32 getRate(); // packets per second. no decimals
    ulong32 getBitrate(); // bits per second. 
    double getTotalTime();
    string getBitrateString(); // return bitrate as a string
    void setStringValue(const char* inString) throw (Exception);
    string getStringValue();
    string getString() const;
  };

#endif

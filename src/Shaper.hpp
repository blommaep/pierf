// Copyright (c) 2006-2011, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#ifndef SHAPER_HPP__
#define SHAPER_HPP__

#include "generics.hpp"
#include <vector>
#include <string>

#include <sys/time.h>

using namespace std;

class Shaper
  {
  private:
    string mName;
    string mRateString;
    double mByteInterval; // in seconds
    bool mPacketRate; // when true, mByteInterval and mRate actually represent packet interva/rate
    double mRate; // Just for information: also keep the rate value
    struct timeval mPrevTime; // (approx) send time of the previous packet
    struct timeval mPrev2Time; // send time of the packet before the previous packet
    ulong32 mPreviousSize;
    ulong32 mLastCalculatedSize;
    ulong32 mLastCalculatedSeconds;
    ulong32 mLastCalculatedMicroseconds;
    void swap(struct timeval& curTime, ulong32 size);
  public:
    Shaper(const char* name);
    Shaper(string& name);
    ~Shaper();
    void setRate(const char* rateStr) throw (Exception);
    void getShapeDelay(const ulong32 size, struct timeval& delay, struct timeval& sendTime, struct timeval& curTime);
    void shapeDelay(const ulong32 size, struct timeval& delay, struct timeval& sendTime, struct timeval& curTime);
    void shape(const ulong32 size);

    void reset();

    string getName();
    string getString() const;
  };

#endif

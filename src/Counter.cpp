// Copyright (c) 2006-2011, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "Counter.hpp"
#include <sstream>

Counter::Counter(char* name)
  :Var(name)
  {
  mRunning = false; // Will be started by cont()
  reset();
  cont();
  }

Counter::Counter(string& name)
  :Var(name)
  {
  mRunning = false; // Will be started by cont()
  reset();
  cont();
  }

Counter::~Counter()
  {
  }

void Counter::reset()
  {
  mCount = 0;
  mByteCount = 0;
  mTotalTime.tv_sec=0;
  mTotalTime.tv_usec=0;
  }

void Counter::reset(ulong32 newCount)
  {
  reset();
  mCount = newCount;
  }

void Counter::increment(ulong32 incr, ulong32 sizeIncr)
  {
  mCount += incr;
  mByteCount += sizeIncr;
  }

void Counter::overrule(ulong32 nrPackets, ulong32 totalSize)
  {
  mCount = nrPackets;
  mByteCount = totalSize;
  }

void Counter::overruleXl(ulong32 nrPackets, unsigned long long totalSize)
  {
  mCount = nrPackets;
  mByteCount = totalSize;
  }

void Counter::timeClick()
  {
  if (not(mRunning)) // hold while not running is ignored
    {
    return; // Not running anymore, so no need for update
    }

  struct timezone tz;
  gettimeofday(&mStopTime, &tz);
  doTimeClick();
  }

void Counter::timeClick(struct timeval pktTime)
  {
  if (not(mRunning)) // hold while not running is ignored
    {
    return; // Not running anymore, so no need for update
    }
  mStopTime = pktTime;
  doTimeClick();
  }

void Counter::doTimeClick()
  {
  // Algorithm as illustrated on http://www.delorie.com/gnu/docs/glibc/libc_428.html
  /* Perform the carry for the later subtraction by updating y. */
  if (mStopTime.tv_usec < mStartTime.tv_usec) 
    {
    int nsec = (mStartTime.tv_usec - mStopTime.tv_usec) / 1000000 + 1;
    mStartTime.tv_usec -= 1000000 * nsec;
    mStartTime.tv_sec += nsec;
    }
  if (mStopTime.tv_usec - mStartTime.tv_usec > 1000000) 
    {
    int nsec = (mStopTime.tv_usec - mStartTime.tv_usec) / 1000000;
    mStartTime.tv_usec += 1000000 * nsec;
    mStartTime.tv_sec -= nsec;
    }

  /* Compute the time remaining to wait.
     tv_usec is certainly positive. */
  mTotalTime.tv_sec += (mStopTime.tv_sec - mStartTime.tv_sec);
  mTotalTime.tv_usec += (mStopTime.tv_usec - mStartTime.tv_usec);
  if (mTotalTime.tv_usec > 1000000)
    {
    mTotalTime.tv_sec += 1;
    mTotalTime.tv_usec -= 1000000;
    }

  mStartTime.tv_sec = mStopTime.tv_sec;
  mStartTime.tv_usec = mStopTime.tv_usec;
  }

void Counter::hold()
  {
  timeClick();
  mRunning = false;
  }

void Counter::hold(struct timeval pktTime)
  {
  if (not(mRunning)) // hold while not running is ignored
    {
    return; // Not running anymore, so no need for update
    }
  mStopTime = pktTime;
  doTimeClick();
  mRunning = false;
  }

void Counter::cont()
  {
  if (not(mRunning)) // cont while still running is ignored
    {
    struct timezone tz;
    gettimeofday(&mStartTime, &tz);
    mRunning = true;
    }
  }

void Counter::cont(struct timeval pktTime)
  {
  if (not(mRunning)) // cont while still running is ignored
    {
    mStartTime = pktTime;
    mRunning = true;
    }
  }

ulong32 Counter::getCount()
  {
  return mCount;
  }

ulong32 Counter::getByteCount()
  {
  return mByteCount;
  }

ulong32 Counter::getRate() 
  {
  if (mTotalTime.tv_sec == 0 && mTotalTime.tv_usec == 0) // Have been measuring for 0 time (so not)
    {
    return 0;
    }

  // calculating in doubles to easily cover all cases. On modern processers, this shouldn't be too expensive
  double time = mTotalTime.tv_usec;
  time = time/1000000; // microsecs
  time += (double) mTotalTime.tv_sec;

  // adding time/2 to do a correct rounding, relevant with a small rate
  int rate = int((double(mCount) + (time/2))/time);

  return rate;
  }
   
ulong32 Counter::getBitrate()
  {
  if (mTotalTime.tv_sec == 0 && mTotalTime.tv_usec == 0) // Have been measuring for 0 time (so not)
    {
    return 0;
    }

  // calculating in doubles to easily cover all cases. On modern processers, this shouldn't be too expensive
  double time = mTotalTime.tv_usec;
  time = time/1000000; // microsecs
  time += (double) mTotalTime.tv_sec;

  // adding time/2 to do a correct rounding, relevant with a small rate
  // Multiply by 8 to get  bitrate rather then byterate
  return ulong32(((double(mByteCount) * 8) + (time/2)) /time);
  }

string Counter::getBitrateString() // return bitrate as a string
  {
  if (mTotalTime.tv_sec == 0 && mTotalTime.tv_usec == 0) // Have been measuring for 0 time (so not)
    {
    return 0;
    }

  // calculating in doubles to easily cover all cases. On modern processers, this shouldn't be too expensive
  double time = mTotalTime.tv_usec;
  time = time/1000000; // microsecs
  time += (double) mTotalTime.tv_sec;

  double rate = ((double(mByteCount) * 8) + (time/2)) /time;

  int scale = 0;
  while (rate > 1000)
    {
    rate = rate / 1000;
    scale++;
    }
  char scaleToKey[] = " kMGT";
  char scaleChar = scaleToKey[scale];
  
  stringstream retval;
  retval << rate << " " << scaleChar << "bps" << flush;
  return retval.str();
  }

double Counter::getTotalTime()
  {
  double time = mTotalTime.tv_usec;
  time = time/1000000; // microsecs
  time += (double) mTotalTime.tv_sec;
 
  return time; 
  }

void Counter::setStringValue(const char* inString) throw (Exception)
  {
  mCount = textToLong(inString);
  }

string Counter::getStringValue()
  {
  return intToString((int) mCount);
  }

string Counter::getString() const
  {
  stringstream retval;
  retval << "<var type=\"counter\" id=\"" << mName << "\"";
  retval << "/>" << endl << flush;
  return retval.str();
  }

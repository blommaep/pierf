// Copyright (c) 2006-2011, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "Shaper.hpp"
#include "zthread/Thread.h"
#include <iostream> // for cout and cin
#include <sstream>

#include <stdio.h>
#include <time.h>
#include <sys/time.h>

Shaper::Shaper(const char* name)
  : mPacketRate(false), mLastCalculatedSize(0) 
  {
  mName = name;

  reset();
  }

Shaper::Shaper(string& name)
  : mPacketRate(false), mLastCalculatedSize(0)
  {
  mName = name;
  reset();
  }

Shaper::~Shaper()
  {
  }

void Shaper::setRate(const char* rateStr) noexcept(false)
  {
  char* ratePos = (char*) rateStr;
  int decimals = 0;
  int nrDigits = 0;
  int multi = 1;
  char digits[9]; // max 8 digits + 0
  mRateString = rateStr; // for later print use only

  while (*ratePos != 0)
    {
    if (*ratePos == '0')
      {
      if (nrDigits > 0)
        {
        digits[nrDigits++] = *ratePos;
        }
      else if (decimals > 0)
        {
        decimals ++;
        }
      }
    else if (*ratePos > '0' && *ratePos <='9')
      {
      digits[nrDigits++] = *ratePos;
      }
    else if (*ratePos == '.')
      {
      if (decimals > 0)
        {
        throw Exception("Dot (.) is interpreted as a decimal separator. Hence, there may be only one dot in a value. Found two.");
        }
      decimals ++; // Will be set to 1, although te first decimal is still to be read. Will be compensated later
      }
    else if (*ratePos == 'G' || *ratePos == 'M' || *ratePos == 'k' || *ratePos == 'b' || *ratePos =='f')
      {
      if (multi != 1)
        {
        throw Exception("Bad rate format. Problems reading \"M\" where not expected.");
        }
      if (*ratePos == 'G')
        {
        multi = 1024*1024*1024;
        ratePos++;
        }
      else if (*ratePos == 'M')
        {
        multi = 1024*1024;
        ratePos++;
        }
      else if (*ratePos == 'k')
        {
        multi = 1024; 
        ratePos++;
        }
      digits[nrDigits] = 0; // end of this string

      // simple bps will fall through to here
      if (*ratePos == 'b')
        {
        }
      else if (*ratePos == 'B')
        {
        multi = multi*8;
        }
      else if (*ratePos == 'f') //frames per second
        {
        mPacketRate = true;
        }
      else
        {
        throw Exception("Bad rate format. Expecting b, B or f at: " + string(ratePos));
        }
      ratePos++;
      if (*ratePos != 'p' && ratePos[1] != 's')
        {
        throw Exception("Bad rate format. Expecting ps at: " + string(ratePos));
        }
      ratePos ++; // skip also the s (move one position here and one position because of the loop
      }
    else
      {
      throw Exception("Bad rate format. Unexpected charater read at:" + string(ratePos));
      }

    if (nrDigits > 8)
      {
      throw Exception ("Cannot handle a rate precision higher then 8 digits.");
      }
    ratePos++;
    }

  mRate = textToLong(digits);
  mRate *= multi;
  if (decimals > 1)
    {
    decimals--;
    mRate /= decimals;
    }

  if (!mPacketRate)
    {
    mByteInterval = 8/(mRate); //seconds per Byte, to be multiplied by nr of bytes
    }
  else
    {
    mByteInterval = 1/(mRate); //seconds betweens packets (probably a fraction of a second...)
    }
  }

void Shaper::swap(struct timeval& curTime, ulong32 size)
  {
  mPrev2Time = mPrevTime;
  mPrevTime = curTime;
  if (!mPacketRate) // else: keep 0
    {
    mPreviousSize = size;
    }
  }

void Shaper::getShapeDelay(const ulong32 size, struct timeval& delay, struct timeval& sendTime, struct timeval& curTime)
  {
  // Effectively shaping over two packets, in this way somehow taking into account the packet sending delay and being more accurate.

//  To get the current time:  (required as input)
//
//  struct timezone tz;
//  struct timeval curTime; 
//  gettimeofday(&curTime, &tz);

  // default, used for immediate sending
  delay.tv_sec=0;
  delay.tv_usec=0;
  sendTime = curTime;

//  cout << curTime.tv_sec << ":" <<curTime.tv_usec << " Start pkt" << endl;
  double packetInterval;
  struct timeval compareTime;

  // First check if there were previous packets 
  if (mPreviousSize == 0) // There was no previous packet or packet rate
    {
    if (mPacketRate)
      {
      if (mPrev2Time.tv_sec == 0) // first and second packet
        {
        compareTime = mPrevTime;
        packetInterval = mByteInterval; // actually packet interval
        }
      else
        {
        compareTime = mPrev2Time;
        packetInterval = mByteInterval * 2; // actually packet interval
        }
      }
    else // No packet rate => there was no previous packet - send immediately
      {
      return; // !!!! RETURN 
      }
    }
  else
    {
    if (mPrev2Time.tv_sec == 0) // This is the second packet, can only take into account the previos time
      {
      packetInterval = size*mByteInterval;
      compareTime = mPrevTime;
      }
    else
      {
      packetInterval = (size + mPreviousSize) * mByteInterval;
      compareTime = mPrev2Time;
      }
    }

  // Calculate the sum comparetime + packetInterval == time to send
  long seconds = (ulong32) packetInterval;
  long microseconds = (ulong32) ((packetInterval - seconds) * 1000000);
  
  // sum
  microseconds += compareTime.tv_usec;
  if (microseconds > 1000000)
    {
    seconds += microseconds / 1000000;
    microseconds -= 1000000;
    }
  seconds += compareTime.tv_sec;

  // If the time to send is in the past: send immeditaly
  if (curTime.tv_sec > seconds) // send immediately: time long passed
    {
    return;
    }
  else
    {
    if (curTime.tv_sec == seconds)
      {
      if (curTime.tv_usec > microseconds)
        {
        return;
        }
      }
    }


  // Storing the target time for possible active loop
  sendTime.tv_sec = seconds;
  sendTime.tv_usec = microseconds;

  // subtract current time from the target time. That gives the needed delay
  // Variation on algorithm as illustrated on http://www.delorie.com/gnu/docs/glibc/libc_428.html
  // Perform the carry for the later subtraction by updating y. 
  if (microseconds < curTime.tv_usec)
    {
    int nsec = (curTime.tv_usec - microseconds)/1000000 + 1;
    microseconds = microseconds + (1000000 * nsec);
    seconds -= nsec;
    }

  seconds -= curTime.tv_sec;
  microseconds -= curTime.tv_usec;

  delay.tv_sec = seconds;
  delay.tv_usec = microseconds;

  }


void Shaper::shapeDelay(const ulong32 size, struct timeval& delay, struct timeval& sendTime, struct timeval& curTime)
  {
  if (delay.tv_sec == 0 && delay.tv_usec == 0)
    {
    swap(curTime,size);
    return;
    }

  bool highPrecision = true; // if the time between the packets is sufficiently high, we don't care about high precision and will set this to false.
  if (delay.tv_sec > 0)
    {
    highPrecision = false;
    }  
  if (delay.tv_usec > 100000)
    {
    highPrecision = false;
    }

  ulong32 milliseconds = delay.tv_sec * 1000 + delay.tv_usec / 1000;

  if (milliseconds > 10) // Maybe there's only microseconds delay?
    {
//    cout << "sleeping " << milliseconds << endl;
    ZThread::Thread::sleep(milliseconds-10);
    }

//  gettimeofday(&curTime, &tz);
  if (highPrecision) // This is a short sleep, want more precision => active loop
    {
    while (true)
      {
      if (curTime.tv_sec > sendTime.tv_sec)
        {
        break; // EXIT LOOP
        }
      if (curTime.tv_sec == sendTime.tv_sec) // most likely
        {
        if (curTime.tv_usec > sendTime.tv_usec)
          {
          break; // EXIT LOOP
          }
        }

      struct timezone tz;
      gettimeofday(&curTime, &tz);
      }
    }

  swap(sendTime,size); // currently using the wanted time and not real time because getting the real time again seems to cause too high delays... (inaccuracy of approx. 10 ms)
  }

void Shaper::shape(const ulong32 size)
  {
  struct timezone tz;
  struct timeval curTime; 
  gettimeofday(&curTime, &tz);
  struct timeval delay;
  struct timeval sendTime;

  getShapeDelay(size, delay, sendTime, curTime);
  shapeDelay(size, delay, sendTime, curTime);
  }


void Shaper::reset()
  {
  mPreviousSize = 0;;
  mPrevTime.tv_sec=0;
  mPrevTime.tv_usec=0;
  mPrev2Time.tv_sec=0;
  mPrev2Time.tv_usec=0;
  }


string Shaper::getName()
  {
  return mName;
  }

string Shaper::getString() const
  {
  stringstream retval;
  retval << "<shaper id=\"" << mName << "\" rate=\"" << mRateString << "\" />";

  retval << endl << flush;
  return retval.str();    
  }

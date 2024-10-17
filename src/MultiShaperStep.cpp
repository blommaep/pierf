// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "MultiShaperStep.hpp"

#include <iostream> // for cout and cin
#include <fstream>
#include "zthread/Thread.h"

MultiShaperStep::MultiShaperStep()
  {
  }

MultiShaperStep::~MultiShaperStep()
  {
  }

void MultiShaperStep::addPacket(Packet* packet) throw (Exception)
  {
  Shaper* shaper = packet->getShaper();
  if (shaper == NULL)
    {
    throw Exception("No shaper attached to the <packet>. Cannot add it to <multishaper>.");
    }

  deque<Packet*>::iterator iterPacket;
  for(iterPacket = mPackets.begin(); iterPacket != mPackets.end(); iterPacket++)
    {
    Packet* iPacket = *iterPacket;
    Shaper* iShaper = iPacket->getShaper();
    if (iShaper == shaper) // also NULL may only occur once
      {
      throw Exception("Found another Packet that is using the same shaper in a <multishaper>. This is not supported.");
      }
    }
  mPackets.push_back(packet);
  }

void MultiShaperStep::play()
  {
  struct timezone tz;
  struct timeval curTime; 
  gettimeofday(&curTime, &tz);
  struct timeval minSendTime;
  struct timeval minDelay;
  minDelay.tv_sec = -1;
  ulong minSize = 0;
  Packet* minPacket = NULL;

  deque<Packet*>::iterator iterPacket;
  for(iterPacket = mPackets.begin(); iterPacket != mPackets.end(); iterPacket++)
    {
    Packet* packet = *iterPacket;
    Shaper* shaper = packet->getShaper();
    ulong size = packet->getRawSize();
    struct timeval delay;
    struct timeval sendTime;
    shaper->getShapeDelay(size, delay, sendTime, curTime);
    if ((ulong) delay.tv_sec < (ulong) minDelay.tv_sec)
      {
      minDelay = delay;
      minPacket = packet;
      minSendTime = sendTime;
      minSize = size;
      }
    else if (delay.tv_sec == minDelay.tv_sec)
      {
      if (delay.tv_usec < minDelay.tv_usec)
        {
        minDelay = delay;
        minPacket = packet;
        minSendTime = sendTime;
        minSize = size;
        }
      }
    }

  if (minPacket != NULL)
    {
    Shaper* shaper = minPacket->getShaper();
    shaper->shapeDelay(minSize,minDelay,minSendTime,curTime);
    minPacket->sendNoShaper();
    }
  else
    {
    throw Exception("CODE BUG... DIDN'T FIND APPROPRIATE PACKET TO SEND???");
    }
  }


// Copyright (c) 2006-2011, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "ReceiveStep.hpp"

#include <iostream> // for cout and cin
#include <fstream>
#include <sstream>
#include "zthread/Thread.h"

ReceiveStep::ReceiveStep()
  : mPort(NULL), mPacket(NULL), mNomatchLoop(false), mMatched(false)
  {
  }

ReceiveStep::~ReceiveStep()
  {
  delete mPacket; // must not exist afte receive. Packet must be copied if needed outside the receive
  }

void ReceiveStep::setPort(Port* port)
  {
  mPort = port;
  }

void ReceiveStep::setNomatchLoop(bool loop)
  {
  mNomatchLoop = loop;
  }

void ReceiveStep::play()
  {
  do  
    {
    mPort->waitReceive();
    mRxPacket = mPort->receive();
    Seq::play();
    if (mPacket != NULL)
      {
      delete mPacket; // Packet only valid within the receive step. This step has now finished
      mPacket = NULL;
      }
    }
  while (mNomatchLoop && !mMatched);
  mMatched = false; // at the end of the ReceiveStep, the receive packet is deleted and the match is reset, in case we are looping...
  }

void ReceiveStep::matched()
  {
  mMatched = true;
  }

Packet* ReceiveStep::getAnalyzePacket()
  {
  if (mPacket == NULL)
    {
    mPacket = new Packet;
    mPacket->setRawPacket(mRxPacket.getPacket(),mRxPacket.size());
    }

  return mPacket;
  }

uchar* ReceiveStep::getPacket()
  {
  return mRxPacket.getPacket();
  }

ulong32 ReceiveStep::getPacketSize()
  {
  return mRxPacket.size();
  }

struct timeval ReceiveStep::getPktTime()
  {
  return mRxPacket.getPktTime();
  }

string ReceiveStep::getString() const
  {
  stringstream retval;
  retval << "<receive port=\"" << mPort->getName() << "\" ";

  if (mNomatchLoop)
    {
    retval << "nomatch=\"loop\" ";
    }

  retval << ">" << endl;

  retval << getSeqElementsString();

  retval << "</receive>" << endl << flush;
  return retval.str();
  }

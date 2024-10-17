// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "FirstOfStep.hpp"

#include <iostream> // for cout and cin
#include <fstream>
#include "zthread/Thread.h"

FirstOfStep::FirstOfStep()
  : mReceiveStep(NULL), mCurrentPacket(NULL), mMatched(false)
  {
  }

FirstOfStep::~FirstOfStep()
  {
  vector<MatchStep *>::iterator iter;
  for (iter = mMatches.begin();iter != mMatches.end() && !mMatched;iter++)
    {
    MatchStep* elem= *iter;
    delete elem;
    }
  }

void FirstOfStep::setReceiveStep(ReceiveStep* receiveStep)
  {
  mReceiveStep = receiveStep;
  }

void FirstOfStep::addMatchStep(MatchStep* matchStep)
  {
  mMatches.push_back(matchStep);
  matchStep->setFirstOfStep(this);
  }

Packet* FirstOfStep::getCurrentPacket()
  {
  return mCurrentPacket;
  }

void FirstOfStep::matched() // called by the MatchStep to indicate a match
  {
  mMatched = true;
  mReceiveStep->matched();
  }

void FirstOfStep::play()
  {
  mCurrentPacket = mReceiveStep->getAnalyzePacket();
  mCurrentPacket->analyse();

  vector<MatchStep *>::iterator iter;
  for (iter = mMatches.begin();iter != mMatches.end() && !mMatched;iter++)
    {
    MatchStep* elem= *iter;
    elem->play();
    }
  mCurrentPacket = NULL;
  mMatched = false; //at the end of running, reset it, to be ready for a possible next loop...
  }


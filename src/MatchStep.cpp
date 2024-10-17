// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "MatchStep.hpp"
#include "FirstOfStep.hpp"

#include <iostream> // for cout and cin
#include <fstream>
#include <string.h>
#include "zthread/Thread.h"


MatchStep::MatchStep()
  : mMatchPacket(NULL), mMatchByString(false), mMatchMethod(eAnalyze)
  {
  }

MatchStep::~MatchStep()
  {
  if (mMatchPacket != NULL)
    {
    delete mMatchPacket; 
    }
  }

void MatchStep::setMatchPacket(Packet* matchPacket)
  {
  mMatchPacket = matchPacket;
  }

void MatchStep::setFirstOfStep(FirstOfStep* firstOfStep)
  {
  mFirstOfStep = firstOfStep;
  }

void MatchStep::setMatchByString(const char* byString)
  {
  if (!strcmp(byString,"true"))
    {
    mMatchByString = true;
    }
  else if (!strcmp(byString,"false"))
    {
    mMatchByString = false;
    }
  else
    {
    throw Exception("Value of byString attribute must be \"true\" or \"false\" in match tag.");
    }
  }

void MatchStep::setMatchMethod(const char* matchMethod)
  {
  if (!strcmp(matchMethod,"analyze"))
    {
    mMatchMethod = eAnalyze;
    }
  else if (!strcmp(matchMethod,"compare"))
    {
    mMatchMethod = eCompare;
    }
  else
    {
    throw Exception("Value of matchMethod attribute must be \"analyze\" or \"compare\" in match tag.");
    }
  }

bool MatchStep::getMatchByString()
  {
  return mMatchByString;
  }

void MatchStep::play()
  {
  Packet* receivedPacket = mFirstOfStep->getCurrentPacket();
  if (mMatchMethod == eAnalyze)
    {
    if (mMatchPacket->match(receivedPacket))
      {
  //    cout << "Packet match" << endl;
      mFirstOfStep->matched();  
      Seq::play();
      }
    }
  else // eCompare
    {
    Packet* comparePacket = mFirstOfStep->getAnalyzePacket(); // every time a new packet
    if (comparePacket->compare(mMatchPacket,mMatchByString)) // the tempory captured packet (comparePacket) runs the compare because it has the binary
      {
      mFirstOfStep->matched();  
      Seq::play();
      }
    delete comparePacket;
    }
  }


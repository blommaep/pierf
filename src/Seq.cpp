// Copyright (c) 2006-2011, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "Seq.hpp"

#include <iostream> // for cout and cin
#include <fstream>
#include <sstream>
#include <typeinfo>

Seq::Seq()
  :mAuto (eAutoEnherit), mRepeat(1)
  {
  }

Seq::~Seq()
  {
  vector<PlayStep *>::iterator iter;
  for (iter = mPlaySteps.begin();iter != mPlaySteps.end();iter++)
    {
    PlayStep* playStep= *iter;
    delete playStep;
    }
  }

void Seq::push_back(Packet& packet)
  {
  PlayStep* playstep = &packet;
  mPlaySteps.push_back(playstep);
  }

void Seq::push_back(PlayStep* playstep)
  {
  mPlaySteps.push_back(playstep);
  }

void Seq::setRepeat(char* repeat) noexcept(false)
  {
  mRepeat = textToLong(repeat);
  }

void Seq::play()
  {
  vector<PlayStep *>::iterator iter;
  ulong32 i;
  for (i=0; i<mRepeat; i++)
    {
    for (iter = mPlaySteps.begin();iter != mPlaySteps.end();iter++)
      {
      PlayStep* elem= *iter;
      elem->play();
      }
    }
  }

int Seq::size() const
  {
  return mPlaySteps.size();
  }

void Seq::setName(char* name)
  {
  mName = name;
  }

bool Seq::isRef()
  {
  return mName.size() > 0;
  }

string Seq::getName()
  {
  return mName;
  }

string Seq::getString() const
  {
  stringstream retval;
  retval << "<seq ";

  if (mName.size() > 0)
    {
    retval << "id=\"" << mName << "\" ";
    }

  if (mRepeat != 1)
    {
    retval << "repeat=\"" << mRepeat << "\" ";
    }

  retval << ">" << endl;

  vector<PlayStep *>::const_iterator iter;
  for (iter = mPlaySteps.begin();iter != mPlaySteps.end();iter++)
    {
    PlayStep* elem= *iter;
    retval << elem->getString();
    }    

  retval << "</seq>" << endl << flush;
  return retval.str();
  }

string Seq::getSeqElementsString() const
  {
  stringstream retval;

  vector<PlayStep *>::const_iterator iter;
  for (iter = mPlaySteps.begin();iter != mPlaySteps.end();iter++)
    {
    PlayStep* elem= *iter;
    retval << elem->getString();
    }    

  retval << endl << flush;
  return retval.str();
  }



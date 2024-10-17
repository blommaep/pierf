// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "CounterStep.hpp"

#include <iostream> // for cout and cin
#include <fstream>
#include <sstream>
#include "zthread/Thread.h"
#include <string.h>
#include "VarContainer.hpp"

CounterStep::CounterStep()
  : mReceiveStep(NULL), mCounter(NULL), mValue(0), mBytesValue(0), mActionVar(NULL)
  {
  }

CounterStep::~CounterStep()
  {
  }

void CounterStep::setReceiveStep(ReceiveStep* receiveStep)
  {
  mReceiveStep = receiveStep;
  }

void CounterStep::setCounter(Counter* counter)
  {
  mCounter = counter;
  }

void CounterStep::setAction(CounterAction action)
  {
  mAction = action;
  }

void CounterStep::setAction(char* action)
  {
  if (!strcmp(action,"increment"))
    {
    mAction = eIncrement;
    if (mValue == 0)
      {
      mValue++; // Set to 1: default increment
      }
    }
  else if (!strcmp(action,"reset"))
    {
    mAction = eReset;
    }
  else if (!strcmp(action,"hold"))
    {
    mAction = eHold;
    }
  else if (!strcmp(action,"cont"))
    {
    mAction = eCont;
    }
  else if (!strcmp(action,"report"))
    {
    mAction = eReport;
    }
  }

void CounterStep::setValue(const char* value) noexcept(false)
  {
  mValue = textToLong(value);
  }

void CounterStep::setBytesValue(const char* bytesValue) noexcept(false)
  {
  mBytesValue = textToLong(bytesValue);
  }

void CounterStep::setVar(const char* varName) noexcept(false)
  {
  Var* var = VarContainer::getVar(varName);
  if (var == NULL)
    {
    throw Exception("Unexisting variable with name: " + string(varName));
    }
  if (var->isConst())
    {
    throw Exception("Cannot assign to a constant: " + string(varName));
    }
  mActionVar = var;
  }

void CounterStep::play()
  {
  // just overwrite mValue every time in case of variable
  if (mActionVar != NULL)
    {
    mValue = textToLong(mActionVar->getStringValue().c_str());
    }

  switch(mAction)
    {
    case eIncrement:
      {
      ulong32 size = mBytesValue; // No packet size possible when not in a receive
      if (mReceiveStep != NULL)
        {
        size = mReceiveStep->getPacketSize();
        }
      mCounter->increment(mValue,size);
      }
      break;
    case eReset:
      if (mValue != 0)
        {
        mCounter->reset(mValue);
        }
      else
        {
        mCounter->reset();
        }
      break;
    case eHold:
      if (mReceiveStep != NULL)
        {
        struct timeval pktTime = mReceiveStep->getPktTime();
        mCounter->hold(pktTime);
        }
      else
        {
        mCounter->hold();
        }
      break;
    case eCont:
      if (mReceiveStep != NULL)
        {
        struct timeval pktTime = mReceiveStep->getPktTime();
        mCounter->cont(pktTime);
        }
      else
        {
        mCounter->cont();
        }
      break;
    case eReport:
      if (mReceiveStep != NULL)
        {
        struct timeval pktTime = mReceiveStep->getPktTime();
        mCounter->timeClick(pktTime);
        }
      else
        {
        mCounter->timeClick();
        }
      cout << "Counter: " << mCounter->getName() 
           << " - Count: " << mCounter->getCount() 
           << " - Rate: " << mCounter->getRate()
           << " - Bytes: " << mCounter->getByteCount()
           << " - Bitrate: " << mCounter->getBitrateString()
           << endl;
      break;
    }
  }

string CounterStep::getString() const
  {
  stringstream retval;
  retval << "<counter ";

  if (mCounter != NULL)
    {
    retval << "ref=\"" << mCounter->getName() << "\" ";
    }

  stringstream valueString;
  if (mActionVar != NULL)
    {
    valueString << "value=\"" << mActionVar->getName() << "\" ";
    }

  retval << "action=\"";
  switch (mAction)
    {
    case eIncrement:
      retval << "increment\" ";
      if (mActionVar == NULL && mValue != 1)
        {
        valueString << "value=\"" << mValue << "\" ";
        }
      valueString << flush;
      retval << valueString.str();
      break;
    case eReset:
      retval << "reset\" ";
      if (mActionVar == NULL && mValue != 0)
        {
        valueString << "value=\"" << mValue << "\" ";
        }
      valueString << flush;
      retval << valueString.str();
      break;
    case eHold:
      retval << "hold\" ";
      break;
    case eCont:
      retval << "cont\" ";
      break;
    case eReport:
      retval << "report\" ";
      break;
    }

  retval << " />" << endl << flush;
  return retval.str();
  }


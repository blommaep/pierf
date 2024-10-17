// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "AutoObject.hpp"

AutoObject::AutoObject()
  :mAuto(eAutoEnherit) // default
  {
  }

AutoObject::~AutoObject()
  {
  }

void AutoObject::setAuto(char* instr) throw (Exception)
  {
  if (!strcmp(instr,"auto"))
    {
    mAuto = eAutoConfig;
    }
  else if (!strcmp(instr,"manual"))
    {
    mAuto = eManualConfig;
    }
  else
    {
    throw Exception("Unknown auto type: " + string(instr));
    }
  }

void AutoObject::enheritAuto(AutoObject* mother)
  {
  switch (mother->getAuto())
    {
    case eManualConfig:
    case eManualEnherit:
      mAuto = eManualEnherit;
      break;
    case eAutoConfig:
    case eAutoEnherit:
      mAuto = eAutoEnherit;
      break;
    }
  }

void AutoObject::setOrEnheritAuto(char* instr, AutoObject* mother) throw (Exception)
  {
  if (instr != NULL)
    {
    setAuto(instr);
    }
  else
    {
    enheritAuto(mother);
    }
  }

void AutoObject::setAuto(AutoComplete newval)
  {
  mAuto = newval;
  }

AutoObject::AutoComplete AutoObject::getAuto()
  {
  return mAuto;
  }

bool AutoObject::isAuto()
  {
  if ((mAuto == eAutoEnherit) || (mAuto == eAutoConfig))
    {
    return true;
    }
  return  false;
  }

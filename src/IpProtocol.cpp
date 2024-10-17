// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "IpProtocol.hpp"
#include <sstream>
#include <string.h>
#include <typeinfo>

//// OVERLOADED CLASSES FOR DEDICATED FIELD INTERPRETATIONS ////

void IpProtocol::setManualFromValue(const char* inString) throw (Exception)
  {
  if (!strcmp(inString,"icmp"))
    {
    Bitfield8::setManualFromValue(0x01);
    }
  else if (!strcmp(inString,"igmp"))
    {
    Bitfield8::setManualFromValue(0x02);
    }
  else if (!strcmp(inString,"udp"))
    {
    Bitfield8::setManualFromValue(0x11);
    }
  else if (!strcmp(inString,"tcp"))
    {
    Bitfield8::setManualFromValue(0x06);
    }
  else if (!strncmp(inString,"0x",2))
    {
    Bitfield8::setManualFromValue(inString);
    }
  else
    {
    throw Exception("Protocol not (yet) supported: " + string(inString));
    }
  }

void IpProtocol::setAuto(const char* inString) throw (Exception)
  {
  if (!strcmp(inString,"icmp"))
    {
    Bitfield8::setAuto(0x01);
    }
  else if (!strcmp(inString,"igmp"))
    {
    Bitfield8::setAuto(0x02);
    }
  else if (!strcmp(inString,"udp"))
    {
    Bitfield8::setAuto(0x11);
    }
  else if (!strcmp(inString,"udp"))
    {
    Bitfield8::setAuto(0x06);
    }
  else if (!strncmp(inString,"0x",2))
    {
    Bitfield8::setAuto(inString);
    }
  else
    {
    throw Exception("Protocol not (yet) supported: " + string(inString));
    }
  }

void IpProtocol::setDefault(const char* inString) throw (Exception)
  {
  if (!strcmp(inString,"icmp"))
    {
    Bitfield8::setDefault(0x01);
    }
  else if (!strcmp(inString,"igmp"))
    {
    Bitfield8::setDefault(0x02);
    }
  else if (!strcmp(inString,"udp"))
    {
    Bitfield8::setDefault(0x11);
    }
  else if (!strcmp(inString,"tcp"))
    {
    Bitfield8::setDefault(0x06);
    }
  else if (!strncmp(inString,"0x",2))
    {
    Bitfield8::setDefault(inString);
    }
  else
    {
    throw Exception("Protocol not (yet) supported: " + string(inString));
    }
  }

string IpProtocol::getString()
  {
  if (mData == 0x01)
    {
    return "icmp";
    }
  else if (mData == 0x02)
    {
    return "igmp";
    }
  else if (mData == 0x11)
    {
    return "udp";
    }
  else if (mData == 0x06)
    {
    return "tcp";
    }

  return Bitfield8::getString();
  }

bool IpProtocol::getString(string& stringval)
  {
  if (hasValue())
    {
    stringval = getString();
    return true;
    }
  return false;
  }



// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "RxPacket.hpp"

void RxPacket::setPort(Port* port)
  {
  mPort = port;
  }

Port* RxPacket::getPort()
  {
  return mPort;
  }

void RxPacket::setPacket(uchar* packet, struct pcap_pkthdr* header)
  {
  mPacket = packet;
  mHeader = header;
  }

uchar* RxPacket::getPacket()
  {
  return mPacket;
  }

struct pcap_pkthdr* RxPacket::getHeader()
  {
  return mHeader;
  }

ulong RxPacket::size()
  {
  return mHeader->caplen;
  }
  
ulong RxPacket::rxSecs()
  {
  return mHeader->ts.tv_sec;
  }

ulong RxPacket::rxUsecs()
  {
  return mHeader->ts.tv_usec;
  }

struct timeval RxPacket::getPktTime()
  {
  return mHeader->ts;
  }


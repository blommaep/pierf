// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "PcapFile.hpp"

#include <iostream> // for cout and cin
#include <iomanip>
#include <fstream>
#include <sys/time.h>

void PcapFile::openForWriting(const char* name) noexcept(false)
  {
  mBinFH.open(name,ios::out | ios::binary);
  if (!mBinFH.is_open())
    {
    throw Exception(string("Failed to open file for output") + name);
    }
  struct pcap_file_header header;
  header.magic = 0xa1b2c3d4;
  header.version_major = 2;
  header.version_minor = 4;
  header.thiszone = 0;
  header.sigfigs = 0;
  header.snaplen = 65535;
  header.linktype = 1;
  mBinFH.write((const char*)&header,sizeof(header));
  }

void PcapFile::openForWriting(string& name) noexcept(false)
  {
  openForWriting(name.c_str());
  }

void PcapFile::addPacket(struct pcap_pkthdr *header, u_char *pkt_data) noexcept(false)
  {
  if (!mBinFH.is_open())
    {
    throw Exception("Attempt to write to a capture file that isn't open");
    }
  
  mBinFH.write((const char*)header,sizeof(struct pcap_pkthdr));
  mBinFH.write((const char*)pkt_data,header->caplen);
  }

void PcapFile::addPacket(u_char *pkt_data, ulong32 size) noexcept(false)
  {
  if (!mBinFH.is_open())
    {
    throw Exception("Attempt to write to a capture file that isn't open");
    }

  struct pcap_pkthdr header;
  struct timezone tz;
  gettimeofday(&header.ts, &tz);
  header.caplen = size;
  header.len = size;
  mBinFH.write((const char*)&header,sizeof(header));
  mBinFH.write((const char*)pkt_data,size);
  }

void PcapFile::close()
  {
  if (!mBinFH.is_open())
    {
    mBinFH.close();
    }
  }


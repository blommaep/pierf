// Copyright (c) 2013, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include <iostream> // for cout and cin
#include <iomanip>
#include <fstream>
#include <string>
#include <string.h>
#include "../Counter.hpp"
#include "../PcapFile.hpp"
#include "../generics.hpp"

extern "C"
  {
#include "expat.h"
#include <pcap.h>
  }

#ifndef VERSION
#define VERSION "Makefile bug. No version specified"
#endif

using namespace std;



int main(int argc, char **argv)
  {
  bool detail = false;
  ulong32 rate = 1000000;
  ulong32 streamPos = 11; // last byte of source mac@ by default
  ulong32 counterPos = 44; 
  if (argc < 2)
    {
    cout << "usage: " << argv[0] << " <interface> [debug|go] [rate] [stream id position] [counter position] " << endl;
    cout << "  debug|go : use debug to dump first dropped packets details" << endl;
    cout << "  rate : total expected packet rate per second (reporting interval)" << endl;
    cout << "  stream id postition: byte offset in the packet to use as differentiator between streams" << endl;
    cout << "  counter position: byte offset in the packet for a 4 bytes counter that increments with every packet at transmit" << endl;
    cout << "Version: " << VERSION << endl;
    cout << "Copyright (c) 2006-2011, Pieter Blommaert: see license.txt" << endl;
    return 1;
    }
  else
    {
    if (argc >=3)
      {
      if (!strcmp(argv[2],"debug"))
        {
        detail = true;
        }
      }
    if (argc >= 4)
      {
      rate = textToLong(argv[3]);
      if (rate < 10)
        {
        cout << "invalid rate. Must be minimal 10. Set to default." << endl;
        rate = 1000000;
        }
      }
    if (argc >= 5)
      {
      streamPos = textToLong(argv[4]);
      if (streamPos > 1500)
        {
        cout << "invalid stream position. Cannot be larger then 1500. Set to default." << endl;
        streamPos = 11;
        }
      }
    if (argc >= 6)
      {
      counterPos = textToLong(argv[5]);
      if (counterPos < streamPos)
        {
        cout << "Invalid counter position. Currently not supported to be smalled than the stream id position" << endl;
        return 1;
        }
      }
    }


  // INIT //////////////////////////////
    pcap_t *fp;
    int status;
    char errbuf[PCAP_ERRBUF_SIZE];

    fp = pcap_create(argv[1], errbuf);
    if (fp == NULL)
      {
      cout << "The specified device: eth0 cannot be opened by pcap. " << endl;
      cout << endl;
      cout << "\nUnable to open the device (using pcap)\n"; 
      return 1;
      }
    status = pcap_set_snaplen(fp, (counterPos + 8)); // capture only the bytes needed
    if (status < 0)
      {
      cout << "\nUnable to configure the device snaplen (using pcap)\n"; 
      }
    status = pcap_set_promisc(fp, 1);
    if (status < 0)
      {
      cout << "\nUnable to configure the device in promiscue mode (using pcap)\n";
      }
    status = pcap_set_timeout(fp, 1000);
    if (status < 0)
      {
      throw Exception ("\nUnable to configure the device timeout (using pcap)\n"); // tbd: error handling
      }
    if((pcap_set_buffer_size(fp, 32*1024*1024))!=0) // try 2MB
      {
      cout << "Failed to allocate 2 MB\n" << endl;
      if((pcap_set_buffer_size(fp, 1024*1024))!=0)
        {
        cout << "Failed to allocate 1 MB\n" << endl;
        // do nothing: keep default size
        }
      }
    status=(pcap_activate(fp));
    if(status!=0)
      {
      cout << "\nUnable to configure the device timeout (using pcap)\n" << endl; // tbd: detailed error handling
      return 1;
      }


  // RECEIVE ////////////////////////
  struct pcap_pkthdr *header;
  uchar *pkt_data;
  int res=0;
  unsigned int ii;
  ushort jj; 


//  PcapFile pcapFile;
//  pcapFile.openForWriting("log.cap");

  // COUNT //////////////////////////
  Counter timeCounter("Main");
  ulong32 nrPackets[256];
  ulong32 totalSize[256];
  ulong32 signature[256];
  ulong32 dropped[256];
  ulong32 sequenceFail[256];
  ulong32 overallDropped[256];
  ulong32 overallSequenceFail[256];
  unsigned long long overallNrPackets[256];
  unsigned long long overallTotalSize[256];

  ulong32 buffer[1024];
  int bufferpos = 0;
  for (ii=0;ii<1024;ii++)
    {
    buffer[ii] = 0;
    }
  for (ii=0;ii<256;ii++)
    {
    overallDropped[ii] = 0;
    overallSequenceFail[ii] = 0;
    }

  uchar field;

  // uncounted packets, just to demark a proper counting of the time

  cout << "Waiting for first packets" << endl;
  for (ii=0;ii<10;ii++)
    {
    while (res != 1) // active loop
      {
      res = pcap_next_ex(fp, &header, (const uchar**) &pkt_data);
      }
    }

  cout << "First packets received. Starting observation" << endl;

  int nrReports;
  for (nrReports=0;nrReports<10;nrReports++)
    {
    // CLEAR COUNTERS /////////////////
    for (jj=0;jj<=255;jj++)
      {
      field = (uchar) jj;
      nrPackets[field]=0;
      totalSize[field]=0;
      dropped[field]=0;
      sequenceFail[field]=0;
      signature[field] = 0;
      }    

    timeCounter.reset();
    timeCounter.cont(header->ts);
    
    
    for (ii=0;ii<rate;ii++)
      {
      res=0;
      while (res != 1)
        {
        res = pcap_next_ex(fp, &header, (const uchar**) &pkt_data);
        }

//      pcapFile.addPacket(header,pkt_data);

      field = pkt_data[streamPos];
  //    cout.setf(ios::hex,ios::basefield); 
  //    cout.fill('0');
  //    for (int kk=0; kk<40;kk++)
  //      {
  //      cout << setw(2) << (int) pkt_data[kk] << " ";
  //      }
  //    cout << endl;
  //    cout.setf(ios::dec,ios::basefield); 

      nrPackets[field]++;
      totalSize[field] += header->len;    

      ulong32 *pktSignature = (ulong32*) (pkt_data + counterPos); // mytest: 44
      ulong32 cap = ntohl(*pktSignature);
      ulong32 cur = signature[field];
      cur++;
      if (cur == cap)
        {
        signature[field] = cur;
        }
      else
        {
        if (cur == 1) // should occur only once, given the size of ulong : initial situation (was 0, but incremented to 1 at the start)
          {
          signature[field] = cap;
          }
        else if (cur < cap)
          {
          //cout << "dropped: " << cur << " - " << cap << endl;
          ulong32 diff = cap-cur;
          dropped[field]+=diff;
          signature[field] = cap;
          }
        else // later frame has already been recieved
          {
          sequenceFail[field]++;
          if (dropped[field] > 0)
            {
            dropped[field]--;
            }
          }
        if (detail && bufferpos <= 1020)
          {
          buffer[bufferpos++] = (ulong) field;
          buffer[bufferpos++] = (ulong) ii;
          buffer[bufferpos++] = cur;
          buffer[bufferpos++] = cap;
          }
        }
      }

    // REPORT //////////////////////////
    timeCounter.hold(header->ts);
    timeCounter.timeClick();

    ulong32 nrPacketsAll=0;
    unsigned long long totalSizeAll=0;
    unsigned long long totalDroppedAll=0;
    unsigned long long totalSequenceFailAll=0;
    ulong32 nrPacketsCur;
    ulong32 totalSizeCur;
    bool bufferNeeded = false;

    for (jj=0;jj<=255;jj++)
      {
      field = (uchar) jj;
      nrPacketsCur = nrPackets[field];
      if (nrPacketsCur > 0)
        {
        nrPacketsAll += nrPacketsCur;
        totalSizeCur = totalSize[field];
        totalSizeAll += totalSizeCur;
        timeCounter.overrule(nrPacketsCur,totalSizeCur);
        totalDroppedAll += dropped[field];
        totalSequenceFailAll += sequenceFail[field];

        ulong32 rate = timeCounter.getRate();
        cout << "Value: " << (ushort) field << " - Count: " << nrPacketsCur << " - Bytes: " << totalSize[field] << " - Rate: " << rate << " - Bitrate: " << timeCounter.getBitrateString() ;
        cout << " - Dropped: " << dropped[field] << " - Seq fail: " << sequenceFail[field] << endl;
        overallNrPackets[field] += nrPacketsCur;
        overallTotalSize[field] += totalSizeCur;
        overallDropped[field] += dropped[field];
        overallSequenceFail[field] += sequenceFail[field];

        if (detail)
          {
          if (dropped[field] > 0 || sequenceFail[field] > 0)
            {
            bufferNeeded = true;
            }
          }
        }
        
      }

    timeCounter.overruleXl(nrPacketsAll,totalSizeAll);

    if (bufferNeeded)
      {
      cout << "First dropped/misordered packet log:" << endl;
      // print investigation buffer of dropped/oos packets
      for (ii=0;ii<1024 && buffer[ii] != 0;ii++)
        {
        cout << " Stream: " << buffer[ii];
        buffer[ii++] = 0;
        cout << " - Seq: " << buffer[ii];
        buffer[ii++] = 0;
        cout << " -  Expect: " << buffer[ii];
        buffer[ii++] = 0;
        cout << " - Rx: " << buffer[ii] << endl;
        buffer[ii] = 0;
        } 
      bufferpos = 0;
      cout << endl;
      }

    cout << "Total:   - Count: " << nrPacketsAll << " - Bytes: " << totalSizeAll << " - Rate: " << timeCounter.getRate() << " - Bitrate: " <<   timeCounter.getBitrateString() << " - Dropped: " << totalDroppedAll << " - Seq fail: " << totalSequenceFailAll << " - Time: " << timeCounter.getTotalTime() << endl;
    cout << "-------" << endl;
    }

//  pcapFile.close();

  for (jj=0;jj<=255;jj++)
    {
    field = (uchar) jj;
    if (overallDropped[field] > 0 || overallSequenceFail[field] > 0 )
      {
      cout << "OVERALL: Value: " << (ushort) field  ;
      cout << " - Count: " << overallNrPackets[field] << " - Total bytes: " << overallTotalSize[field] << " - Dropped: " << overallDropped[field] << " - Sequence fail: " << overallSequenceFail[field] << endl;
      }
      
    }
  

  return 0;
  }

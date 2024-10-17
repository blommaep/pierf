// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "Port.hpp"

#include <iostream> // for cout and cin
#include <iomanip>
#include <fstream>
#include <sstream>
#include "zthread/Guard.h"
#include <sys/time.h>
#include <stdio.h>
#include <string.h>

//because on different platforms, the include files for sockaddr_in seem to be located on different places
//and somehow, the libpcap library didn't seem to locate them correctly on my linux
//i decided that i didn't want to make platform dependend code or other kind of trouble for just information purpose
//and simply added local definitions of struct sockaddr_in here
//per my knowledge, this definition is valid cross platform

struct local_in_addr {
    unsigned long addr; // load with inet_aton()
};

struct local_sockaddr_in {
    unsigned short         sa_family;   // e.g. AF_INET
    unsigned short         sin_port;     // e.g. htons(3490)
    struct local_in_addr   sin_addr;     // see struct in_addr, below
    char                   sin_zero[8];  // zero this if you want to
};



using namespace ZThread;

//Declaration
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data);

Port::Port(char* easyName, char* sourceName) throw (Exception)
  : mTraceThreadCondition(mTraceThreadMutex)
  {
  initPort(eDevice, easyName, sourceName, NULL);
  }

Port::Port(PortType portType, char* easyName, char* sourceName, char* destName) throw (Exception)
  : mTraceThreadCondition(mTraceThreadMutex)
  {
  initPort(portType, easyName, sourceName, destName);
  }

void Port::initPort(PortType portType, char* easyName, char* sourceName, char* destName) throw (Exception)
  {
  // generic member initialisation
  mEasyName=easyName;
  mSourceName=sourceName;
  if (destName != NULL)
    {
    mDestName=destName;
    }
  mPrevReadTime = 0;
  mSleepForReplay = false;
  mPortType = portType;
  mLogState = eIdle;
  mEndThread = false;
  mReceiveState = eIdleRx;
  mTraceFH = NULL;
  mCurrentPacket.setPort(this);
  mDestFp = NULL;
  mSourceFp = NULL;
  mSilent = false;
  mLoopbackPkt = NULL;
  mLoopbackHdr = NULL;
  mLoopbackState = eLoopbackIdle; //initial state is received, which means received andhandled previous packet
  
  char errbuf[PCAP_ERRBUF_SIZE];

  /* Open the adapter */
  if (portType == eDevice)
    {
    pcap_t *fp;
    int status;

    fp = pcap_create(sourceName, errbuf);
    if (fp == NULL)
      {
      cout << "The specified device: " << mSourceName << " cannot be opened by pcap. The following devices exist: " << endl;
      printAllDevices();
      cout << endl;
      throw Exception ("\nUnable to open the device (using pcap)\n"); // tbd: error handling
      return;
      }
    status = pcap_set_snaplen(fp, 65536);
    if (status < 0)
      {
      throw Exception ("\nUnable to configure the device snaplen (using pcap)\n"); // tbd: error handling
      }
    status = pcap_set_promisc(fp, 1);
    if (status < 0)
      {
      throw Exception ("\nUnable to configure the device in promiscue mode (using pcap)\n"); // tbd: error handling
      }
    status = pcap_set_timeout(fp, 1000);
    if (status < 0)
      {
      throw Exception ("\nUnable to configure the device timeout (using pcap)\n"); // tbd: error handling
      }
    if((pcap_set_buffer_size(fp, 2*1024*1024))!=0) // try 2MB
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
      throw Exception ("\nUnable to configure the device timeout (using pcap)\n"); // tbd: detailed error handling
      }

//    if ((mSourceFp = pcap_open_live(sourceName,		// name of the device
//          65536,			// portion of the packet to capture. It doesn't matter in this case 
//          1,				// promiscuous mode (nonzero means promiscuous)
//          1000,			// read timeout
//          errbuf			// error buffer
//          )) == NULL)
    mSourceFp=fp;
    mDestFp = mSourceFp;
    }
  else if (portType == eInFileOutDevice)
    {
    if ((mSourceFp = pcap_open_offline(sourceName, errbuf	)) == NULL)
      {
      cout << "The specified file: " << sourceName << " cannot be opened by pcap: " << errbuf << endl;
      cout << endl;
      throw Exception ("\nUnable to open the file (using pcap)\n"); // tbd: error handling
      return;
      }
    if ((mDestFp = pcap_open_live(destName,		// name of the device
          65536,			// portion of the packet to capture. It doesn't matter in this case 
          1,				// promiscuous mode (nonzero means promiscuous)
          1000,			// read timeout
          errbuf			// error buffer
          )) == NULL)
      {
      cout << "The specified device: " << destName << " cannot be opened by pcap. The following devices exist: " << endl;
      printAllDevices();
      cout << endl;
      throw Exception ("\nUnable to open the device (using pcap)\n"); // tbd: error handling
      return;
      }
    }
  else if (portType == eFiles)
    {
    if ((mSourceFp = pcap_open_offline(sourceName, errbuf	)) == NULL)
      {
      cout << "The specified file: " << sourceName << " cannot be opened by pcap: " << errbuf << endl;
      cout << endl;
      throw Exception ("\nUnable to open the file (using pcap)\n"); // tbd: error handling
      return;
      }
  
    mDestFile.openForWriting(destName);
    }
  else if (portType == eLoopback)
    {
    // only need a destination file
    mDestFile.openForWriting(destName);
    }
  else
    {
    throw Exception ("\nBUG in creating Port\n");
    }
  }

Port::~Port()
  {
  if (mTraceFH != NULL)
    {
    pcap_dump_close(mTraceFH);
    mTraceFH=NULL;
    }
  if ((mSourceFp == mDestFp) && (mSourceFp != NULL))
    {
    pcap_close(mSourceFp);	
    mSourceFp = NULL;
    mDestFp=NULL;
    }
  else
    {
    if (mSourceFp != NULL)
      {
      pcap_close(mSourceFp);
      mSourceFp = NULL;
      }
    if (mDestFp != NULL)
      {
      pcap_close(mDestFp);
      }
    mDestFp = NULL;
    }  
  
  mDestFile.close();

  if (mLoopbackHdr != NULL)
    {
    delete mLoopbackHdr;
    }
  if (mLoopbackPkt != NULL)
    {
    delete mLoopbackPkt;
    }
  
  }

void Port::run() // This function is supposed to be used as a separate THREAD!
  { 
  struct pcap_pkthdr *header;
  uchar *pkt_data;
  int res;

  while (!mEndThread)
    {
    // Problem to be solved: when log is enabled, this loop may proceed before the receiver has registered himself again (in case of loops), as such, that receiver may miss some messages, which is not the intention.
    bool waitReceive = true;
    bool mPortOpen = true;
    while (waitReceive)
      {
      Guard<Mutex> g(mTraceThreadMutex);
      if (mEndThread)
        {
        waitReceive = false; //Quit the loop, so that we can exit
        }
      else if (mReceiveState == eRequest)
        {
        // cout << "Request for a packet, start tracing" << endl;
        waitReceive=false; // Go ahead
        }
      else if (mReceiveState == eIdleRx && mLogState == eOpen)
        {
        waitReceive=false; // Go ahead, possibly for logging
        }
      else // Wait here for all the receivers to finish their job 
        {
        // cout << "Waiting till some request comes in" << endl;
        mTraceThreadCondition.wait();
        }
      }

    //Loop until a message is succesfull received    
    res = -3; // Just an initial value
    while((!mEndThread) && (mPortOpen) && (res <= 0))
      {
      if (mPortType == eLoopback)
        {
        if (mLoopbackState == eHasPacket)
          {
          pkt_data = mLoopbackPkt;
          header = mLoopbackHdr;
          mLoopbackState = eHasPacket;
          res = 1;
          }
        else
          {
          res = 0; // act as timeout.
          }
        }
      else
        {
        res = pcap_next_ex(mSourceFp, &header, (const uchar**) &pkt_data);
        }
      if (res == 0)
        {
        Thread::sleep(100); //sleep and try again
        // cout << "Timeout reading on port" << endl;
        }
      else if (res == -1)
        {
        Thread::sleep(100); //sleep and try again
        cout << "Failure reading on port" << endl;
        }
      else if (res == -2) // EOF
        {
        mPortOpen = false; 
        }
      }


    if (res > 0)
      {
      if (mLogState == eOpen)
        {
        pcap_dump((uchar *)mTraceFH, header, pkt_data); //Currently simply saving everything always
        }
      if (mReceiveState == eRequest)
        {
        mCurrentPacket.setPacket(pkt_data,header);

        if (mPortType==eInFileOutDevice || mPortType==eFiles)
          {
          ulong32 time = ((header->ts.tv_sec%10000) * 1000) + (header->ts.tv_usec/1000); // tv_secs is too big to fit in a log, so take remainder by division by 10000 and assume that we will never test longer. Then multiply by 1000 to transfer teh seconds into milliseconds. tv_usec is in microseconds. We can do only milliseconds, so that's three digits less. Hence, divide by 1000.
          ulong32 diff = time - mPrevReadTime;
          if (mPrevReadTime > 0 && mSleepForReplay) // First packet must be sent immediately
            {
            if (!mSilent)
              {
              cout << "Waiting " << diff << " milliseconds to handle next packet from realtime file " << endl;
              }
            Thread::sleep (diff);
            }
          mPrevReadTime = time;
          }

        // signal that we're ready
          {
          Guard<Mutex> g(mTraceThreadMutex);
          mReceiveState = eReceived;
          //cout << "Packet received" << endl;
          mTraceThreadCondition.broadcast(); // signal to all receivers
          }

        }
      }
    }
  
//  cout << "Reaching end of thread" << endl;
  }

void Port::close()
  {
  // We allow the thread to end here. When the thread ends, it deletes the port object and hence calls the destructor that does the rest. If we would to it here, we rist concurrency problems.
    {
    Guard<Mutex> g(mTraceThreadMutex);
    mEndThread = true;
    //cout << "Signaling end of thread" << endl;
    mTraceThreadCondition.broadcast(); //have it continue the loop and exit 
    }
  }

void Port::startLog(char* fileName) throw (Exception)
  {
  mTraceFilename = fileName;
  mLogState = eOpen;
  Guard<Mutex> g(mTraceThreadMutex);
  mTraceThreadCondition.broadcast();
  }

void Port::send(uchar* packet, int packetsize)
  {
  if (!mSilent)
    {
    cout << "Sent on " << mEasyName << ": ";
    cout.setf(ios::hex,ios::basefield); 
    cout.fill('0');
    for (int i=0; i<packetsize;i++)
      {
      cout << setw(2) << (int) packet[i] << " ";
      }
    cout << endl;
    cout.setf(ios::dec,ios::basefield); 
    }

  if (mPortType == eDevice || mPortType == eInFileOutDevice)
    {
    int tryloop = 1;
    while (tryloop)
      {
      if (pcap_sendpacket(mDestFp,	// Adapter
          packet,				// buffer with the packet
          packetsize					// size
          ) != 0)
        {
        char* errorString = pcap_geterr(mDestFp);
        if (!strncmp(errorString, "send: No buffer", 14))
          {
          tryloop ++;
          }
        else
          {
          throw Exception ("Error sending the packet: " + string(errorString));
          }
        }
      else
        {
        if (tryloop > 1)
          {
          if (not(mSilent))
            {
            cout << "Warning: Send Packet: Buffer full hit: " << tryloop << "times\n";
            }
          }
        tryloop = 0;
        }
      }
    }
  else if (mPortType == eFiles)
    {
    mDestFile.addPacket(packet,packetsize);
    }
  else if (mPortType == eLoopback)
    {
    mDestFile.addPacket(packet,packetsize);
    if (mLoopbackState == eLoopbackIdle)
      {
      if (mLoopbackHdr != NULL)
        {
        delete mLoopbackHdr;
        }
      if (mLoopbackPkt != NULL)
        {
        delete mLoopbackPkt;
        }
      struct timezone tz;
      mLoopbackHdr = new struct pcap_pkthdr;
      gettimeofday(&mLoopbackHdr->ts, &tz);
      mLoopbackHdr->caplen = packetsize;
      mLoopbackHdr->len = packetsize;
      mLoopbackPkt = new uchar[packetsize];
      memcpy(mLoopbackPkt,packet,packetsize);
      mLoopbackState = eHasPacket;
      }
    // else: previous loopback packet not yet fully received. Since we don't have a queue, ignore this one.
    }
  else
    {
    throw Exception ("BUG: invalid port type/port type not covered in send)");
    }

  }

string Port::getId()
  {
  return mEasyName;
  }


void Port::printAllDevices()
  {
  pcap_if_t *alldevs;
  char errbuf[PCAP_ERRBUF_SIZE+1];

  /* Retrieve the device list */
  if(pcap_findalldevs(&alldevs, errbuf) == -1)
    {
    fprintf(stderr,"Error in pcap_findalldevs: %s\n", errbuf);
    exit(1);
    }

  /* Scan the list printing every entry */
  for(pcap_if_t* dev=alldevs;dev != NULL;dev=dev->next)
    {
    /* Name */
    cout << dev->name << endl;

    /* Description */
    if (dev->description)
      {
      cout << "\tDescription: " << dev->description << endl;
      }

    /* Loopback Address*/
    cout << "\tLoopback: " << ((dev->flags & PCAP_IF_LOOPBACK)?"yes":"no") << endl;

    /* IP addresses */
    for(pcap_addr_t *addrEl=dev->addresses;addrEl != NULL;addrEl=addrEl->next) 
      {
      if (addrEl->addr) //NULL in case of tunnel interface
        {
        unsigned short sa_family = ((struct local_sockaddr_in *)addrEl->addr)->sa_family;

        switch(sa_family)
          {
          case 2: //AF_INET
            cout << "\tAddress Family: AF_INET (IP) - " << sa_family << endl;
            if (addrEl->addr)
              cout << "\tAddress: " << iptos(((struct local_sockaddr_in *)addrEl->addr)->sin_addr.addr) << endl;
            if (addrEl->netmask)
              cout << "\tNetmask: " << iptos(((struct local_sockaddr_in *)addrEl->netmask)->sin_addr.addr) << endl;
            if (addrEl->broadaddr)
              cout << "\tBroadcast Address: " << iptos(((struct local_sockaddr_in *)addrEl->broadaddr)->sin_addr.addr) << endl;
            if (addrEl->dstaddr)
              cout << "\tDestination Address: " << iptos(((struct local_sockaddr_in *)addrEl->dstaddr)->sin_addr.addr) << endl;
            break;

          case 10: //AF_INET6 
            cout << "\tAddress Family Name: AF_INET6 (IPv6) - " << sa_family << endl;
            break;

          default:
            cout << "\tAddress Family Name: Unknown (" << sa_family << ")" << endl;
            break;
          }
        cout << endl;
        }
      }
    }
  cout << endl << "On Linux, if this list is empty, it's probably because you are not root. This tool directly accesses devices and therefore you need to be root." << endl;

  /* Free the device list */
  pcap_freealldevs(alldevs);

  }


string Port::iptos(u_long in)
  {
  stringstream output;

  u_char *pByte;
  pByte = (u_char *)&in;
  output << (unsigned int) pByte[0] 
    << "." << (unsigned int) pByte[1] 
    << "." << (unsigned int) pByte[2] 
    << "." << (unsigned int) pByte[3] << flush;
  return output.str();
  }

void Port::setSleepForReplay(bool sleep)
  {
  mSleepForReplay = sleep;
  }


void Port::waitReceive()
  {
    {
    Guard<Mutex> g2(mTraceThreadMutex);
    if (mReceiveState != eIdleRx)
      {
      cout << "Code problem: trying to receive a new packet while previous one is yet not handled" << endl;
      }
    mReceiveState = eRequest;
//    cout << "Requesting for a new packet" << endl;
    mTraceThreadCondition.broadcast(); //have it continue the loop
    }

  bool loop=true;
  while (loop)
    {
    Guard<Mutex> g(mTraceThreadMutex);
    if (mReceiveState == eReceived) // Just to be sure that since the Signal, nothing has hapened
      {
      loop = false;
      }
    else
      {
      mTraceThreadCondition.wait();
      }
    }
  }

RxPacket Port::receive()
  {
  Guard<Mutex> g(mTraceThreadMutex);
  if (mReceiveState != eReceived)
    {
    cout << "Code problem: taking a packet of the stack while state is not received" << endl;
    }
  mReceiveState = eIdleRx;
  mTraceThreadCondition.broadcast(); //have it continue the loop (in case of logging/...)

  mLoopbackState = eLoopbackIdle;
  return mCurrentPacket; //The Mutex is freed by the Guard upon exiting this function
  }

void Port::setSilent(const char* silent) throw (Exception)
  {
  if (!strcmp(silent,"true"))
    {
    mSilent = true;
    }
  else if (!strcmp(silent,"false"))
    {
    mSilent = false;
    }
  else
    {
    throw Exception ("Expecting true or false for attribute silent while reading: " + string(silent));
    }
  }

/*
   void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data)
   {
//struct tm *ltime;
//char timestr[16];
//ltime=localtime(&header->ts.tv_sec);

cout << "Packet received at" << header->ts.tv_sec << "." << header->ts.tv_usec << ", len:" << header->len;

}
 */

string Port::getString() const
  {
  stringstream retval;
  retval << "<port id=\"" << mEasyName << "\" ";

  switch (mPortType)
    {
    case eDevice:
      retval << "type=\"device\" device=\"" << mSourceName << "\" " ;
      break;
    case eInFileOutDevice:
      retval << "type=\"inFileOutDevice\" infile=\"" << mSourceName << "\" outdevice=\"" << mDestName << "\" ";
      break;
    case eFiles:
      retval << "type=\"files\" infile=\"" << mSourceName << "\" outfile=\"" << mDestName << "\" ";
      break;
    case eLoopback:
      retval << "type=\"loopback\" outfile=\"" << mDestName << "\" ";
      break;
    }
  
  if (mSilent)
    {
    retval << "silent=\"true\" ";
    } 
    
  retval << "/>" << endl << flush;
  return retval.str();
  }

string Port::getName() const
  {
  return mEasyName;
  }

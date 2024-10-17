// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#ifndef PORT_HPP__
#define PORT_HPP__

#include "generics.hpp"
#include "PcapFile.hpp"
#include "RxPacket.hpp"
#include <string>
#include "zthread/Runnable.h"
#include "zthread/Thread.h"
#include "zthread/Mutex.h"
#include "zthread/Condition.h"
#include <fstream>

extern "C"
  {
#include <pcap.h>
  }

using namespace std;

class Port: public ZThread::Runnable
  {
  public:
    enum PortType { eDevice, 
                    eInFileOutDevice, 
                    eFiles, // One file for input, one file for output
                    eLoopback,
                  };
    enum ReceiveState { eIdleRx,
                        eRequest,
                        eReceived
                      };
  private:
    string mEasyName; // the ID of the port tag
    bool mSilent;
    PortType mPortType;
    pcap_t *mDestFp;
    pcap_t *mSourceFp;
    PcapFile mDestFile;
    string mSourceName;
    string mDestName;
    ulong32 mPrevReadTime; // Previous time (in microsec) read from an input file (source)
    bool mSleepForReplay;
    ZThread::Mutex mTraceThreadMutex; // Mutex needed for the Condition
    ZThread::Condition mTraceThreadCondition; // Used to wait() the trace thread for a signal() from the main thread that it must start doing something (start tracing).
    ReceiveState mReceiveState;

    ZThread::Thread mTraceThread;
    string mTraceFilename;
    enum LogState {eIdle, eOpen};
    LogState mLogState;

    bool mEndThread; // Set to true when the thread is no longer neaded.
    pcap_dumper_t *mTraceFH;

    RxPacket mCurrentPacket; // Packet currently in buffer
    
    string iptos(u_long in);
    void printAllDevices();
    void initPort(PortType portType, char* easyName, char* sourceName, char* destName) noexcept(false);
    struct pcap_pkthdr* mLoopbackHdr;
    uchar* mLoopbackPkt;
    enum LoopbackState {eLoopbackIdle, eHasPacket, ePacketReceived};
    LoopbackState mLoopbackState;

  public:
    Port(char* easyName, char* sourceName) noexcept(false);
    Port(PortType portType, char* easyName, char* sourceName, char* destName) noexcept(false);
    ~Port();
    void run();
    void close(); //due to the threading, cannot just do it in the destructor
    void startLog(char* fileName) noexcept(false);
    string getId();
    void setSleepForReplay(bool sleep);
    void send(uchar* packet, int packetsize);
    void waitReceive(); // Wait loop waiting for the next packet to come in
    RxPacket receive(); // Always call waitReceive first. This gives you the received packet. Also includes waiting system: to avoid memory allocations, no (additional) buffering is done, but the receive thread will simply wait till everybody can handle the packet
    void setSilent(const char* silent) noexcept(false); // true not to print to stdout
    string getString() const;
    string getName() const;
  };

#endif

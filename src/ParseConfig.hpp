// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#ifndef PARSECONFIG_HPP__
#define PARSECONFIG_HPP__

#include <string>
#include <vector>
#include <deque>
#include <map>

#include "Scene.hpp"
#include "Seq.hpp"
#include "Port.hpp"
#include "Raw.hpp"
#include "IgmpV3.hpp"
#include "Exception.hpp"
#include "Element.hpp"
#include "ReceiveStep.hpp"
#include "zthread/ThreadedExecutor.h"
#include "Counter.hpp"
#include "FirstOfStep.hpp"
#include "StringStep.hpp"
#include "Shaper.hpp"
#include "MultiShaperStep.hpp"
#include "VarContainer.hpp"

extern "C"
  {
#include "expat.h"
  }

class ParseConfig
  {
  private:
  ZThread::ThreadedExecutor mThreadedExecutor;

  enum State {eIdle, ePierf, ePort, eInclude, eLog, eScene, eSeq, ePacket, eSleep, eReceive, eMirror, ePrint, eFirstOf, eMatch, eRaw, eEth, eVlans,eArp,eIpHdr, eIpv6, eIcmp, eIgmp, eIgmpV3Query, eIgmpV3Report, eIgmpGroupRecord, eIgmpV3Source, eUdp, eTcp, ePlay, eCounter, eVar, eText, eShaper, eMultiShaper, eVarAssign, eSignature};

  vector<State> mStateStack;

  struct PierFileContext
    {
    ifstream mStream;
    string   mName; // also needed for relative path includes: must be relative vs. the file that includes it
    XML_Parser mParser; // need a separate parser object per include file, a.o. to assure correct file position tracking when using includes. 
    };

  deque<PierFileContext*> mPierfContextStack;
  PierFileContext* mCurContext;  
  VarContainer mVarContainer;
  
  map<string,Scene*> mScenes;
  map<string,Port*> mPorts;
  map<string,Shaper*> mShapers;
  map<string,Seq*> mSeqs;
  bool mCurrentSequenceUpdateAllowed;
  Port* mCurPort;
  Scene* mCurScene;
  Seq* mCurSeq;
  deque<Seq*> mSeqStack;
 deque<MatchStep*> mMatchStack;
  Packet* mCurrentPacket; // no deque for Packet because we avoid packet match and seq to be mixed in match tag
  Element* mCurElem; // current element
  Raw* mCurRaw;
  ReceiveStep* mCurReceiveStep;
  bool mHandlingMatchElems;
  deque<ReceiveStep*> mReceiveStepStack;
  FirstOfStep* mCurFirstOfStep;
  deque<FirstOfStep*> mFirstOfStepStack;
  IgmpV3* mCurIgmpV3;
  IgmpGroupRec* mCurIgmpGroupRec;
  StringStep* mCurText;
  MultiShaperStep* mCurMultiShaper;
  MatchStep* mCurMatch;
  
  State curState();
  string getParserLine();
  void parserException(const string& msg) noexcept(false);
  Packet* handlePacketTag(const char** attr, PlayStep* root);
  
  // need to add:
  // Stack with string of the xml stree
  // Stack with interfaces

  public:
  ParseConfig();
  ~ParseConfig();
  void parse(std::string& configFile) noexcept(false);
  void parse(const char* configFile) noexcept(false);

  // only to be called by expat 
  void start_hndl(const char *el, const char **attr) noexcept(false);
  void end_hndl(const char *el) noexcept(false);
  void char_hndl(const char *txt, int txtlen) noexcept(false);
  string stateToString(State state);
  string getString() const;
  };

#endif

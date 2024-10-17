// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "ParseConfig.hpp"

#include <string>
#include <iostream> 
#include <fstream>
#include <sstream>

#include "zthread/Thread.h"
using namespace ZThread;

#include "Ethernet.hpp"
#include "Vlan.hpp"
#include "Arp.hpp"
#include "IpHdr.hpp"
#include "Igmp.hpp"
#include "IgmpV2.hpp"
#include "Icmp.hpp"
#include "Udp.hpp"
#include "SleepStep.hpp"
#include "MirrorStep.hpp"
#include "PrintStep.hpp"
#include "CounterStep.hpp"
#include "MatchStep.hpp"
#include "VarAssignStep.hpp"

using namespace std;

void pierfc_start_hndl(void *data, const char *el, const char **attr) 
  {
  ((ParseConfig*)data)->start_hndl(el,attr);
  }

void pierfc_end_hndl(void *data, const char *el)
  {
  ((ParseConfig*)data)->end_hndl(el);
  }

void pierfc_char_hndl(void *data, const char *txt, int txtlen)
  {
  ((ParseConfig*)data)->char_hndl(txt,txtlen);
  }

ParseConfig::ParseConfig()
  :mCurReceiveStep(NULL), mHandlingMatchElems(false), mCurMultiShaper(NULL)
  {
  }

ParseConfig::~ParseConfig()
  {
  map<string,Scene*>::iterator sceneIter;
  for (sceneIter=mScenes.begin(); sceneIter != mScenes.end(); sceneIter++)
    {
    Scene* scene = (*sceneIter).second;
    delete scene;
    }

// Since introduction of threaded Ports, the Thread library will cleanup the ports...
  map<string,Port*>::iterator portsIter;
  for (portsIter = mPorts.begin(); portsIter != mPorts.end(); portsIter++)
    {
    Port* port = (*portsIter).second;
    if (port != NULL)
      {
      port->close();
      }
//    delete port; // Will be done at thread end
    }

  map<string,Shaper*>::iterator shaperIter;
  for (shaperIter=mShapers.begin(); shaperIter != mShapers.end(); shaperIter++)
    {
    Shaper* shaper = (*shaperIter).second;
    delete shaper;
    }

  Thread::sleep(1500); // Dumb sleep just to make sure that all port threads get their time to close. Need to look for more smart mechanism when introducing TaskQueue mechanism to replace the current wait/signal shit
  }

void ParseConfig::parserException(const string& msg) throw (Exception)
  {
  throw Exception("Line " + getParserLine() + ": " + msg);
  }


ParseConfig::State ParseConfig::curState()
  {
  vector<State>::reverse_iterator iterState;
  iterState = mStateStack.rbegin();
  if (iterState != mStateStack.rend())
    {
    return *iterState;
    }
  else
    {
    return eIdle;
    }
  }

Packet* ParseConfig::handlePacketTag(const char** attr, PlayStep* root)
  {
  mStateStack.push_back(ePacket);
  Packet* packet = new Packet();
  packet->setAnalysisReady(true); // defined packets are analysed by default
  mCurrentPacket = packet;

  char* portName=NULL;
  char* autoStr=NULL;
  char* shaperStr=NULL;
  int i=0;
  while (attr[i] != NULL)
    {
    if (!strcmp(attr[i],"port"))
      {
      i++;
      portName = (char*) attr[i++];
      }
    else if (!strcmp(attr[i],"shaper"))
      {
      i++;
      shaperStr = (char*) attr[i++];
      }
    else if (!strcmp(attr[i],"auto"))
      {
      i++;
      autoStr = (char*) attr[i++];
      }
    else
      {
      parserException("Unexpected attribute: " + string(attr[i]) + " in <packet> tag.");
      i++;
      }
    }

  packet->setOrEnheritAuto(autoStr,root);
  if ((portName != NULL))
    {
    Port* port = mPorts[portName];
    if (port != NULL)
      {
      packet->setPort(port);
      }
    else
      {
      parserException("Unknown port: " + string(portName) + " in <packet> tag");
      }
    }
  else
    {
    parserException("Missing mandatory attribute to tag <packet> tag: 'port'");
    }
  if ((shaperStr != NULL))
    {
    Shaper* shaper = mShapers[shaperStr];
    if (shaper != NULL)
      {
      packet->setShaper(shaper);
      }
    else
      {
      parserException("Unknown shaper: " + string(shaperStr) + " in <packet> tag");
      }
    }
  return packet;
  }


void ParseConfig::start_hndl(const char* el, const char **attr) throw (Exception) 
  {
  /*
  cout << "Start tag: " << el;
  if (attr != NULL)
    {
    cout << " (";
    int i=0;
    while (attr[i] != NULL)
      {
      if (i>0)
        {
        cout << ", ";
        }
      cout << attr[i++] << "=" << attr[i++];
      }
    cout << ")";
    }
  cout << endl;
  */

  string elem = el;
  State state = curState();

  switch (state)
    {
    case eIdle: // fallthrough
    case eInclude:
      if (elem == "pierf")
        {
        mStateStack.push_back(ePierf);
        }
      else
        { // syntax error
        parserException("In include file: expected <pierf> starting tag when reading another tag: " + elem);
        }
      break;
    case ePierf:
      if (elem == "scene")
        {
        mStateStack.push_back(eScene);

        char* name=NULL; 
        char* autoStr=NULL;
        int i=0;
        while (attr[i] != NULL)
          {
          if (!strcmp(attr[i],"id"))
            {
            i++;
            name = (char*) attr[i++];
            }
          else if (!strcmp(attr[i],"auto"))
            {
            i++;
            autoStr = (char*) attr[i++];
            }
          else
            {
            parserException("Unexpected attribute: " + string(attr[i]) + " in <scene> tag.");
            i++;
            }
          }
        if (name != NULL)
          {
          Scene* scene = new Scene();
          mCurScene = scene;
          mScenes[name]=scene;
          if (autoStr != NULL)
            {
            scene->setAuto(autoStr);
            }
          }
        else
          {
          parserException("Missing mandatory attribute to tag <scene> tag: 'name'");
          }
        }
      else if (elem == "port")
        {
        mStateStack.push_back(ePort);
        char* portName=NULL;
        char* portDev=NULL; 
        char* inFileName=NULL;
        char* outFileName=NULL;
        char* outDevice=NULL;
        char* realtimeStr=NULL;
        char* silentStr=NULL;
        char* typeStr=NULL;
        int i=0;
        while (attr[i] != NULL)
          {
          if (!strcmp(attr[i],"id"))
            {
            i++;
            portName = (char*) attr[i++];
            }
          else if (!strcmp(attr[i],"device"))
            {
            i++;
            portDev = (char*) attr[i++];
            }
          else if (!strcmp(attr[i],"infile"))
            {
            i++;
            inFileName = (char*) attr[i++];
            }
          else if (!strcmp(attr[i],"outfile"))
            {
            i++;
            outFileName = (char*) attr[i++];
            }
          else if (!strcmp(attr[i],"realtime"))
            {
            i++;
            realtimeStr = (char*) attr[i++];
            }
          else if (!strcmp(attr[i],"outdevice"))
            {
            i++;
            outDevice = (char*) attr[i++];
            }
          else if (!strcmp(attr[i],"silent"))
            {
            i++;
            silentStr = (char*) attr[i++];
            }
          else if (!strcmp(attr[i],"type"))
            {
            i++;
            typeStr = (char*) attr[i++];
            }
          else 
            {
            parserException("Unexpected attribute: " + string(attr[i]) + " in <port> tag.");
            }
          }
        if ((portName != NULL) && (portDev != NULL))
          {
          mCurPort = new Port(portName, portDev);
          mPorts[portName] = mCurPort;
          if (silentStr != NULL)
            {
            mCurPort->setSilent(silentStr);
            }
          mThreadedExecutor.execute(mCurPort); // Start a thread on the port for receiving messages
          }
        else if ((portName != NULL) && (inFileName != NULL) && (outDevice != NULL))
          {
          mCurPort = new Port(Port::eInFileOutDevice, portName, inFileName, outDevice);
          mPorts[portName] = mCurPort;
          if (silentStr != NULL)
            {
            mCurPort->setSilent(silentStr);
            }
          mThreadedExecutor.execute(mCurPort); // Start a thread on the port for receiving messages
          }
        else if ((portName != NULL) && ((inFileName != NULL) || !strcmp(typeStr,"loopback")) && (outFileName != NULL))
          {
          if (!strcmp(typeStr,"loopback"))
            {
            mCurPort = new Port(Port::eLoopback, portName, "", outFileName);
            }
          else
            {
            mCurPort = new Port(Port::eFiles, portName, inFileName, outFileName);
            }
          if (mCurPort == NULL)
            {
            throw Exception("Failed to create Port");
            }
          if (silentStr != NULL)
            {
            mCurPort->setSilent(silentStr);
            }
          mPorts[portName] = mCurPort;
          bool setRealtime = false;
          if (realtimeStr != NULL)
            {
            if (!strcmp(realtimeStr,"yes"))
              {
              setRealtime = true;
              }
            else if (!strcmp(realtimeStr,"no"))
              {
              }
            else
              {
              parserException("In <port> tag: value for 'realtime' must be 'yes' or 'no'");
              }
            mCurPort->setSleepForReplay(setRealtime);
            }

          mThreadedExecutor.execute(mCurPort); // Start a thread on the port for receiving messages
          }
        else
          {
          string missing = "Missing mandatory attributes to <port> tag: ";
          if (portName == NULL)
            {
            missing += "id ";
            }
          if (portDev == NULL) 
            {
            if ((inFileName == NULL) && (outDevice == NULL))
              {
              missing += "device or infilename+outdevice";
              }
            else if (inFileName == NULL)
              {
              missing += "infilename ";
              }
            else
              {
              missing += "outdevice";
              }
            }
          parserException(missing);
          }
        }
      else if (elem == "log")
        {
        mStateStack.push_back(eLog);
        char* portName=NULL;
        char* fileName=NULL;

        int i=0;
        while (attr[i] != NULL)
          {
          if (!strcmp(attr[i],"port"))
            {
            i++;
            portName = (char*) attr[i++];
            }
          else if (!strcmp(attr[i],"file"))
            {
            i++;
            fileName = (char*) attr[i++];
            }
          else 
            {
            parserException("Unexpected attribute: " + string(attr[i]) + " in <port> tag.");
            }
          }

        if ((portName != NULL) && (fileName != NULL))
          {
          Port* port = mPorts[portName];
          if (port==NULL)
            {
            parserException("Unknown port in <log> tag: " + string(portName));
            }
          port->startLog(fileName);
          }
        else
          {
          string missing = "Missing mandatory attributes to <port> tag: ";
          if (portName == NULL)
            {
            missing += "port ";
            }
          if (fileName == NULL)
            {
            missing += "file";
            }
          parserException(missing);
          }
        }
      else if (elem == "var")
        {
        mStateStack.push_back(eVar);
        mVarContainer.createVar(attr);
        }
      else if (elem == "shaper")
        {
        mStateStack.push_back(eShaper);
        char* rateStr=NULL;
        char* idStr=NULL;

        int i=0;
        while (attr[i] != NULL)
          {
          if (!strcmp(attr[i],"id"))
            {
            i++;
            idStr = (char*) attr[i++];
            }
          else if (!strcmp(attr[i],"rate"))
            {
            i++;
            rateStr = (char*) attr[i++];
            }
          else 
            {
            parserException("Unexpected attribute: " + string(attr[i]) + " in <var> tag.");
            }
          }

        string missing = "";
        if (idStr == NULL)
          {
          missing += "id ";
          }
        if (rateStr == NULL)
          {
          missing += "rate ";
          }

        if (missing != "")
          {
          parserException("Missing mandatory attribute(s) to shaper: " + missing);
          }

        Shaper* shaper = new Shaper(idStr);
        shaper->setRate(rateStr);
        mShapers[idStr]=shaper;
        }
      else if (elem == "include")
        {
        mStateStack.push_back(eInclude);
        char* fileStr=NULL;

        int i=0;
        while (attr[i] != NULL)
          {
          if (!strcmp(attr[i],"file"))
            {
            i++;
            fileStr = (char*) attr[i++];
            }
          else 
            {
            parserException("Unexpected attribute: " + string(attr[i]) + " in <include> tag.");
            }
          }

        if (fileStr != NULL)
          {
          PierFileContext* newContext = new PierFileContext();
          char slash = fileStr[0];
          if (slash == '/' || slash == '\\') // works for dos and linux/unix
            {
            newContext->mName = fileStr;
            }
          else
            {
            newContext->mName = mCurContext->mName;
            size_t lastSlash = newContext->mName.find_last_of("/\\");
            if (lastSlash != 0)
              {
              lastSlash += 1;
              newContext->mName.erase(lastSlash);
              newContext->mName.append(fileStr);
              }
            else // no slash at all
              {
              newContext->mName = fileStr;
              }
            }

          newContext->mStream.open(newContext->mName.c_str());
          if (newContext->mStream.is_open())
            {
            newContext->mParser = XML_ExternalEntityParserCreate(mCurContext->mParser,"",NULL);
            if (newContext->mParser) // returns 0 if out of memory
              {
              mPierfContextStack.push_back(newContext);
              mCurContext = newContext;
              }
            }
          else
            {
            parserException("Failed to open include file: " + newContext->mName);
            }
          }
        else
          {
          string missing = "Missing mandatory attributes to <include> tag: ";
          if (fileStr == NULL)
            {
            missing += "file ";
            }
          parserException(missing);
          }
        }
      else if (elem == "play")
        {
        char* sceneName = NULL;
        int i=0;
        while (attr[i] != NULL)
          {
          if (!strcmp(attr[i],"scene"))
            {
            i++;
            sceneName = (char*) attr[i++];
            }
          else
            {
            parserException("Unexpected attribute: " + string(attr[i]) + " in <scene> tag.");
            i++;
            }
          }
        if (sceneName != NULL)
          {
          Scene* scene = mScenes[sceneName];
          if (scene != NULL)
            {
            scene->send();
            }
          else
            {
            parserException("Unknown scene in <play> tag: " + string(sceneName));
            }
          }
        else
          {
          parserException("Missing mandatory attribute to tag <play> tag: 'scene'");
          }
        }
      else
        {
        parserException("Reading unexpected tag in <pierf> section: " + elem);
        }
      break;
    case eScene:
      if (elem == "seq")
        {
        mStateStack.push_back(eSeq);

        char* autoStr=NULL;
        char* repeatStr=NULL;
        int i=0;
        while (attr[i] != NULL)
          {
          if (!strcmp(attr[i],"auto"))
            {
            i++;
            autoStr = (char*) attr[i++];
            }
          if (!strcmp(attr[i],"repeat"))
            {
            i++;
            repeatStr = (char*) attr[i++];
            }
          else
            {
            parserException("Unexpected attribute: " + string(attr[i]) + " in <seq> tag.");
            i++;
            }
          }

        Seq* seq = new Seq();
        mCurSeq = seq;
        mSeqStack.push_back(seq);
        mCurScene->push_back(seq);
        seq->setOrEnheritAuto(autoStr, mCurScene);
        if (repeatStr != NULL)
          {
          seq->setRepeat(repeatStr);
          }
        }
      else
        {
        parserException("Reading unexpected tag in <scene> section: " + elem);
        }
      break;
    case eSeq:
      if (elem == "packet")
        {
        Packet* packet = handlePacketTag(attr, mCurSeq);
        mCurSeq->push_back(packet);
        }
      else if (elem == "sleep")
        {
        mStateStack.push_back(eSleep);
        char* millisecStr=NULL;
        char* autoStr=NULL;
        int i=0;
        while (attr[i] != NULL)
          {
          if (!strcmp(attr[i],"millisec"))
            {
            i++;
            millisecStr = (char*) attr[i++];
            }
          else if (!strcmp(attr[i],"auto"))
            {
            i++;
            autoStr = (char*) attr[i++];
            }
          else
            {
            parserException("Unexpected attribute: " + string(attr[i]) + " in <sleep> tag.");
            i++;
            }
          }

        if ((millisecStr != NULL))
          {
          SleepStep* sleep = new SleepStep();
          sleep->setOrEnheritAuto(autoStr,mCurSeq);
          sleep->setSleep(millisecStr);
          mCurSeq->push_back(sleep);
          }
        else
          {
          parserException("Missing mandatory attribute to tag <sleep> tag: 'millisec'");
          }
        }
      else if (elem == "counter")
        {
        mStateStack.push_back(eCounter);
        char* idStr=NULL;
        char* actionStr=NULL;
        char* valueStr=NULL;
        int i=0;
        while (attr[i] != NULL)
          {
          if (!strcmp(attr[i],"ref"))
            {
            i++;
            idStr = (char*) attr[i++];
            }
          else if (!strcmp(attr[i],"action"))
            {
            i++;
            actionStr = (char*) attr[i++];
            }
          else if (!strcmp(attr[i],"value"))
            {
            i++;
            valueStr = (char*) attr[i++];
            }
          else
            {
            parserException("Unexpected attribute: " + string(attr[i]) + " in <counter> tag.");
            i++;
            }
          }

        if ((idStr != NULL) && (actionStr != NULL))
          {
          Counter* counter = (Counter*) mVarContainer.getVar(idStr);
          if (counter == NULL)
            {
            parserException("Unknown variable ref in <counter> tag: " + string(idStr));
            }
          if (mVarContainer.getVarType(idStr) != VarContainer::eCounter)
            {
            parserException("The referenced variable in <counter> tag is not a counter: " + string(idStr));
            }
          CounterStep* counterStep = new CounterStep();
          counterStep->setCounter(counter);
          counterStep->setAction(actionStr);
          if (valueStr != NULL)
            {
            counterStep->setValue(valueStr);
            }
          if (mCurReceiveStep != NULL)
            {
            counterStep->setReceiveStep(mCurReceiveStep);
            }
          mCurSeq->push_back(counterStep);
          }
        else
          {
          string missing = "Missing mandatory attributes to <counter> tag: ";
          if (idStr == NULL)
            {
            missing += "ref ";
            }
          if (actionStr == NULL)
            {
            missing += "action";
            }
          parserException(missing);
          }
        }
      else if (elem == "receive")
        {
        if (mCurReceiveStep != NULL)
          {
          parserException("Nested receive is not supported.");
          }
        mStateStack.push_back(eReceive);
        char* portStr=NULL;
        char* nomatchStr=NULL;
        char* autoStr=NULL;
        int i=0;
        while (attr[i] != NULL)
          {
          if (!strcmp(attr[i],"port"))
            {
            i++;
            portStr = (char*) attr[i++];
            }
          else if (!strcmp(attr[i],"nomatch"))
            {
            i++;
            nomatchStr = (char*) attr[i++];
            }
          else if (!strcmp(attr[i],"auto"))
            {
            i++;
            autoStr = (char*) attr[i++];
            }
          else
            {
            parserException("Unexpected attribute: " + string(attr[i]) + " in <sleep> tag.");
            i++;
            }
          }

        if ((portStr != NULL))
          {
          Port* port = mPorts[portStr];
          if (port != NULL)
            {
            mCurReceiveStep = new ReceiveStep();
            mCurReceiveStep->setPort(port);
            mCurReceiveStep->setOrEnheritAuto(autoStr,mCurSeq);
            mCurSeq->push_back(mCurReceiveStep);
            mReceiveStepStack.push_back(mCurReceiveStep);
            if (nomatchStr != NULL)
              {
              if (!strcmp(nomatchStr,"loop"))
                {
                mCurReceiveStep->setNomatchLoop(true);
                }
              else if (strcmp(nomatchStr,"next")) // not matching next either
                {
                parserException("Wrong value for attribute nomatch. Expecting loop or next: " + string(nomatchStr));
                }
              }
            }
          else
            {
            parserException("Unknown port in <receive> tag: " + string(portStr));
            }
          }
        else
          {
          parserException("Missing mandatory attribute to tag <receive> tag: 'port'");
          }
        }
      else if (elem == "seq")
        {
        mStateStack.push_back(eSeq);
        char* autoStr=NULL;
        char* repeatStr=NULL;
        int i=0;
        while (attr[i] != NULL)
          {
          if (!strcmp(attr[i],"auto"))
            {
            i++;
            autoStr = (char*) attr[i++];
            }
          if (!strcmp(attr[i],"repeat"))
            {
            i++;
            repeatStr = (char*) attr[i++];
            }
          else
            {
            parserException("Unexpected attribute: " + string(attr[i]) + " in <seq> tag.");
            i++;
            }
          }

        Seq* seq = new Seq();
        mSeqStack.push_back(seq);
        mCurSeq->push_back(seq);
        seq->setOrEnheritAuto(autoStr, mCurSeq);
        mCurSeq = seq;
        if (repeatStr != NULL)
          {
          seq->setRepeat(repeatStr);
          }
        }
      else if (elem == "text")
        {
        mStateStack.push_back(eText);
        char* autoStr=NULL;
        int i=0;
        while (attr[i] != NULL)
          {
          if (!strcmp(attr[i],"auto"))
            {
            i++;
            autoStr = (char*) attr[i++];
            }
          else
            {
            parserException("Unexpected attribute: " + string(attr[i]) + " in <seq> tag.");
            i++;
            }
          }

        mCurText = new StringStep();
        mCurSeq->push_back(mCurText);
        mCurText->setOrEnheritAuto(autoStr, mCurSeq);
        }
      else if (elem == "multishaper")
        {
        mStateStack.push_back(eMultiShaper);

        if (mCurMultiShaper != NULL)
          {
          parserException("Unexpected tag in a <seq> : " + elem + " (nested multishaper not supported)");          
          }

        char* autoStr=NULL;
        int i=0;
        while (attr[i] != NULL)
          {
          if (!strcmp(attr[i],"auto"))
            {
            i++;
            autoStr = (char*) attr[i++];
            }
          else
            {
            parserException("Unexpected attribute: " + string(attr[i]) + " in <seq> tag.");
            i++;
            }
          }

        mCurMultiShaper = new MultiShaperStep();
        mCurSeq->push_back(mCurMultiShaper);
        mCurMultiShaper->setOrEnheritAuto(autoStr, mCurSeq);
        }
      else if (elem == "assign-variable")
        {
        mStateStack.push_back(eVarAssign);

        char* strName=NULL;
        char* strValue=NULL;
        char* strAuto=NULL;
        int i=0;
        while (attr[i] != NULL)
          {
          if (!strcmp(attr[i],"auto"))
            {
            i++;
            strAuto = (char*) attr[i++];
            }
          else if (!strcmp(attr[i],"name"))
            {
            i++;
            strName = (char*) attr[i++];
            }
          else if (!strcmp(attr[i],"value"))
            {
            i++;
            strValue = (char*) attr[i++];
            }
          else
            {
            parserException("Unexpected attribute: " + string(attr[i]) + " in <seq> tag.");
            i++;
            }
          }

        string missing = "";
        if (strName == NULL)
          {
          missing += "name ";
          }
        if (strValue == NULL)
          {
          missing += "value ";
          }
        if (missing == "")
          {
          VarAssignStep* varAssign = new VarAssignStep();
          varAssign->setVar(strName);
          varAssign->setFormula(strValue);
          mCurSeq->push_back(varAssign);
          }
        else
          {
          parserException("Missing mandatory argument to <variable-assign> tag: " + missing);
          }
        }
      else if (elem == "mirror")
        {
        if (mCurReceiveStep == NULL) // mirror can only occur if seq is a sub of a receivestep
          {
          parserException("Unexpected tag in a <seq> : " + elem);          
          }

        mStateStack.push_back(eMirror);

        char* autoStr=NULL;
        char* portStr=NULL;
        int i=0;
        while (attr[i] != NULL)
          {
          if (!strcmp(attr[i],"auto"))
            {
            i++;
            autoStr = (char*) attr[i++];
            }
          if (!strcmp(attr[i],"port"))
            {
            i++;
            portStr = (char*) attr[i++];
            }
          else
            {
            parserException("Unexpected attribute: " + string(attr[i]) + " in <seq> tag.");
            i++;
            }
          }
        if ((portStr != NULL))
          {
          Port* port = mPorts[portStr];
          if (port != NULL)
            {
            MirrorStep* mirror = new MirrorStep();
            mirror->setPort(port);
            mirror->setReceiveStep(mCurReceiveStep);
            mCurSeq->push_back(mirror); 
            }
          else
            {
            parserException("Unknown port in <receive> tag: " + string(portStr));
            }
          }
        else
          {
          parserException("Missing mandatory attribute to tag <receive> tag: 'port'");
          }

        }
      else if (elem == "print")
        {
        if (mCurReceiveStep == NULL) // mirror can only occur if seq is a sub of a receivestep
          {
          parserException("Unexpected tag in a <seq> : " + elem);          
          }

        mStateStack.push_back(ePrint);

        if (true)
          {
          PrintStep* step =  new PrintStep();
          step->setReceiveStep(mCurReceiveStep);
          mCurSeq->push_back(step); 
          }
        }
      else
        { // syntax error
        parserException("Unexpected tag in a <seq> : " + elem);
        }
      break;
    case eReceive:
      if (elem == "mirror")
        {
        mStateStack.push_back(eMirror);

        char* autoStr=NULL;
        char* portStr=NULL;
        int i=0;
        while (attr[i] != NULL)
          {
          if (!strcmp(attr[i],"auto"))
            {
            i++;
            autoStr = (char*) attr[i++];
            }
          if (!strcmp(attr[i],"port"))
            {
            i++;
            portStr = (char*) attr[i++];
            }
          else
            {
            parserException("Unexpected attribute: " + string(attr[i]) + " in <seq> tag.");
            i++;
            }
          }
        if ((portStr != NULL))
          {
          Port* port = mPorts[portStr];
          if (port != NULL)
            {
            MirrorStep* mirror = new MirrorStep();
            mirror->setPort(port);
            mirror->setReceiveStep(mCurReceiveStep);
            mCurReceiveStep->push_back(mirror); 
            }
          else
            {
            parserException("Unknown port in <receive> tag: " + string(portStr));
            }
          }
        else
          {
          parserException("Missing mandatory attribute to tag <receive> tag: 'port'");
          }

        }
      else if (elem == "print")
        {
        mStateStack.push_back(ePrint);

        if (true)
          {
          PrintStep* step =  new PrintStep();
          step->setReceiveStep(mCurReceiveStep);
          mCurReceiveStep->push_back(step); 
          }
        }
      else if (elem == "firstof")
        {
        mStateStack.push_back(eFirstOf);

        FirstOfStep* firstof = new FirstOfStep();
        firstof->setReceiveStep(mCurReceiveStep);
        mCurReceiveStep->push_back(firstof);
        mCurFirstOfStep = firstof;
        mFirstOfStepStack.push_back(firstof);
        }
      else if (elem == "counter")
        {
        mStateStack.push_back(eCounter);
        char* idStr=NULL;
        char* actionStr=NULL;
        int i=0;
        while (attr[i] != NULL)
          {
          if (!strcmp(attr[i],"ref"))
            {
            i++;
            idStr = (char*) attr[i++];
            }
          else if (!strcmp(attr[i],"action"))
            {
            i++;
            actionStr = (char*) attr[i++];
            }
          else
            {
            parserException("Unexpected attribute: " + string(attr[i]) + " in <counter> tag.");
            i++;
            }
          }

        if ((idStr != NULL) && (actionStr != NULL))
          {
          Counter* counter = (Counter*) mVarContainer.getVar(idStr);
          if (counter == NULL)
            {
            parserException("Unknown variable ref in <counter> tag: " + string(idStr));
            }
          if (mVarContainer.getVarType(idStr) != VarContainer::eCounter)
            {
            parserException("The referenced variable in <counter> tag is not a counter: " + string(idStr));
            }
          CounterStep* counterStep = new CounterStep();
          counterStep->setCounter(counter);
          counterStep->setAction(actionStr);
          counterStep->setReceiveStep(mCurReceiveStep);
          mCurReceiveStep->push_back(counterStep);
          }
        else
          {
          string missing = "Missing mandatory attributes to <counter> tag: ";
          if (idStr == NULL)
            {
            missing += "ref ";
            }
          if (actionStr == NULL)
            {
            missing += "action";
            }
          parserException(missing);
          }
        }
      else
        {
        parserException("Reading unexpected tag in <recieve> section: " + elem);
        }
      break;
    case eFirstOf:
      if (elem == "match")
        {
        mStateStack.push_back(eMatch);
        mHandlingMatchElems = true;

        MatchStep* matchStep = new MatchStep;
        mCurFirstOfStep->addMatchStep(matchStep);
        mCurMatch = matchStep; // Store it as a match (currently unused: nested match not supported
        mMatchStack.push_back(matchStep);

        Packet* matchPacket = new Packet();
        matchPacket->setAnalysisReady(true); // defined packets/matches are analysed by default as that is the input
        matchStep->setMatchPacket(matchPacket);
        mCurrentPacket = matchPacket; 
        }
      else
        {
        parserException("Reading unexpected tag in <firstof> section: " + elem);
        }
      break;
    case eMatch:
      // a match has a special seq that points to itself
      if (elem == "seq")
        {
        if (mCurMatch->size() > 0) // For this check a dedicated mCurMatch was needed on top of mCurSeq
          {
          parserException("Reading another <seq> in a <match> while there is already a <seq>. <match> can contain only one <seq>.");
          }
        mHandlingMatchElems = false; //once we are reading the seq, no more match elems may come
        mCurSeq = mCurMatch; // Take the match as a seq now (it is)
        mSeqStack.push_back(mCurMatch);

        mStateStack.push_back(eSeq);
        char* autoStr=NULL;
        char* repeatStr=NULL;
        int i=0;
        while (attr[i] != NULL)
          {
          if (!strcmp(attr[i],"auto"))
            {
            i++;
            autoStr = (char*) attr[i++];
            }
          if (!strcmp(attr[i],"repeat"))
            {
            i++;
            repeatStr = (char*) attr[i++];
            }
          else
            {
            parserException("Unexpected attribute: " + string(attr[i]) + " in <seq> tag.");
            i++;
            }
          }

        // a matchstep is derived from seq. No need to create one just set the attribs
        mCurSeq->setOrEnheritAuto(autoStr, mCurScene);
        if (repeatStr != NULL)
          {
          mCurSeq->setRepeat(repeatStr);
          }
        break; // Don't continue with packet parsing
        }
      else
        { // syntax error
        if (mCurMatch->size() > 0)
          {
          parserException("Unexpected tag in a <match> while we had already a <seq>, which should be the last tag in <match> : " + elem);
          }
        }
      // conscious fall-through! match and packet have same body (before <seq> in match)
    case ePacket:
      {
      bool checkMandatory = true; // Not all fields must be added for a match condition
      if (state == eMatch)
        {
        checkMandatory = false;
        }
      if (elem == "raw")
        {
        mStateStack.push_back(eRaw);
        mCurRaw = new Raw();
        if (mCurRaw == NULL)
          {
          parserException("Failed to allocate heap memory");
          }
        mCurrentPacket->push_back(mCurRaw);
        mCurElem = mCurRaw;
        mCurRaw->parseAttrib(attr, mCurrentPacket, checkMandatory); // may throw exception
        }
      else if (elem == "eth")
        {
        mStateStack.push_back(eEth);
        Ethernet* eth = new Ethernet();
        if (eth == NULL)
          {
          parserException("Failed to allocate heap memory");
          }
        mCurElem = eth;
        mCurrentPacket->push_back(eth);
        eth->parseAttrib(attr, mCurrentPacket, checkMandatory); // may throw exception
        }
      else if (elem == "vlans")
        {
        mStateStack.push_back(eVlans);
        Vlan* vlan =  new Vlan();
        if (vlan == NULL)
          {
          parserException("Failed to allocate heap memory");
          }
        mCurElem = vlan;
        mCurrentPacket->push_back(vlan);
        vlan->parseAttrib(attr, mCurrentPacket, checkMandatory); // may throw exception
        }
      else if (elem == "arp")
        {
        mStateStack.push_back(eArp);
        Arp* arp = new Arp();
        if (arp == NULL)
          {
          parserException("Failed to allocate heap memory");
          }
        mCurElem = arp;
        mCurrentPacket->push_back(arp);
        arp->parseAttrib(attr, mCurrentPacket, checkMandatory); // may throw exception
        }
      else if (elem == "iphdr")
        {
        mStateStack.push_back(eIpHdr);
        IpHdr* ipHdr = new IpHdr();
        if (ipHdr == NULL)
          {
          parserException("Failed to allocate heap memory");
          }

        mCurElem = ipHdr;
        mCurrentPacket->push_back(ipHdr);
        ipHdr->parseAttrib(attr, mCurrentPacket, checkMandatory); // may throw exception
        }
      else if (elem == "icmp")
        {
        mStateStack.push_back(eIgmp);
        Icmp* icmp = new Icmp(); 
        if (icmp == NULL)
          {
          parserException("Failed to allocate heap memory");
          }

        mCurElem = icmp;
        mCurrentPacket->push_back(icmp);
        icmp->parseAttrib(attr, mCurrentPacket, checkMandatory); // may throw exception
        }
      else if (elem == "igmp")
        {
        mStateStack.push_back(eIgmp);
 
        // Insert a general igmp layer, doing noting, but for compatibility with captured packets
        Igmp* igmp = new Igmp(); 
        if (igmp == NULL)
          {
          parserException("Failed to allocate heap memory");
          }

        mCurElem = igmp;
        mCurrentPacket->push_back(igmp);
        igmp->parseAttrib(attr, mCurrentPacket, checkMandatory); // may throw exception
        uchar version = igmp->getVersion();

        if (version == 2)
          {
          IgmpV2* igmpv2 = new IgmpV2();
          if (igmpv2 == NULL)
            {
            parserException("Failed to allocate heap memory");
            }
          mCurElem = igmpv2;
          mCurrentPacket->push_back(igmpv2);
          igmpv2->parseAttrib(attr, mCurrentPacket, checkMandatory); 
          }
        else if (version == 3)
          {
          mCurIgmpV3 = new IgmpV3();
          if (mCurIgmpV3 == NULL)
            {
            parserException("Failed to allocate heap memory");
            }
          mCurElem = mCurIgmpV3;
          mCurrentPacket->push_back(mCurIgmpV3);
          mCurIgmpV3->parseAttrib(attr, mCurrentPacket, checkMandatory); 

          if (mCurIgmpV3->getType() == 0x11)
            {
            mStateStack.push_back(eIgmpV3Query);
            }
          else if (mCurIgmpV3->getType() == 0x22)
            {
            mStateStack.push_back(eIgmpV3Report);
            }
          }
        else
          {
          parserException("Invalid value for attribute 'version' of tag <igmp>: only igmp version 2 and 3 are supported");
          }          
        }
      else if (elem == "udp")
        {
        mStateStack.push_back(eUdp);
        Udp* udp = new Udp(); // to be completed
        if (udp == NULL)
          {
          parserException("Failed to allocate heap memory");
          }
        mCurElem = udp;
        mCurrentPacket->push_back(udp);
        udp->parseAttrib(attr, mCurrentPacket, checkMandatory); // may throw exception
        }
      else
        { // syntax error
        parserException("Unexpected tag in a <packet> : " + elem);
        }
      break;
      }
    case eIgmpV3Query:
      if (elem == "source")
        {
        mStateStack.push_back(eIgmpV3Source);
        char* strAddress=NULL;
        int i=0;
        while (attr[i] != NULL)
          {
          if (!strcmp(attr[i],"address"))
            {
            i++;
            strAddress = (char*) attr[i++];
            }
          else
            {
            parserException("Unexpected attribute: " + string(attr[i]) + " in <source> tag.");
            i++;
            }
          }

        if (strAddress != NULL)
          {
          mCurIgmpV3->addSource(strAddress);
          }
        else
          {
          parserException("Missing mandatory attributes to <source> tag: address");
          }
        }
      else
        { // syntax error
        parserException("Unexpected tag in a <igmp> : " + elem);
        }
      break;
    case eIgmpV3Report:
      if (elem == "group")
        {
        mStateStack.push_back(eIgmpGroupRecord);
        char* strType=NULL;
        char* strTo=NULL;
        int i=0;
        while (attr[i] != NULL)
          {
          if (!strcmp(attr[i],"type"))
            {
            i++;
            strType = (char*) attr[i++];
            }
          else if (!strcmp(attr[i],"to"))
            {
            i++;
            strTo = (char*) attr[i++];
            }
          else
            {
            parserException("Unexpected attribute: " + string(attr[i]) + " in <group> tag.");
            i++;
            }
          }

        string missing="";

        if (strType == NULL)
          {
          missing += " type";
          }
        if (strTo == NULL)
          {
          missing += " type";
          }

        if (missing != "")
          {
          parserException("Missing mandatory attributes to <group> tag: " + missing);
          }
        mCurIgmpGroupRec = mCurIgmpV3->addGroupRecord(strType,strTo);
        mCurElem = mCurIgmpGroupRec;
        }
      else
        { // syntax error
        parserException("Unexpected tag in a <igmp> : " + elem);
        }
      break;
    case eIgmpGroupRecord:
      if (elem == "source")
        {
        mStateStack.push_back(eIgmpV3Source);
        char* strAddress=NULL;
        int i=0;
        while (attr[i] != NULL)
          {
          if (!strcmp(attr[i],"address"))
            {
            i++;
            strAddress = (char*) attr[i++];
            }
          else
            {
            parserException("Unexpected attribute: " + string(attr[i]) + " in <source> tag.");
            i++;
            }
          }

        if (strAddress != NULL)
          {
          mCurIgmpGroupRec->addSource(strAddress);
          }
        else
          {
          parserException("Missing mandatory attributes to <source> tag: address");
          }
        }
      else
        { // syntax error
        parserException("Unexpected tag in a <source> : " + elem);
        }
      break;

    case eMultiShaper:
      if (elem == "packet")
        {
        Packet* packet = handlePacketTag(attr,mCurMultiShaper);
        mCurMultiShaper->addPacket(packet);
        // tdb : add multishaper specific things
        }
      else
        { // syntax error
        parserException("Unexpected tag in a <multishaper> : " + elem);
        }
      break;

    // All element types: purpously fall-through
    case eRaw:
    case eEth:
    case eVlans:
    case eArp:
    case eIpHdr:
    case eIcmp:
    case eIgmp: // v2
    case eIgmpV3Source:
    case eUdp:
      {
      if (elem == "assign-variable")
        {
        if (!mHandlingMatchElems)
          {
          parserException("assign-variable inside packet layers is only allowed in match definition. Regular sent packets cannot have assign-variable tags.");
          }

        mStateStack.push_back(eVarAssign);

        char* strName=NULL;
        char* strValue=NULL;
        char* strAuto=NULL;
        int i=0;
        while (attr[i] != NULL)
          {
          if (!strcmp(attr[i],"auto"))
            {
            i++;
            strAuto = (char*) attr[i++];
            }
          else if (!strcmp(attr[i],"name"))
            {
            i++;
            strName = (char*) attr[i++];
            }
          else if (!strcmp(attr[i],"value"))
            {
            i++;
            strValue = (char*) attr[i++];
            }
          else
            {
            parserException("Unexpected attribute: " + string(attr[i]) + " in <seq> tag.");
            i++;
            }
          }

        string missing = "";
        if (strName == NULL)
          {
          missing += "name ";
          }
        if (strValue == NULL)
          {
          missing += "value ";
          }
        if (missing == "")
          {
          VarAssignStep* varAssign = mCurElem->getNewVarAssignStep();
          varAssign->setVar(strName);
          varAssign->setFormula(strValue);
          }
        else
          {
          parserException("Missing mandatory argument to <variable-assign> tag: " + missing);
          }
        }
      else
        {
        parserException("Unexpected tag in a packet layer tag: " + string(elem));
        }
      }
      break;
    case eSleep:
    case ePort:
    case eLog:
    case eVar:
    case eCounter:
    case ePlay:
    case eMirror:
    case ePrint:
    case eText:
    case eShaper:
    case eVarAssign:
      parserException("Unexpected new tag: " + elem + " - when expecting a close tag (of previous element) first.");
      break;
    }
  }  /* End of start_hndl */


void ParseConfig::end_hndl(const char *el) throw (Exception)
  {
  //cout << "End tag: " << el << endl;
  State state = curState();
  switch (state) // tdb: check that the right element is ended
    {
    case ePacket:
      mCurrentPacket->tryComplete(false); // If complete now, execution will be faster
      break;
    case eIgmpV3Query:
    case eIgmpV3Report:
      mStateStack.pop_back(); // The upper state is igmp, which also closes!
      break;
    case eInclude:
      mStateStack.push_back(eInclude); // Push it a second time, so that it remains once after popping: the include state must remain after the close and that untill the include file has been parsed and is closed.
      break;
    case eSeq:
      {
      mSeqStack.pop_back();
      if (mSeqStack.size() > 0)
        {
        mCurSeq = mSeqStack.back();
        }
      else
        {
        mCurSeq = NULL;
        }
      }
      break;
    case eReceive:
      mReceiveStepStack.pop_back();
      if (mReceiveStepStack.size() > 0)
        {
        mCurReceiveStep = mReceiveStepStack.back();
        }
      else
        {
        mCurReceiveStep = NULL;
        }
      break;
    case eFirstOf:
      mFirstOfStepStack.pop_back();
      if (mFirstOfStepStack.size() > 0)
        {
        mCurFirstOfStep = mFirstOfStepStack.back();
        }
      else
        {
        mCurFirstOfStep = NULL;
        }
      break;
    case eMatch:
      mMatchStack.pop_back();
      if (mMatchStack.size() > 0)
        {
        mCurMatch = mMatchStack.back();
        }
      else
        {
        mCurMatch = NULL;
        }
      break;
    case eMultiShaper:
      mCurMultiShaper = NULL;
      break;
    case ePort:
      break;
    case eIdle:
    case ePierf:
    case eLog:
    case eVar:
    case eCounter:
    case eScene:
    case eSleep:
    case eMirror:
    case ePrint:
    case eRaw:
    case eEth:
    case eVlans:
    case eArp:
    case eIpHdr:
    case eIcmp:
    case eIgmp:
    case eIgmpGroupRecord:
    case eIgmpV3Source:
    case eUdp:
    case ePlay:
    case eText:
    case eShaper:
    case eVarAssign:
      break;
    }
  mStateStack.pop_back();
  }  /* End of end_hndl */

void ParseConfig::char_hndl(const char *txt, int txtlen) throw (Exception)
  {
  string chars = string(txt,txtlen);

  State state = curState();
  switch (state) // tdb: check that the right element is ended
    {
    case eRaw:
      mCurRaw->addString(chars);
      break;
    case eText:
      mCurText->addString(chars);
      break;
    case eIdle:
    case ePierf:
    case eInclude:
    case ePort:
    case eLog:
    case eVar:
    case eCounter:
    case eScene:
    case eSeq:
    case eSleep:
    case eReceive:
    case eMirror:
    case ePrint:
    case eFirstOf:
    case eMatch:
    case ePacket:
    case eEth:
    case eVlans:
    case eArp:
    case eIpHdr:
    case eIcmp:
    case eIgmp:
    case eIgmpV3Query:
    case eIgmpV3Report:
    case eIgmpGroupRecord:
    case eIgmpV3Source:
    case eUdp:
    case ePlay:
    case eShaper:
    case eMultiShaper:
    case eVarAssign:
      break;
    }

  }  /* End char_hndl */

string ParseConfig::getParserLine()
  {
  stringstream line;
  line << XML_GetCurrentLineNumber(mCurContext->mParser);
  return line.str();
  }

void ParseConfig::parse(string& configFile) throw (Exception)
  {
  mCurContext = new PierFileContext;
  mPierfContextStack.push_back(mCurContext);

  mCurContext->mName = configFile;
  mCurContext->mParser = XML_ParserCreate(NULL);
  if (! mCurContext->mParser) {
    fprintf(stderr, "Couldn't allocate memory for parser\n");
    exit(-1);
  }
  XML_SetElementHandler(mCurContext->mParser, pierfc_start_hndl, pierfc_end_hndl);
  XML_SetCharacterDataHandler(mCurContext->mParser, pierfc_char_hndl);
  XML_SetUserData(mCurContext->mParser,this);

  mCurContext->mStream.open(configFile.c_str());
  if (mCurContext->mStream.is_open())
    {
    string pierfcLine;
    int len;
    while (mCurContext != NULL)
      {
      while (! mCurContext->mStream.eof() )
        {
        getline (mCurContext->mStream,pierfcLine);
        pierfcLine += "\n"; // add newline again.
        len = pierfcLine.length();
        int result;
        try
          {
          result = XML_Parse(mCurContext->mParser, pierfcLine.c_str(), len, 0);
          }
        catch (Exception e)
          {
          stringstream exceptstr;
          exceptstr << "while parsing " << mCurContext->mName << ":" << XML_GetCurrentLineNumber(mCurContext->mParser) << " :" << endl;
          mPierfContextStack.pop_back();
          while (mPierfContextStack.size() > 0)
            {
            mCurContext = mPierfContextStack.back();
            exceptstr << "  (included from " << mCurContext->mName << ":" << XML_GetCurrentLineNumber(mCurContext->mParser) << ")" << endl;
            mCurContext->mStream.close();
            XML_ParserFree(mCurContext->mParser);
            delete mCurContext;
            mPierfContextStack.pop_back();
            }
          exceptstr << "  " << e.what();
          throw Exception(exceptstr.str());
          }
        if (!result)
          {
          stringstream exceptstr;
          exceptstr << "XML parse error in state " << stateToString(curState()) << " at " << mCurContext->mName << ":" << XML_GetCurrentLineNumber(mCurContext->mParser) << ": "
            << XML_ErrorString(XML_GetErrorCode(mCurContext->mParser)) << flush;
          throw Exception(exceptstr.str());
          }
        }
      mCurContext->mStream.close();
      mPierfContextStack.pop_back(); // The top element is always the current
      if (mPierfContextStack.size()>0)
        {
        XML_ParserFree(mCurContext->mParser);
        delete mCurContext;
        mCurContext = mPierfContextStack.back();

        // restore the state -> now we can take the include state from the state stack
        State state = curState();
        if (state == eInclude)
          {
          mStateStack.pop_back();
          }
        }
      else
        {
        XML_Parse(mCurContext->mParser, " ", 0, 1);
        XML_ParserFree(mCurContext->mParser);
        delete mCurContext;
        mCurContext = NULL;
        }
      }
    }
  else
    {
    throw Exception("Failed to open pierf config file: " + mCurContext->mName);
    }

  }

void ParseConfig::parse(const char* configFile) throw (Exception)
  {
  string cFile = configFile;
  parse(cFile);
  }

string ParseConfig::stateToString(State state)
  {
  switch (state)
    {
    case eIdle:
      return "idle";
      break;
    case ePierf:
      return "pier";
      break;
    case ePort:
      return "port";
      break;
    case eInclude:
      return "include";
      break;
    case eLog:
      return "log";
      break;
    case eScene:
      return "scene";
      break;
    case eSeq:
      return "seq";
      break;
    case ePacket:
      return "packet";
      break;
    case eSleep:
      return "sleep";
      break;
    case eReceive:
      return "receive";
      break;
    case eMirror:
      return "mirror";
      break;
    case ePrint:
      return "print";
      break;
    case eFirstOf:
      return "firstof";
      break;
    case eMatch:
      return "match";
      break;
    case eRaw:
      return "raw";
      break;
    case eEth:
      return "eth";
      break;
    case eVlans:
      return "vlans";
      break;
    case eArp:
      return "arp";
      break;
    case eIpHdr:
      return "iphdr";
      break;
    case eIcmp:
      return "icmp";
      break;
    case eIgmp:
      return "igmp";
      break;
    case eIgmpV3Query:
      return "igmp v3 query";
      break;
    case eIgmpV3Report:
      return "igmp v3 report";
      break;
    case eIgmpGroupRecord:
      return "group";
      break;
    case eIgmpV3Source:
      return "source";
      break;
    case eUdp:
      return "udp";
      break;
    case ePlay:
      return "play";
      break;
    case eCounter:
      return "counter";
      break;
    case eVar:
      return "var";
      break;
    case eText:
      return "text";
      break;
    case eShaper:
      return "shaper";
      break;
    case eMultiShaper:
      return "multishaper";
      break;
    case eVarAssign:
      return "assign-variable";
      break;
    }
  return "none";
  }

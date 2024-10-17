// Copyright (c) 2006-2011, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "Scene.hpp"
#include <sstream>

void Scene::push_back(Seq* seq)
  {
  mParts.push_back(seq);
  }

void Scene::push_back(SeqRef* seqRef)
  {
  mParts.push_back(seqRef);
  }

void Scene::play()
  {
  vector<PlayStep *>::iterator iter;
  for (iter = mParts.begin();iter != mParts.end();iter++)
    {
    PlayStep* elem= *iter;
    elem->play();
    }
  }

Scene::Scene()
  :mAuto(eAutoEnherit)
  {
  }

Scene::~Scene()
  {
  vector<PlayStep *>::iterator iter;
  for (iter = mParts.begin();iter != mParts.end();iter++)
    {
    PlayStep* elem= *iter;
    delete elem;
    }
  // if it is a SeqRef, SeqRef will only delete the reference, not the named Seq itself. 
  }

void Scene::setName(char* name)
  {
  mName = name;
  }

string Scene::getString() const
  {
  stringstream retval;
  retval << "<scene ";

  if (mName.size() > 0)
    {
    retval << "id=\"" << mName << "\" ";
    }

  retval << ">" << endl;

  vector<PlayStep *>::const_iterator iter;
  for (iter = mParts.begin();iter != mParts.end();iter++)
    {
    PlayStep* elem= *iter;
    retval << elem->getString();
    }    

  retval << "</scene>" << endl << flush;
  return retval.str();
  }

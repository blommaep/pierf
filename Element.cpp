// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "Element.hpp"

Element::Element()
  :mVarAssignSeq(NULL)
  {
  }

Element::~Element()
  {
  }

VarAssignStep* Element::getNewVarAssignStep()
  {
  VarAssignStep* varStep = NULL;
  varStep = new VarAssignStep();
  if (mVarAssignSeq == NULL)
    {
    mVarAssignSeq = new VarAssignSeq();
    }
  mVarAssignSeq->push_back(varStep);
  return varStep;
  }

    
void Element::playVarAssigns(Element* receivedElem)
  {
  if (mVarAssignSeq == NULL) // No varassigns
    {
    return;
    }

  vector<VarAssignStep *>::iterator iter;
  for (iter = mVarAssignSeq->begin();iter != mVarAssignSeq->end();iter++)
    {
    VarAssignStep* varStep= *iter;
    varStep->setElement(receivedElem); // set the rxElement from which to copy nly just before play
    varStep->play();    
    varStep->setElement(NULL); // and set it back to NULL
    }

  }


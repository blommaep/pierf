// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "Interface.hpp"
#include <sstream>

Interface::Interface(char* id)
  : mId(id)
  {
  }

Interface::~Interface()
  {
  vector<Element *>::iterator iter;
  for (iter = mElems.begin();iter != mElems.end();iter++)
    {
    Element* elem= *iter;
    delete elem;
    }  

  }

void Interface::push_back(Element* elem)
  {
  mElems.push_back(elem);
  }

void Interface::setPort(Port* outport)
  {
  mOutPort = outport;
  }


string Interface::getString()
  {
  stringstream retval;
  retval << "<interface";
  if (mOutPort != NULL)
    {
    retval << " port=\"" << mOutPort->getId() << "\"";
    }
  retval << ">" << endl;
  vector<Element *>::iterator iter;
  for (iter = mElems.begin();iter != mElems.end();iter++)
    {
    Element* elem= *iter;
    retval << "  " << elem->getString() << endl;
    }

  retval << "</interface>" << endl;
  return retval.str();
  }

// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "SignatureElem.hpp"
#include "Arp.hpp"
#include "Vlan.hpp"
#include "IpHdr.hpp"
#include "IgmpV2.hpp"
#include "Igmp.hpp"

#include <iostream> // for cout and cin
#include <fstream>
#include <sstream>
#include <string.h>
#include <typeinfo>

SignatureElem::SignatureElem()
  : mHasName(false)
  {
  }


void SignatureElem::parseAttrib(const char** attr, AutoObject* parent, bool checkMandatory, bool storeAsString) throw (Exception)
  {
  //char* autoStr=NULL;
  int i=0;
  while (attr[i] != NULL)
    {
    if (!strcmp(attr[i],"name"))
      {
      i++;
      mName = string(attr[i++]);
      mHasName=true;
      }
    else
      {
      throw Exception("Unexepected attribute: " + string(attr[i]) + " in <signature> tag.");
      i++;
      }
    }

  if (!mHasName)
    {
    mName="noname";
    }

  }


bool SignatureElem::copyVar() throw (Exception)
  {
  return false;
  }

bool SignatureElem::hasVar()
  {
  return false;
  }

uchar* SignatureElem::copyTo(uchar* toPtr)
  {
  toPtr = mSignature.copyTo(toPtr);
  return toPtr;
  }

uchar* SignatureElem::copyTail(uchar* toPtr)
  {
  return toPtr;
  }

bool SignatureElem::analyze_Head(uchar*& fromPtr, ulong& remainingSize)
  {
  if (!mSignature.analyze(fromPtr,remainingSize)) return false;
  return true;
  }

bool SignatureElem::analyze_Tail(uchar*& fromPtr, ulong& remainingSize)
  {
  return true;
  }

Element* SignatureElem::analyze_GetNextElem()
  {
  return NULL;
  }

bool SignatureElem::match(Element* other)
  {
  if (typeid(*other) != typeid(SignatureElem))
    {
    return false;
    }
  SignatureElem* otherSig = (SignatureElem*) other;

  if (!mSignature.match(otherSig->mSignature))
    {
    if (otherSig->mSignature.RxSignatureFound()) // otherSig is the RX'd one
      {
      cout << "Signature mismatch: " << mName << ": " << mSignature.getMismatch() << endl << flush;
      }
    else
      {
      return false;
      }
    }

  return true;  
  }

string SignatureElem::getString()
  {
  stringstream retval;
  retval << "<signature ";
  if (mHasName)
    {
    retval << "name=\"" << mName << "\" ";
    }
  retval << "/>" << flush;
  return retval.str();
  }

bool SignatureElem::getString(string& stringval, const char* fieldName)
  {
  if (!strcmp(fieldName, "name"))
    {
    stringval=mName;
    return true;
    }

  return false;
  }

ulong SignatureElem::getSize()
  {
  return 8;
  }

ulong SignatureElem::getTailSize()
  {
  return 0;
  }


bool SignatureElem::tryComplete(ElemStack& stack)
  {
  return true;
  }

bool SignatureElem::checkComplete()
  {
  return true;
  }

string SignatureElem::whatsMissing()
  {
  return "";  
  }

Element* SignatureElem::getNewBlank()
  {
  SignatureElem* signatureElem = new SignatureElem();
  return (Element*) signatureElem;
  }

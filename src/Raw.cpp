// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "Raw.hpp"

#include <sstream>
#include <string.h>
#include <typeinfo>

Raw::Raw()
  {
//  mSize.setDefault((ushort)0); // removed: there is no reason to have a default for size (default is the size of the binary data, but that is coded) and it disturbs analyze_Head to have a value for mSize (could change that too, but didn't)
  mType.setDefault((ushort)0); // in absence of better design, using a 16 bits integer for the type: 0 = hex, 1 = text
  mData.setStrict(false);
  mSize.displayDecimal();
  mDataFieldEntered = false;
  }

void Raw::parseAttrib(const char** attr, AutoObject* parent, bool checkMandatory, bool storeAsString) noexcept(false)
  {
  int i=0;
  char* autoStr = NULL;
  const char* dataStr = NULL; // data and filler must be assured to be done last: parsing depends on type
  const char* fillerStr = NULL; 

  while (attr[i] != NULL)
    {
    if (!strcmp(attr[i],"auto"))
      {
      i++;
      autoStr = (char*) attr[i++];
      }
    else if (!strcmp(attr[i],"size"))
      {
      i++;
      setSize(attr[i++]);
      }
    else if (!strcmp(attr[i],"filler"))
      {
      i++;
      fillerStr = attr[i++];
      }
    else if (!strcmp(attr[i],"type"))
      {
      i++;
      setType(attr[i++]);
      }
    else if (!strcmp(attr[i],"data")) // Optional: can be defined as a data tag for too for variable input
      {
      i++;
      dataStr = attr[i++];
      }
    else
      {
      throw Exception("Unexepected attribute: " + string(attr[i]) + " in <raw> tag.");
      }
    }

  if (fillerStr != NULL)
    {
    setFiller(fillerStr);
    }

  setOrEnheritAuto(autoStr,parent);
  if (dataStr != NULL)
    {
    mData.setManual(dataStr, storeAsString);
    mDataFieldEntered = true;
    }
  }

void Raw::addString(const char* inString) noexcept(false)
  {
  if (mDataFieldEntered)
    {
    throw Exception("Raw cannot contain a body when it has a data tag");
    }
  mData.addString(inString);
  if (mData.size() > 0) // the string to add contained something useful
    {
    mData.wasManuallySet();
    }
  }

void Raw::addString(const string& inString) noexcept(false)
  {
  if (mDataFieldEntered)
    {
    throw Exception("Raw cannot contain a body when it has a data tag");
    }
  mData.addString(inString);
  if (mData.size() > 0) // the string to add contained something useful
    {
    mData.wasManuallySet();
    }
  }

void Raw::setSize(ulong32 size)
  {
  mSize.setManual(intToString(size).c_str(), false);
  }

void Raw::setSize(const char* size)
  {
  mSize.setManual(size, false);
  }

void Raw::setType(const char* type) noexcept(false)
  {
  if (!strcmp(type,"hex"))
    {
    mType.setManual(intToString(ulong32(0)).c_str(), false);
    mFiller.setInputChar(false);
    mData.setInputChar(false);
    }
  else if (!strcmp(type,"text"))
    {
    ulong32 ltype = 1;
    mType.setManual(intToString(ltype).c_str(), false);
    mFiller.setInputChar(true);
    mData.setInputChar(true);
    }
  else
    {
    throw Exception("Invalid type: must be hex or text");
    }
  }

void Raw::setFiller(const char* filler)
  {
  mFiller.resetString();
  mFiller.setManual(filler, false);
  }

string Raw::getString()
  {
  stringstream retval;
  retval << "<raw";
  if (mType.getValue() == 1)
    {
    retval << " type=\"text\"";
    }
  if (mSize.isManual())
    {
    retval << " size=\"" << mSize.getConfigString() << "\"";
    }
  if (mFiller.isManual())
    {
    retval << " filler=\"" << mFiller.getConfigString() << "\"";
    }

  if (mData.isString() || mData.isVar())
    {
    retval << " data=\"" << mData.getConfigString() << "\" >";
    }
  else
    {
    retval << ">" << mData.getString();
    }
 
  if (hasVarAssigns())
    {
    retval << endl << getVarAssignsString() << "  ";
    }
 
  retval << "</raw>" << flush;
  return retval.str();
  }

bool Raw::getString(string& stringval, const char* fieldName)
  {
  if (!strcmp(fieldName, "size"))
    {
    return mSize.getString(stringval);
    }
  else if (!strcmp(fieldName, "filler"))
    {
    return mFiller.getString(stringval);
    }
  else if (!strcmp(fieldName, "data")) // Special: data isn't really a field but rather tag content for raw...
    {
    return mData.getString(stringval);
    }
  return false;
  }

ulong32 Raw::getSize()
  {
  if (mSize.isManual())
    {
    return (ulong32) mSize.getValue();
    }
  return mData.size();
  }

ulong32 Raw::getTailSize()
  {
  return 0;
  }

bool Raw::copyVar() noexcept(false)
  {
  bool copy = false;
  bool res;
  res = mData.copyVar();
  copy = copy || res;
  res = mFiller.copyVar();
  copy = copy || res;
  res = mSize.copyVar();
  copy = copy || res;
  return copy;
  }

uchar* Raw::copyTo(uchar* toPtr)
  {
  ulong32 curSize = mData.size();
  ulong32 targetSize = 0;
  if (mSize.isManual())
    {
    targetSize = (ulong32) mSize.getValue();
    }
  if (curSize < targetSize || targetSize==0)
    {
    // start by copying the defined content
    toPtr = mData.copyTo(toPtr);

    // Now the filler
    ulong32 fillerSize = mFiller.size();
    if (fillerSize > 0) // use a specific filler
      {
      vector<uchar>::iterator fillerStart, fillerEnd;
      fillerStart = mFiller.begin();
      fillerEnd = mFiller.end();
      while (targetSize > curSize + fillerSize)
        {
        toPtr = mFiller.copyTo(toPtr);
        curSize += fillerSize;
        }
      while (targetSize > curSize) // fill byte by byte now, as we don't need the full filler anymore
        {
        *toPtr = *fillerStart;
        toPtr++;
        fillerStart++;
        curSize ++;
        }
      }
    else // just expand using the default filler (0)
      {
      while (targetSize > curSize)
        {
        *toPtr++ = 0x0;
        curSize++;
        }
      }
    }
  else // just print te Data part untill mSize length
    {
    toPtr = mData.copyTo(toPtr,targetSize);
    }

  return toPtr;
  }

uchar* Raw::copyTail(uchar* toPtr)
  {
  return toPtr;
  }

bool Raw::analyze_Head(uchar*& fromPtr, ulong32& remainingSize)
  {
  if (mSize.hasValue()) // copied from match packet in compare: in compare mode, always take exactly same size if it is specified
    {
    ulong32 rawSize = (ulong32) mSize.getValue();
    if (remainingSize < rawSize)
      {
      return false;
      }
    mData.addBytes(fromPtr,rawSize);
    fromPtr += rawSize;
    remainingSize -= rawSize;
    }
  else
    {
    mData.addBytes(fromPtr,remainingSize);
    fromPtr += remainingSize;
    remainingSize = 0;
    }
  mData.wasCaptured();
  return true;
  }

bool Raw::analyze_Tail(uchar*& fromPtr, ulong32& remainingSize)
  {
  return true;
  }

Element* Raw::analyze_GetNextElem()
  {
  return NULL;
  }

bool Raw::checkComplete()
  {
  return true; 
  }

bool Raw::tryComplete(ElemStack& stack)
  {
  if (checkComplete())
    {
    return true;
    }

  return true; // always complete
  }

string Raw::whatsMissing()
  {
  return "";
  }

bool Raw::match(Element* other)
  {
  if (typeid(*other) != typeid(Raw))
    {
    return false;
    }
  Raw* otherRaw = (Raw*) other;

  if (!mData.match(otherRaw->mData))
    {
    return false;
    }

  return true;  
  }

Element* Raw::getNewBlank()
  {
  Raw* raw = new Raw();
  if (mSize.isManual())
    {
    raw->mSize = mSize;
    }
  return (Element*) raw;
  }

void Raw::copySize(Raw* fromRaw)
  {
  mSize=fromRaw->mSize;
  }

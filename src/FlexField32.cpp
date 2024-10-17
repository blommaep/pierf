// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "FlexField32.hpp"
#include "VarContainer.hpp"

#include <iostream> // for cout and cin
#include <iomanip>
#include <fstream>
#include <sstream>

FlexField32::FlexField32()
  {
  int i;
  for (i=0;i<4;i++)
    {
    mValueStates.mChar[i] = eUndef;
    }
  mValueStates.mShort[0] = eUndef;
  mValueStates.mShort[1] = eUndef;
  mData.uLong = 0; //Reset the entire long to 0 as a start
  }

void FlexField32::stringToVal(const char* inString) noexcept(false)
  {
  ulong32 temp = textToLong(inString);
  mData.uLong = temp; 
  }

uchar FlexField32::stringToChar(const char* val) noexcept(false)
  {
  ulong32 temp = textToLong(val);
  if (temp < 0 || temp > 0xFF)
    {
    throw Exception("Invalid input string for what should be a 2 byte integer value");
    }
  return temp;
  }

ushort FlexField32::stringToShort(const char* val) noexcept(false)
  {
  ulong32 temp = textToLong(val);
  if (temp < 0 || temp > 0xFFFF)
    {
    throw Exception("Invalid input string for what should be a 2 byte integer value");
    }
  return temp;
  }

Field::ValueState FlexField32::how2ValueState(How how)
   {
   switch (how)
     {
     case eHowUndef:
       return eUndef;
     case eHowDefault:
       return eDefault;
     case eHowAuto:
       return eAuto;
     case eHowManual:
       return eManual;
     case eHowCaptured:
       return eCaptured;
     }
   return eUndef; // pro forma, will never get here
   }

void FlexField32::setDefault(const char* inString) noexcept(false)
  {
  stringToVal(inString);
  for (int i=0;i<4;i++)
    {
    mValueStates.mChar[i] = eDefault;
    }
  wasDefaulted();
  }

void FlexField32::setManualFromValue(const char* inString) noexcept(false)
  {
  stringToVal(inString);
  for (int i=0;i<4;i++)
    {
    mValueStates.mChar[i] = eManual;
    }
  wasManuallySet();
  }

void FlexField32::setAuto(const char* inString) noexcept(false)
  {
  stringToVal(inString);
  for (int i=0;i<4;i++)
    {
    mValueStates.mChar[i] = eAuto;
    }
  wasAutoSet();
  }

void FlexField32::setName(const char* inString)
  {
  mFieldNames[0] = inString;
  }

// tbd: major rework, allowing the variables to also work with a Flexfield:
//      Must have elements mLong, mShort and mChar that each are of the Field type...
//      Maybe immeditely make a more generic container that allows conditional filling
void FlexField32::setLong(const char* fieldname, ulong32 val, How how)
  {
  mData.uLong=htonl32(val);
  int i;
  ValueState valueState = how2ValueState(how);
  for (i=0;i<4;i++)
    {
    mValueStates.mChar[i] = valueState;
    }
  mValueState = valueState;
  }

void FlexField32::setLong(const char* fieldname, const char* val, How how) noexcept(false)
  {
  ulong32 temp = textToLong(val);
  setLong(fieldname, temp, how);
  }

void FlexField32::setShort(const char* fieldname, int pos, ushort val, How how) noexcept(false)
  {
  if (pos>1)
    {
    throw Exception ("Bug: trying to set unexisting value");
    }
  mFieldNames[pos*2] = fieldname;
  ValueState valueState = how2ValueState(how);
  mValueStates.mChar[pos*2] = valueState;
  mValueStates.mChar[pos*2+1] = valueState;
  mValueStates.mShort[pos] = valueState;
  mData.uShort[pos] = htons(val);
  }

void FlexField32::setShort(const char* fieldname, int pos, const char* val, How how) noexcept(false)
  {
  if (val[0] == '$')
    {
    mFieldNames[pos*2] = fieldname;
    mValueStates.mShort[pos] = eVar;
    mValueStates.mChar[pos*2] = eVar;
    mValueStates.mChar[pos*2+1] = eVar;
    mVars[pos*2] = VarContainer::getVar(val);
    if (mVars[pos*2] == NULL)
      {
      throw Exception ("Trying to assign unexisting variable to field " + string(fieldname));
      }
    }
  else
    {
    setShort(fieldname, pos, stringToShort(val), how);
    }
  }

void FlexField32::setChar(const char* fieldname, int pos, uchar val, How how) noexcept(false)
  {
  if (pos>3)
    {
    throw Exception ("Bug: trying to set unexisting value");
    }
  mFieldNames[pos] = fieldname;
  mValueStates.mChar[pos] = how2ValueState(how);
  mData.uChar[pos] = val;
  }

void FlexField32::setChar(const char* fieldname, int pos, const char* val, How how) noexcept(false)
  {
  if (val[0] == '$')
    {
    mFieldNames[pos] = fieldname;
    mValueStates.mChar[pos] = eVar;
    mValueStates.mChar[pos] = eVar;
    mVars[pos] = VarContainer::getVar(val);
    if (mVars[pos] == NULL)
      {
      throw Exception ("Trying to assign unexisting variable to field " + string(fieldname));
      }
    }
  else
    {
    setChar(fieldname, pos, stringToChar(val), how);
    }
  }

uchar FlexField32::getChar(int pos)
  {
  return mData.uChar[pos];
  }

ushort FlexField32::getShort(int pos)
  {
  return mData.uShort[pos];
  }

ulong32 FlexField32::getLong()
  {
  return mData.uLong;
  }

bool FlexField32::hasValue()
  {
  int i;
  for (i=0;i<4;i++)
    {
    if (mValueStates.mChar[i] == eUndef)
      {
      return false;
      }
    }
  return true;
  }

string FlexField32::getStringFromBinary() const
  {
  stringstream retval;
  //  retval.setf(ios::uppercase); // print hex in uppercase
  //  retval.setf(ios::hex,ios::basefield); // print hex in uppercase
  //  retval.fill('0');

  if (mValueState != eUndef) // The field is one big 4bytes field
    {
    if (mFieldNames[0] != "unused")
      {
      retval << " " << mFieldNames[0] << "=\"" << ntohl32(mData.uLong) << "\"";
      }
    }
  else 
    {
    if (mValueStates.mShort[0] != eUndef) // The field is split in a 2 bytes field and some more later
      {
      if (mFieldNames[0] != "unused")
        {
        retval << " " << mFieldNames[0] << "=\"" << ntohs(mData.uShort[0]) << "\"";
        }
      }
    else
      {
      if (mFieldNames[0] != "unused")
        {
        retval << " " << mFieldNames[0] << "=\"" << mData.uChar[0] << "\"";
        }
      if (mFieldNames[1] != "unused")
        {
        retval << " " << mFieldNames[1] << "=\"" << mData.uChar[1] << "\"";
        }
      }

    if (mValueStates.mShort[1] != eUndef) // The last 2 bytes are one field
      {
      if (mFieldNames[2] != "unused")
        {
        retval << " " << mFieldNames[2] << "=\"" << ntohs(mData.uShort[1]) << "\"";
        }
      }
    else
      {
      if (mFieldNames[2] != "unused")
        {
        retval << " " << mFieldNames[2] << "=\"" << mData.uChar[2] << "\"";
        }
      if (mFieldNames[3] != "unused")
        {
        retval << " " << mFieldNames[3] << "=\"" << mData.uChar[3] << "\"";
        }
      }
    }
  retval << flush;
  return retval.str();
  }

bool FlexField32::getStringFromBinary(string& stringval) const
  {
  if (mValueState == eUndef)
    {
    return false;
    }
  stringstream retval;
  retval << mData.uLong;
  retval << flush;
  stringval = retval.str();
  return true;
  }

string FlexField32::getString() const
  {
  return Field::getString();
  }

bool FlexField32::getString(string& stringval, const string& fieldname) const
  {
  stringstream retval;
  bool ok = false;

  if (mValueState != eUndef) // The field is one big 4bytes field
    {
    if (mFieldNames[0] == fieldname)
      {
      retval << mData.uLong;
      ok = true;
      }
    }
  else 
    {
    if (mValueStates.mShort[0] != eUndef) // The field is split in a 2 bytes field and some more later
      {
      if (mFieldNames[0] == fieldname)
        {
        retval << mData.uShort[0];
        ok = true;
        }
      }
    else
      {
      if (mFieldNames[0]  == fieldname)
        {
        retval << mData.uChar[0];
        ok = true;
        }
      else if (mFieldNames[1]  == fieldname)
        {
        retval << mData.uChar[1];
        ok = true;
        }
      }

    if (mValueStates.mShort[1] != eUndef) // The last 2 bytes are one field
      {
      if (mFieldNames[2] == fieldname)
        {
        retval << mData.uShort[1];
        ok = true;
        }
      }
    else
      {
      if (mFieldNames[2]  == fieldname)
        {
        retval << mData.uChar[2];
        ok = true;
        }
      else if (mFieldNames[3]  == fieldname)
        {
        retval << mData.uChar[3];
        ok = true;
        }
      }
    }

  if (!ok)
    {
    return false;
    }
  retval << flush;
  stringval = retval.str();
  return true;
  }


uchar* FlexField32::copyTo(uchar* toPtr)
  {
  ulong32* tmp = (ulong32*) toPtr;
  *tmp++ = mData.uLong;
  return (uchar*) tmp;
  }

bool FlexField32::analyze(uchar*& fromPtr, ulong32& remainingSize)
  {
  if (remainingSize < 4)
    {
    return false;
    }
  mData.uLong = * (ulong32*) fromPtr;

  //valueStates remain undef: must be set by the higher protocol

  fromPtr += 4;
  remainingSize -= 4;
  return true;
  }

bool FlexField32::match(FlexField32& other)
  {
  if (isPrintable() && other.hasValue())
    {
    if (isString() || other.isString())
      {
      return matchByString(other);
      }
    
    if (mData.uLong != other.mData.uLong) //unused bytes should be reset, so compare can be done easily
      {
      return false;
      }
    }
  //else
  return true;
  }

bool FlexField32::copyVar() noexcept(false)
  {
  bool hasVar = false;
  if (mValueState == eVar) // one single 4 bytes field
    {
    setManualFromValue(mVar->getStringValue().c_str());
    return true;
    }
  else
    {
    if (mValueStates.mShort[0] == eVar)
      {
      mData.uShort[0] = stringToShort(mVars[0]->getStringValue().c_str());
      hasVar = true;
      }
    else
      {
      if (mValueStates.mChar[0] == eVar)
        {
        mData.uChar[0] = stringToChar(mVars[0]->getStringValue().c_str());
        hasVar = true;
        }
      if (mValueStates.mChar[1] == eVar)
        {
        mData.uChar[1] = stringToChar(mVars[1]->getStringValue().c_str());
        hasVar = true;
        }
      }
    if (mValueStates.mShort[1] == eVar)
      {
      mData.uShort[1] = stringToShort(mVars[2]->getStringValue().c_str());
      hasVar = true;
      }
    else
      {
      if (mValueStates.mChar[2] == eVar)
        {
        mData.uChar[2] = stringToChar(mVars[2]->getStringValue().c_str());
        hasVar = true;
        }
      if (mValueStates.mChar[3] == eVar)
        {
        mData.uChar[3] = stringToChar(mVars[3]->getStringValue().c_str());
        hasVar = true;
        }
      }
    }
  return hasVar;    
  }


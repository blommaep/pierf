// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "IgmpGroupRec.hpp"
#include "VarContainer.hpp"

#include <sstream>
#include <string.h>
#include <typeinfo>

IgmpGroupRec::IgmpGroupRec()
  {
  }

void IgmpGroupRec::parseAttrib(const char** attr, AutoObject* parent, bool checkMandatory, bool storeAsString) throw (Exception)
  {
  //tbd : currently dummy. This parsing is still done in the ParseConfig.cpp
  }

void IgmpGroupRec::setType(const char* type) throw (Exception)
  {
  if (type[0] == '$')
    {
    Var* var = VarContainer::getVar(type);
    if (var==NULL)
      {
      throw Exception ("Undefined variable used to assing type in <group> tag: " + string(type));
      }
    type = var->getStringValue().c_str();
    }

  if (!strcmp(type,"include"))
    {
    mType.setManualFromValue((uchar) 1);
    }
  else if (!strcmp(type,"exclude"))
    {
    mType.setManualFromValue((uchar) 2);
    }
  else if (!strcmp(type,"changeToInclude"))
    {
    mType.setManualFromValue((uchar) 3);
    }
  else if (!strcmp(type,"changeToExclude"))
    {
    mType.setManualFromValue((uchar) 4);
    }
  else if (!strcmp(type,"allowNewSources"))
    {
    mType.setManualFromValue((uchar) 5);
    }
  else if (!strcmp(type,"blockOldSources"))
    {
    mType.setManualFromValue((uchar) 6);
    }
  else
    {
    mType.setManualFromValue(type);
    }
  }

void IgmpGroupRec::setMcastAddress(const char* mcastAddr, bool storeAsString) throw (Exception)
  {
  mMcastIp.setManual(mcastAddr, storeAsString);
  }

void IgmpGroupRec::addSource(char* srcAddr, bool storeAsString) throw (Exception)
  {
  IpAddress ip;
  ip.setManual(srcAddr, storeAsString);
  mSourceList.push_back(ip);
  }

string IgmpGroupRec::getTypeString()
  {
  if (!mType.hasValue())
    {
    return "";
    }

  switch (mType.getValue())
    {
    case 1:
      return "include";
      break;
    case 2:
      return "exclude";
      break;
    case 3:
      return "changeToInclude";
      break;
    case 4:
      return "changeToExclude";
      break;
    case 5:
      return "addNewSource";
      break;
    case 6:
      return "blockOldSource";
      break;
    default:
      return mType.getString();
    }
  return ""; // should never reach this point
  }

string IgmpGroupRec::getString()
  {
  stringstream retval;
  retval << "<group type=\"";
  retval << getTypeString();
  retval << "\" to=\"" << mMcastIp.getString() << "\"";
  
  if (mSourceList.size() > 0)
    {
    retval << ">" << endl;

    vector<IpAddress>::iterator iter;
    for (iter = mSourceList.begin();iter != mSourceList.end();iter++)
      {
      retval << "  <source address=\"" << iter->getString() <<"\" />" << endl;
      }
    
    retval << "</group>" << endl;
    }
  else
    {
    retval << " />"; // close igmp
    }  
  
  retval << flush;
  return retval.str();
  }

bool IgmpGroupRec::getString(string& stringval, const char* fieldName)
  {
  if (!strcmp(fieldName, "type"))
    {
    if (mType.hasValue())
      {
      stringval = getTypeString();
      return true;
      }
    else
      {
      return false;
      }
    }
  else if (!strcmp(fieldName, "to"))
    {
    return mMcastIp.getString(stringval);
    }

  // tbd: currently can't do source records: cf. same remark in IgmpV3.cpp
  return false;
  }

ulong IgmpGroupRec::getSize()
  {
  return 8+(4*mSourceList.size());
  }

ulong IgmpGroupRec::getTailSize()
  {
  return 0;
  }

bool IgmpGroupRec::copyVar() throw (Exception)
  {
  bool copy = false;
  copy = copy || mType.copyVar();
  copy = copy || mNrSources.copyVar();
  copy = copy || mMcastIp.copyVar();

  vector<IpAddress>::iterator iter;
  for (iter = mSourceList.begin();iter != mSourceList.end();iter++)
    {
    copy = copy || iter->copyVar();
    }
  return copy;
  }

uchar* IgmpGroupRec::copyTo(uchar* toPtr)
  {
  toPtr = mType.copyTo(toPtr);
  *toPtr++ = 0x0;

  if (!mNrSources.hasValue())
    {
    mNrSources.setDefault((ushort) mSourceList.size());
    }
  toPtr = mNrSources.copyTo(toPtr);
  toPtr = mMcastIp.copyTo(toPtr);

  vector<IpAddress>::iterator iter;
  for (iter = mSourceList.begin();iter != mSourceList.end();iter++)
    {
    toPtr = iter->copyTo(toPtr);
    }

  return toPtr;
  }

uchar* IgmpGroupRec::copyTail(uchar* toPtr)
  {
  return toPtr;
  }

bool IgmpGroupRec::analyze_Head(uchar*& fromPtr, ulong& remainingSize)
  {
  if (!mType.analyze(fromPtr,remainingSize)) return false;
  if (remainingSize < 1 || *fromPtr != 0)
    {
    return false;
    }
  fromPtr++;
  remainingSize--;

  if (!mNrSources.analyze(fromPtr,remainingSize)) return false;
  if (!mMcastIp.analyze(fromPtr,remainingSize)) return false;

  for (ushort i=0; i<mNrSources.getValue(); i++)
    {
    IpAddress ip;
    if (!ip.analyze(fromPtr,remainingSize)) return false;
    mSourceList.push_back(ip);
    }

  return true;  
  }

bool IgmpGroupRec::analyze_Tail(uchar*& fromPtr, ulong& remainingSize)
  {
  return true;
  }

Element* IgmpGroupRec::analyze_GetNextElem()
  {
  return NULL;
  }


bool IgmpGroupRec::checkComplete()
  {
  return true; // nothing follows IGMP, so nothing to complete
  }

bool IgmpGroupRec::tryComplete(ElemStack& stack)
  {
  return true; // nothing follows IGMP, so nothing to complete
  }

string IgmpGroupRec::whatsMissing()
  {
  return "";
  }

IpAddress IgmpGroupRec::getDestIp()
  {
  return mMcastIp;
  }

bool IgmpGroupRec::match(Element* other)
  {
  if (typeid(*other) != typeid(IgmpGroupRec))
    {
    return false;
    }
  IgmpGroupRec* otherRec = (IgmpGroupRec*) other;

  if (!mType.match(otherRec->mType))
    {
    return false;
    }
  if (!mNrSources.match(otherRec->mNrSources))
    {
    return false;
    }
  if (!mMcastIp.match(otherRec->mMcastIp))
    {
    return false;
    }

  // there are no auto or defaults for the sourcelist, so they must match entirely and consequently a check on the size is a good start
  if (mSourceList.size() != otherRec->mSourceList.size())
    {
    return false;
    }
  vector<IpAddress>::iterator iterSelf;
  vector<IpAddress>::iterator iterOther;
  iterOther = otherRec->mSourceList.begin();
  // we already checked for identical size, so one end condition is enough
  for (iterSelf = mSourceList.begin();iterSelf != mSourceList.end();iterSelf++, iterOther++)
    {
    if (!iterSelf->match(*iterOther))
      {
      return false;
      }
    }
  
  return true;  
  }

Element* IgmpGroupRec::getNewBlank()
  {
  IgmpGroupRec* igmpGroupRec = new IgmpGroupRec();
  return (Element*) igmpGroupRec;
  }

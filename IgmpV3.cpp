// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "IgmpV3.hpp"
#include "VarContainer.hpp"

#include <sstream>

IgmpV3::IgmpV3()
  {
  }
    
IgmpV3::~IgmpV3()
  {
  vector<IgmpGroupRec*>::iterator iter;
  for (iter = mGroupList.begin();iter != mGroupList.end();iter++)
    {
    delete *iter;
    }
  }

void IgmpV3::parseAttrib(const char** attr, AutoObject* parent, bool checkMandatory) throw (Exception)
  { 
  char* autoStr=NULL;
  int i=0;
  while (attr[i] != NULL)
    {
    if (!strcmp(attr[i],"version"))
      {
      i++;
      i++; // script the tag and the value
      }
    else if (!strcmp(attr[i],"type"))
      {
      i++;
      setType(attr[i++]);
      }
    else if (!strcmp(attr[i],"to"))
      {
      i++;
      setMcastIp(attr[i++]);
      }
    else if (!strcmp(attr[i],"responsetime"))
      {
      i++;
      setResponseTime(attr[i++]);
      }
    else if (!strcmp(attr[i],"checksum"))
      {
      i++;
      setChecksum(attr[i++]);
      }
    else if (!strcmp(attr[i],"sflag"))
      {
      i++;
      setSFlag(attr[i++]);
      }
    else if (!strcmp(attr[i],"qrv"))
      {
      i++;
      setQrv(attr[i++]);
      }
    else if (!strcmp(attr[i],"qqic"))
      {
      i++;
      setQqic(attr[i++]);
      }
    else if (!strcmp(attr[i],"nrrecords"))
      {
      i++;
      mNrRecords.setManual(attr[i++]);
      }
    else if (!strcmp(attr[i],"auto"))
      {
      i++;
      autoStr = (char*) attr[i++];
      }
    else
      {
      throw Exception("Unexepected attribute: " + string(attr[i]) + " in <igmp> tag.");
      i++;
      }
    }

  setOrEnheritAuto(autoStr,parent);


  if (!isAuto() && checkMandatory)
    {
    if (!mType.hasValue())
      {
      throw Exception("Missing mandatory attribute to <igmp> tag: type");
      }
    if (!mMcastIp.hasValue() && mType.getValue() == 0x11) // query
      {
      throw Exception("Missing mandatory attribute to <igmp> tag (version 3, type query): to (destination multicast ip)");
      }
    }
  }

void IgmpV3::setType(const char* type) throw (Exception)
  {
  if (type[0] == '$')
    {
    Var* var = VarContainer::getVar(type);
    if (var==NULL)
      {
      throw Exception ("Undefined variable used to assign type in <igmp> tag: " + string(type));
      }
    type = var->getStringValue().c_str();
    }

  if (!strcmp(type,"query"))
    {
    mType.setManualFromValue((uchar) 0x11);
    mResponseTime.setDefault((ushort) 100);
    mSFlag.setDefault((uchar) 0x0);
    mSFlag.setOffset(3);
    mQrv.setDefault((uchar) 0x2); // default robustness
    mQqic.setDefault((uchar) 125); // default: default query time
    }
  else if (!strcmp(type,"report"))
    {
    mType.setManualFromValue((uchar) 0x22);
    }
  else
    {
    mType.setManual(type);
    }
  }

void IgmpV3::setResponseTime(const char* responseTime) throw (Exception)
  {
  mResponseTime.setManual(responseTime);
  }

void IgmpV3::setChecksum(const char* checksum) throw (Exception)
  {
  mChecksum.setManual(checksum);
  }

void IgmpV3::setSFlag(const char* sflag) throw (Exception)
  {
  if (mType.getValue() == 0x11)
    {
    mSFlag.setManual(sflag);
    }
  else
    {
    throw Exception("Setting sflag value for an igmp V3 message type that is not a query: " + mType.getString());
    }
  }

void IgmpV3::setQrv(const char* qrv) throw (Exception)
  {
  if (mType.getValue() == 0x11)
    {
    mQrv.setManual(qrv);
    }
  else
    {
    throw Exception("Setting qrv value for an igmp V3 message type that is not a query: " + mType.getString());
    }
  }

void IgmpV3::setQqic(const char* qqic) throw (Exception)
  {
  if (mType.getValue() == 0x11)
    {
    mQqic.setManual(qqic);
    }
  else
    {
    throw Exception("Setting qqic value for an igmp V3 message type that is not a query: " + mType.getString());
    }
  }

void IgmpV3::setMcastIp(const char* mcastIp) throw (Exception)
  {
  if (mType.getValue() == 0x11)
    {
    mMcastIp.setManual(mcastIp);
    }
  else
    {
    throw Exception("Setting to value (multicast ip address) for an igmp V3 message type that is not a query: " + mType.getString());
    }
  }

void IgmpV3::addSource(const char* srcIp) throw (Exception)
  {
  if (mType.getValue() != 0x11)
    {
    throw Exception("Adding a source for an igmp V3 message type that is not a query: " + mType.getString());
    }
  IpAddress ip;
  ip.setManual(srcIp);
  mSourceList.push_back(ip);
  }

IgmpGroupRec* IgmpV3::addGroupRecord(const char* type, const char* mcastIp) throw (Exception)
  {
  if (mType.getValue() != 0x22)
    {
    throw Exception("Adding a group record for an igmp V3 message type that is not a report: " + mType.getString());
    }
  IgmpGroupRec* record = new IgmpGroupRec();
  record->setType(type);
  record->setMcastAddress(mcastIp);
  mGroupList.push_back(record);
  return record;
  }

string IgmpV3::getTypeString()
  {
  if (!mType.hasValue())
    {
    return "";
    }

  string retval;
  switch (mType.getValue())
    {
    case 0x11:
      retval = "query";
      break;
    case 0x22:
      retval = "report";
      break;
    default:
      retval = mType.getString();
    }
  return retval;
  }

string IgmpV3::getString()
  {
  stringstream retval;
  retval << "<igmp version=\"3\" type=\"";
  retval << getTypeString();
  retval << "\"";
  if (mMcastIp.isPrintable())
    {
    retval << " to=\"" << mMcastIp.getString();
    }

  // the Query fields
  if (mResponseTime.isPrintable())
    {
    retval << "\" responsetime=\"" << mResponseTime.getString();
    }
  if (mSFlag.isPrintable())
    {
    retval << "\" sflag=\"" << mSFlag.getString();
    }
  if (mQrv.isPrintable())
    {
    retval << "\" qrv=\"" << mQrv.getString();
    }
  if (mQqic.isPrintable())
    {
    retval << "\" qqic=\"" << mQqic.getString();
    }
  if (mNrRecords.isPrintable())
    {
    retval << "\" nrrecords=\"" << mNrRecords.getString();
    }

  retval << "\"";
  
  if (mType.getValue() == 0x11 && mSourceList.size() > 0) // query
    {
    retval << ">" << endl;
    vector<IpAddress>::iterator iter;
    for (iter = mSourceList.begin();iter != mSourceList.end();iter++)
      {
      retval << "  <source address=\"" << iter->getString() <<"\" />" << endl;
      }
    
    retval << "</igmp>" << endl;
    }
  else if (mType.getValue() == 0x22) // report
    {
    retval << ">" << endl;
    vector<IgmpGroupRec*>::iterator iter;
    for (iter = mGroupList.begin();iter != mGroupList.end();iter++)
      {
      IgmpGroupRec* rec = *iter;
      retval << "  " << rec->getString();
      }
    retval << "</igmp>" << endl;
    }
  else
    {
    retval << "/>";
    }

  // tbd: currently not possible to query for igmp sources because for that to work, each tag must match an "Element" object, so we should create a class IgmpV3SourceRec class. In fact, the coupling of both source and group records with the igmpv3 class should be loosened and the nrRecords should be filled via the autocompletion technique.

  retval << flush;
  return retval.str();
  }

bool IgmpV3::getString(string& stringval, const char* fieldName)
  {
  if (!strcmp(fieldName, "version"))
    {
    stringval = "2";
    return true;
    }
  else if (!strcmp(fieldName, "type"))
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
  else if (!strcmp(fieldName, "responsetime"))
    {
    return mResponseTime.getString(stringval);
    }
  else if (!strcmp(fieldName, "sflag"))
    {
    return mSFlag.getString(stringval);
    }
  else if (!strcmp(fieldName, "qrv"))
    {
    return mQrv.getString(stringval);
    }
  else if (!strcmp(fieldName, "qqic"))
    {
    return mQqic.getString(stringval);
    }
  else if (!strcmp(fieldName, "nrrecords"))
    {
    return mNrRecords.getString(stringval);
    }

  return false;
  }

ulong IgmpV3::getSize()
  {
  if (mType.getValue() == 0x11) // query
    {
    return 12 + 4*mSourceList.size();
    }
  else if (mType.getValue() == 0x22) // report
    {
    ulong size = 8;
    vector<IgmpGroupRec*>::iterator iter;
    for (iter = mGroupList.begin();iter != mGroupList.end();iter++)
      {
      IgmpGroupRec* rec = *iter;
      size += rec->getSize();
      }
    return size;
    }
  return 8;
  }

ulong IgmpV3::getTailSize()
  {
  return 0;
  }

bool IgmpV3::copyVar() throw (Exception)
  {
  bool copy = false;
  bool res;
  res = mType.copyVar();
  copy = copy || res;
  res = mResponseTime.copyVar();
  copy = copy || res;
  res = mChecksum.copyVar();
  copy = copy || res;
  res = mSFlag.copyVar();
  copy = copy || res;
  res = mQrv.copyVar();
  copy = copy || res;
  res = mQqic.copyVar();
  copy = copy || res;
  res = mMcastIp.copyVar();
  copy = copy || res;
  res = mNrRecords.copyVar(); 
  copy = copy || res;

    {
    vector<IpAddress>::iterator iter;
    for (iter = mSourceList.begin();iter != mSourceList.end();iter++)
      {
      res = iter->copyVar();
      copy = copy || res;
      }
    }

    {
    vector<IgmpGroupRec*>::iterator iter;
    for (iter = mGroupList.begin();iter != mGroupList.end();iter++)
      {
      IgmpGroupRec* rec = *iter;
      res = rec->copyVar();
      copy = copy || res;
      }
    }

  return copy;
  }

uchar* IgmpV3::copyTo(uchar* toPtr)
  {
  uchar* ckStart = toPtr; // start of checksum
  uchar* ckPos = NULL;
  
  toPtr = mType.copyTo(toPtr);
  if (mType.getValue() == 0x11)
    {
    toPtr = mResponseTime.copyTo(toPtr);
    ckPos = toPtr; // checksum position
    *toPtr++ = 0x00; // checksum
    *toPtr++ = 0x00; // checksum
    toPtr = mMcastIp.copyTo(toPtr);
    *toPtr = 0x0; // Clear the byte, cleaning the reserved (unused) bits
    mSFlag.copyTo(toPtr);
    toPtr = mQrv.copyTo(toPtr);
    toPtr = mQqic.copyTo(toPtr);
    if (!mNrRecords.hasValue())
      {
      mNrRecords.setDefault((ushort) mSourceList.size());
      }
    toPtr = mNrRecords.copyTo(toPtr);

    vector<IpAddress>::iterator iter;
    for (iter = mSourceList.begin();iter != mSourceList.end();iter++)
      {
      toPtr = iter->copyTo(toPtr);
      }
    }
  else
    {
    *toPtr++ = 0x0;
    ckPos = toPtr; // checksum position
    *toPtr++ = 0x00; // checksum
    *toPtr++ = 0x00; // checksum
    *toPtr++ = 0x00; // reserved
    *toPtr++ = 0x00; // reserved
    if (!mNrRecords.hasValue())
      {
      mNrRecords.setDefault((ushort) mGroupList.size());
      }
    toPtr = mNrRecords.copyTo(toPtr);
    vector<IgmpGroupRec*>::iterator iter;
    for (iter = mGroupList.begin();iter != mGroupList.end();iter++)
      {
      IgmpGroupRec* rec = *iter;
      toPtr = rec->copyTo(toPtr);
      }
    }

  // calculate checksum
  mChecksum.calculate(ckStart, toPtr);
  mChecksum.copyTo(ckPos);
  return toPtr;
  }

uchar* IgmpV3::copyTail(uchar* toPtr)
  {
  return toPtr;
  }

bool IgmpV3::analyze_Head(uchar*& fromPtr, ulong& remainingSize)
  {
  if (remainingSize <1)
    {
    return false;
    }

  if (*fromPtr == 0x11) // query
    {
    if (!mType.analyze(fromPtr,remainingSize)) return false;
    if (!mResponseTime.analyze(fromPtr,remainingSize)) return false;
    if (!mChecksum.analyze(fromPtr,remainingSize)) return false;
    if (!mMcastIp.analyze(fromPtr,remainingSize)) return false;
    if (!mSFlag.analyze(fromPtr,remainingSize)) return false;
    if (!mQrv.analyze(fromPtr,remainingSize)) return false;
    fromPtr++; // mSFlag and mQrv are partial fields, so now progressing
    remainingSize--;
    if (!mQqic.analyze(fromPtr,remainingSize)) return false;
      
    if (!mNrRecords.analyze(fromPtr,remainingSize)) return false;

    for (ushort i=0; i<mNrRecords.getValue();i++)
      {
      IpAddress ip;
      if (!ip.analyze(fromPtr,remainingSize)) return false;
      mSourceList.push_back(ip);
      }
    }
  else if (*fromPtr == 0x22) // report (== also leave in igmp v3)
    {
    if (!mType.analyze(fromPtr,remainingSize)) return false;
    if (remainingSize <1)
      {
      return false;
      }
    fromPtr ++;
    remainingSize--;
    if (!mChecksum.analyze(fromPtr,remainingSize)) return false;
    if (remainingSize <2)
      {
      return false;
      }
    fromPtr += 2;
    remainingSize -= 2;

    if (!mNrRecords.analyze(fromPtr,remainingSize)) return false;
    for (ushort i=0; i<mNrRecords.getValue();i++)
      {
      IgmpGroupRec* rec = new IgmpGroupRec;
      if (!rec->analyze_Head(fromPtr,remainingSize)) return false;
      mGroupList.push_back(rec);
      }    
    }
  else
    {
    return false; // unsupported type
    }
  
  return true;
  }

Element* IgmpV3::analyze_GetNextElem()
  {
  return NULL;
  }


bool IgmpV3::analyze_Tail(uchar*& fromPtr, ulong& remainingSize)
  {
  return true;
  }

bool IgmpV3::checkComplete()
  {
  return true; // nothing follows IGMP, so nothing to complete
  }

bool IgmpV3::tryComplete(ElemStack& stack)
  {
  return checkComplete(); // nothing follows IGMP, so nothing to complete
  }

string IgmpV3::whatsMissing()
  {
  return "";
  }

IpAddress IgmpV3::getDestIp()
  {
  if (mType.getValue() == 0x11)
    {
    if (mMcastIp.getAddress() == 0) //general query
      {
      IpAddress temp;
      temp.setAuto("224.0.0.01");
      return temp;
      }
    }
  return mMcastIp;
  }

uchar IgmpV3::getType()
  {
  if (mType.hasValue())
    {
    return mType.getValue();
    }
  return 0;
  }

bool IgmpV3::match(Element* other)
  {
  if (typeid(*other) != typeid(IgmpV3))
    {
    return false;
    }
  IgmpV3* otherIgmp = (IgmpV3*) other;

    Bitfield8 mType;
    Bitfield8 mResponseTime;
    ChecksumIp mChecksum;

    // for the query only
//    Bitfield16 mNrRecords; // nrSources or nrGroupRecords
    vector<IpAddress> mSourceList; // for query
    vector<IgmpGroupRec*> mGroupList; // for report

  if (!mType.match(otherIgmp->mType))
    {
    return false;
    }
  if (!mResponseTime.match(otherIgmp->mResponseTime))
    {
    return false;
    }
  if (!mChecksum.match(otherIgmp->mChecksum))
    {
    return false;
    }
  if (!mNrRecords.match(otherIgmp->mNrRecords))
    {
    return false;
    }
  if (mType.getValue() == 0x11) // query
    {
    if (!mSFlag.match(otherIgmp->mSFlag))
      {
      return false;
      }
    if (!mQrv.match(otherIgmp->mQrv))
      {
      return false;
      }
    if (!mQqic.match(otherIgmp->mQqic))
      {
      return false;
      }
    if (!mMcastIp.match(otherIgmp->mMcastIp))
      {
      return false;
      }

    if (mSourceList.size() != otherIgmp->mSourceList.size())
      {
      return false;
      }

    vector<IpAddress>::iterator iterSelf;
    vector<IpAddress>::iterator iterOther;
    iterOther = otherIgmp->mSourceList.begin();
    for (iterSelf = mSourceList.begin();iterSelf != mSourceList.end();iterSelf++, iterOther++)
      {
      if (!iterSelf->match(*iterOther))
        {
        return false;
        }
      }
    }
  else // report
    {
    if (mGroupList.size() != otherIgmp->mGroupList.size())
      {
      return false;
      }

    vector<IgmpGroupRec*>::iterator iterSelf;
    vector<IgmpGroupRec*>::iterator iterOther;
    iterOther = otherIgmp->mGroupList.begin();
    for (iterSelf = mGroupList.begin();iterSelf != mGroupList.end();iterSelf++, iterOther++)
      {
      IgmpGroupRec* recSelf = *iterSelf; // need double dereference
      IgmpGroupRec* recOther = *iterOther;
      if (!recSelf->match(recOther))
        {
        return false;
        }
      }
    }

  return true;  
  }


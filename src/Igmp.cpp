// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "Igmp.hpp"
#include "IgmpV2.hpp"
#include "IgmpV3.hpp"

#include <sstream>
#include <string.h>

Igmp::Igmp()
  {
  mVersion.setDefault((uchar) 2);
  }

void Igmp::parseAttrib(const char** attr, AutoObject* parent, bool checkMandatory, bool storeAsString) throw (Exception)
  {
  int i=0;
  char* strVersion=NULL;

  while (attr[i] != NULL)
    {
    if (!strcmp(attr[i],"version"))
      {
      i++;
      strVersion = (char*) attr[i++];
      }
    else
      {
      i++;
      }
    }

  if (strVersion == NULL) // igmp version is always mandatory, even for packet capture match
    {
    throw Exception("Missing mandatory attribute to <igmp> tag: version");
    }

  mVersion.setManual(strVersion, false); // igmp version must be set, it is necessary for distinction & further parsing
  if (mVersion.isVar())
    {
    throw Exception("Igmp version cannot be defined by a non const variable");
    }

  uchar version = mVersion.getValue();
  if (version != 2 && version != 3)
    {
    throw Exception("Igmp version not supported. Must be 2 or 3");
    }
  } 

string Igmp::getString()
  {
  return "";
  }

bool Igmp::getString(string& stringval, const char* fieldName)
  {
  return false; // Has no fields, shouldn't be asked for: -> next elem
  }


ulong32 Igmp::getSize()
  {
  return 0;
  }

ulong32 Igmp::getTailSize()
  {
  return 0;
  }

bool Igmp::copyVar() throw (Exception)
  {
  return false;
  }

uchar* Igmp::copyTo(uchar* toPtr)
  {
  return toPtr;
  }

uchar* Igmp::copyTail(uchar* toPtr)
  {
  return toPtr;
  }

bool Igmp::analyze_Head(uchar*& fromPtr, ulong32& remainingSize)
  {
  if (remainingSize < 1)
    {
    return false;
    }

  switch (*fromPtr)
    {
    case 0x16: // Igmp V2 join/report
    case 0x17: // Igmp V2 leave
      mVersion.setManualFromValue((uchar) 2);
      return true;
      break;
    case 0x22: // Igmp V3 report
      mVersion.setManualFromValue((uchar) 3);
      return true;
      break;
    case 0x11: // Query
      if (remainingSize <= 12)
        {
        mVersion.setManualFromValue((uchar) 2); // Further checks are up to IgmpV2 class. It can't be Igmp v3 query
        return true;
        }
      else
        {
        ushort igmpSize = ntohs( *((ushort*) (fromPtr + 10)) ); // 10 bytes after start of igmp v3 query header is the number of source addresses
        igmpSize *= 4; // 4 bytes per address
        igmpSize += 12; // the general header part
        if (remainingSize < igmpSize)
          {
          mVersion.setManualFromValue((uchar) 2);
          return true;
          }
        mVersion.setManualFromValue((uchar) 3);
        return true;
        }
      break;
    default:
      return false;
    }
  return false;

  }

bool Igmp::analyze_Tail(uchar*& fromPtr, ulong32& remainingSize)
  {
  return true;
  }

Element* Igmp::analyze_GetNextElem()
  {
  if (mVersion.getValue() == 2)
    {
    return new IgmpV2();
    }
  else
    {
    return new IgmpV3();
    }
  }

bool Igmp::checkComplete()
  {
  return true; // nothing follows IGMP, so nothing to complete
  }

bool Igmp::tryComplete(ElemStack& stack)
  {
  return true; // nothing follows IGMP, so nothing to complete
  }

string Igmp::whatsMissing()
  {
  return "";
  }

uchar Igmp::getVersion()
  {
  return mVersion.getValue();
  }

bool Igmp::match(Element* other)
  {
  return true; // Even V2 or V3 distinction is left up to the next layer
  }

Element* Igmp::getNewBlank()
  {
  Igmp* igmp =  new Igmp();
  return (Element*) igmp;
  }

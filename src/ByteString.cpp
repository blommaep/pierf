// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "ByteString.hpp"

#include <iostream> // for cout and cin
#include <iomanip>
#include <fstream>
#include <sstream>

void ByteString::stringToVal(const char* inString) throw (Exception)
  {
  char* curPtr = (char*) inString;
  uchar temp=0;
  int nrCharsRead=0;
  uchar separator=0; // 0x0 is unknown, 0x1 is known not to be using a separator, any other value is a supported separator (':')
  bool hexRead = true;

  if (!(*curPtr))
    {
    return;
    // empty string: avoid a single 0 byte being created
    }

  while (*curPtr)
    {
    if (nrCharsRead >= 2 || (*curPtr == ':') || (*curPtr == ' ')) // 2 chars is the max for one byte, really
      {
      if (*curPtr == ':' || *curPtr == ' ')
        {
        if (separator == 0)
          {
          separator = *curPtr;
          }
        else if (separator == 1)
          {
          if (mParseStrict)
            {
            throw Exception ("Reading a separator (:) while started reading a hex string without separators (for >2 characters)");
            }
          }
        else if (*curPtr != separator) // had read another separator 
          {
          if (mParseStrict)
            {
            throw Exception ("Mix of two different byte separators in hex string not supported: '" + string(curPtr,1) + "' and '" + string((char*)&separator,1) + "'");
            }
          }
        }
      else
        {
        if (separator > 1)
          {
          if (mParseStrict)
            {
            throw Exception ("Expecting a separator (:) when reading another character after: " + string(inString,(curPtr-inString)));
            }
          }
        separator = 1; // don't care if it was allready set, can just as well set it again
        }

      // after all these checks, now simply store the byte
      if (nrCharsRead <= 0)
        {
        if (mParseStrict)
          {
          throw Exception ("Found a separator (:) not preceeded by at least one hex character"); 
          }
        }
      else
        {
        push_back(temp);
        }
      temp=0;
      nrCharsRead=0;
      }
    
    if (*curPtr == ':' || *curPtr == ' ')
      {
      // Separator, may be skipped now
      }
    else
      {
      hexRead = true; // Always assume reading hex untill proven otherwise
      if (*curPtr < '0') // character before '0'
        {
        if (mParseStrict)
          {
          throw Exception("Non hex character in hex string: '" + string(curPtr,1) + "'"); 
          }
        hexRead = false;
        }
      uchar nible = *curPtr-'0'; 
      if (nible > 9 && hexRead) // subtract the distance between the number characters and the letters
        {
        if (nible < ('A' - '0' - 0xa)) // character between '9' and 'A'
          {
          if (mParseStrict)
            {
            throw Exception("Non hex character in hex string: " + string(curPtr,1));
            }
          hexRead = false;
          }
        else
          {
          nible = nible - ('A' - '0' - 0xa);
          }
        }

      if (nible>15 && hexRead) // subtract the distance between upper and lower case characters
        {
        if (nible > 15+('a'-'A')) // character between 'F' and 'a'
          {
          if (mParseStrict)
            {
            throw Exception("Non hex character in hex string: " + string(curPtr,1));
            }
          hexRead = false;
          }
        nible = nible - ('a' - 'A');
        if (nible > 15 && hexRead) // character after 'f'
          {
          if (mParseStrict)
            {
            throw Exception("Non hex character in hex string: " + string(curPtr,1));
            }
          hexRead = false;
          }
        }
      if (hexRead)
        {
        temp = temp*16+nible;
        nrCharsRead++;
        }
      }
    curPtr++;
    }
  if (nrCharsRead > 0)
    {
    push_back(temp);
    }
  }
  
ByteString::ByteString()
  :mParseStrict(true), mInputChar(false)
  {
  }

ByteString::ByteString(const char* inString) throw (Exception)
  :mParseStrict(true), mInputChar(false)
  {
  stringToVal(inString);
  }

ByteString::ByteString(const string& inString) throw (Exception)
  :mParseStrict(true), mInputChar(false)
  {
  stringToVal(inString.c_str());
  }

void ByteString::addString(const char* inString) throw (Exception)
  {
  if (mInputChar)
    {
    char* temp = (char*) inString;
    while (*temp != 0x0)
      {
      push_back(*temp);
      temp++;
      }
    }
  else
    {
    stringToVal(inString);
    }
  }

void ByteString::addString(const string& inString) throw (Exception)
  {
  addString(inString.c_str());
  }

void ByteString::resetString()
  {
  clear();
  }

string ByteString::getString() const
  {
  return getString(mInputChar);
  }

string ByteString::getString(bool asChar) const
  {
  stringstream retval;

  if (asChar)
    {
    vector<uchar>::const_iterator iter;
    for (iter=begin();iter!=end();iter++)
      {
      retval << *iter;
      }
    }  
  else
    {
    retval.setf(ios::uppercase); // print hex in uppercase
    retval.setf(ios::hex,ios::basefield); 
    retval.fill('0');

    vector<uchar>::const_iterator iter;
    for (iter=begin();iter!=end();) // !! LOOP INCREMENT INSIDE
      {
      retval << setw(2) << (int) (*iter);

      iter++; // !! loop increment 

      if (iter != end())
        {
        retval << ':';
        }
      }
    }

  retval << flush;
  return retval.str();
  }

void ByteString::addBytes(const uchar* inBytes, ulong32 nrBytes)
  {
  reserve(size() + nrBytes); // avoids repetitive re-allocations of vector during insertion
  for (ulong32 i=0; i<nrBytes;i++)
    {
    push_back(inBytes[i]);
    }  
  }

uchar* ByteString::copyTo(uchar* toPtr)
  {
  vector<uchar>::iterator iter;
  for (iter=begin();iter!=end();iter++)
    {
    *toPtr++ = *iter; // copy byte by byte
    }
  return toPtr;  
  }

uchar* ByteString::copyTo(uchar* toPtr, ulong32 maxSize)
  {
  ulong32 i = 0;
  vector<uchar>::iterator iter;
  for (iter=begin();iter!=end() && i<maxSize;iter++)
    {
    *toPtr++ = *iter; // copy byte by byte
    i++;
    }
  return toPtr;  
  }

void ByteString::setStrict(bool strict)
  {
  mParseStrict = strict;
  }

void ByteString::setInputChar(bool asChar)
  {
  mInputChar = asChar;
  }

bool ByteString::hasInputChar() const
  {
  return mInputChar;
  }


bool ByteString::match(ByteString& other)
  {
  if (size() != other.size())
    {
    return false;
    }

  vector<uchar>::iterator iterSelf;
  vector<uchar>::iterator iterOther;
  iterOther = other.begin();
  for (iterSelf=begin();iterSelf!=end();iterSelf++,iterOther++) // only 1 end condition because we checked for identical size before
    {
    if (*iterSelf != *iterOther)
      {
      return false; // copy byte by byte
      }
    }
  return true;
  }


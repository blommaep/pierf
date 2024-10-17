// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "Ipv6Address.hpp"

#include <iostream> // for cout and cin
#include <iomanip>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include "ByteString.hpp"
#include <string.h>
//#include <asm/bytorder.h>

void Ipv6Address::stringToVal(const char* inString) throw (Exception)
  {
  unsigned int i=0,j=0,k=0;
  unsigned int nrColons = 0;
  char word[5];
  char zeros[5];

  ByteString convert[2];

  if (!strcmp(inString,"::1") || !strcmp(inString,"::")) //loopback
    {
    for (i=0;i<15;i++)
      {
      mBytes[i] = 0;
      }
    if (!strcmp(inString,"::"))
      {
      mBytes[i] = 0;
      }
    else
      {
      mBytes[i] = 1;
      }
    return;
    }

  i=0;
  while (i<37)
    {
    if (inString[i] == ':' || inString[i] == 0)
      {
      if (inString[i] == ':')
        {
        nrColons++;
        if (k==0)
          {
          j++; //double collon encountered
          if (j>1)
            {
            throw Exception("Invalid IPv6 address (too many double colons (::)): " + string(inString));
            }
          }
        }
      if (k>0)
        {
        try
          {
          unsigned int l,m;
          for (l=0;l<(4-k);l++) // add leading zeros
            {
            zeros[l] = '0';
            }
          for (m=0;m<k;m++)
            {
            zeros[l+m] = word[m];
            }
          zeros[4] = 0;
          convert[j].addString(zeros);
          }
        catch (Exception e)
          {
          stringstream msg;
          msg << "Invalid IPv6 address: " << inString << " :" << endl << "  " << e.what() << endl;
          throw (Exception(msg.str()));
          }
        }
      k=0;
      if (inString[i] == 0)
        {
        i=38;
        }      
      }
    else
      {
      word[k] = inString[i];
      k++;
      if (k>4)
        {
        throw Exception("Invalid IPv6 address (missing colon): " + string(inString));
        }
      }
    i++;
    } 

  if (nrColons > 7)
    {
    throw Exception ("Invalid IPv6 address (too many colons): " + string(inString));
    }

  convert[0].copyTo(mBytes);

  if (j>0) // there was a :: -> fill with sufficient zeros
    {
    unsigned int convert0Size;
    convert0Size = convert[0].size();
    unsigned int l;
    for (l=0;l<(8-nrColons);l++)
      {
      unsigned int m = 2*l + convert0Size;
      mBytes[m++] = 0;
      mBytes[m] = 0;
      }
    convert0Size = convert0Size + ((8 - nrColons) * 2);
    convert[1].copyTo(mBytes+convert0Size);
    }
  else
    {
    if (nrColons < 7)
      {
      throw Exception ("Invalid IPv6 address (too few colons): " + string(inString));
      }
    }

  }
  
Ipv6Address::Ipv6Address()
  {
  for (int i=0;i<16;i++)
    {
    mBytes[i] = 0;
    }
  }

void Ipv6Address::setDefault(const char* inString) throw (Exception)
  {
  stringToVal(inString);
  wasManuallySet();
  }

void Ipv6Address::setManualFromValue(const char* inString) throw (Exception)
  {
  stringToVal(inString);
  wasManuallySet();
  }

void Ipv6Address::setAuto(const char* inString) throw (Exception)
  {
  stringToVal(inString);
  wasManuallySet();
  }

string Ipv6Address::getStringFromBinary() const
  {
  stringstream retval; //creates an ostringstream object
  retval.setf(ios::hex,ios::basefield); // print hex in uppercase
  retval.fill('0');
  
  int nrDoubles = 0;
  for (int ii=0;ii<16;ii++)
    {
    if (mBytes[ii] != 0)
      {
      retval << (unsigned int) mBytes[ii++];
      retval << setw(2) << (unsigned int) mBytes[ii]; // leading zeros necessary
      }
    else 
      {
      ii++; // do nothing with first byte
      if (mBytes[ii] == 0)
        {
        if (nrDoubles == 0)
          {
          retval << "::"; // second collon comes later
          nrDoubles++;
          while (mBytes[ii+1] == 0 && mBytes[ii+2] == 0)
            {
            ii++;
            ii++;
            }
          }
        else
          {
          retval << "0:";
          }
        }
      else
        {
        retval << (unsigned int) mBytes[ii++];
        if (ii<15)
          {
          retval << ":"; 
          }
        }
      }
    }

  retval << flush;
  return(retval.str()); //returns the string form of the stringstream object
  }

bool Ipv6Address::getStringFromBinary(string& stringval) const
  {
  if (hasValue())
    {
    stringval = getStringFromBinary();
    return true;
    }
  return false;
  }

uchar* Ipv6Address::copyTo(uchar* toPtr)
  {
  int i;
  for (i=0;i<16;i++)
    {
    *toPtr++ = mBytes[i];    
    }
  return toPtr;
  }

bool Ipv6Address::analyze(uchar*& fromPtr, ulong32& remainingSize)
  {
  if (remainingSize >= 16)
    {
    int i;
    for (i=0;i<16;i++)
      {
      mBytes[i] = *fromPtr++;
      }
    wasCaptured();
    remainingSize -= 16;
    return true;
    }
  return false;
  }

bool Ipv6Address::match(Ipv6Address& other)
  {
  if (isComparable() && other.hasValue())
    {
    if (isString() || other.isString())
      {
      return matchByString(other);
      }

    int i;
    for (i=0;i<16;i++)
      {
      if (mBytes[i] != other.mBytes[i])
        {
        return false;
        }
      }
    }
  //else
  return true;
  }

void Ipv6Address::addToSum(ChecksumIp& cksum)
  {
  cksum.addToSum(mBytes,mBytes+16);
  }


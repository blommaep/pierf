// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "generics.hpp"
#include <sstream>
#include <string.h>
#include <stdlib.h>

void setChecksum(uchar* ckLoc, uchar* start, uchar* stop)
  {
  // Checksum is 1's complement. 
  // The advantage: little/big endian does not matter.
  // The algorithm: add as usual, but if result is e.g. 12345, add the 1 to the 2345.
  //                then take FFFF-x and that's it.

  unsigned long checksum;
  int i;

  unsigned short *packetBy16;
  packetBy16=(unsigned short*) start;
  checksum = 0;

  for (i=0;i<((stop-start)/2);i++)
    {
    checksum = checksum + packetBy16[i];
    }

  while (checksum >= 0x10000)
    {
    checksum = (checksum & 0xFFFF) + (checksum / 0x10000);
    }

  checksum = 0xFFFF - checksum;
  *((ushort*) ckLoc) = (unsigned short) checksum;

  //  ckLoc[0] = (char) (checksum/0x100);//checksum;
  //  ckLoc[1] = (char) checksum;

  }

std::string intToString(int i)
  {
  std::stringstream retval; //creates an ostringstream object
  retval << i << std::flush;
  return(retval.str()); //returns the string form of the stringstream object
  }

uchar charToNible(char inChar) throw (Exception)
  {
  uchar nible = ((uchar) inChar)-'0';
  if (nible > 9) // subtract the distance between the number characters and the letters
    {
    if (nible < ('A' - '0' - 0xa)) // character between '9' and 'A'
      {
      throw Exception("Non hex character in hex string: " + string(&inChar,1));
      }
    else
      {
      nible = nible - ('A' - '0' - 0xa);
      }
    }

  if (nible>15) // subtract the distance between upper and lower case characters
    {
    if (nible < 15+('a'-'A')) // character between 'F' and 'a'
      {
      throw Exception("Non hex character in hex string: " + string(&inChar,1));
      }
    nible = nible - ('a' - 'A');
    if (nible > 15) // character after 'f'
      {
      throw Exception("Non hex character in hex string: " + string(&inChar,1));
      }
    }
  return nible;
  }

ulong textToLong(const char* inString) throw (Exception)
  {
  if (!strcmp(inString,"0") || !strcmp(inString,"0x0"))
    {
    return 0;
    }

  if (!strncmp(inString,"0x",2)) // hex
    {
    ulong temp = 0;
    int i;
    for (i=2;(i<10 && inString[i] != 0);i++) // first 2 characters are 0x, so read char 2 till 10
      {
      temp = temp * 16 + charToNible(inString[i]);
      }
    return temp;
    }
  else
    {
    unsigned int temp = atoi(inString);
    if (temp == 0)
      {
      throw Exception("Failed to decode as integer value: " + string(inString));
      }
    return (ulong) temp;
    }

  return 0; // pro forma
  }

ushort atos(char* instr) throw (Exception)
  {
  int temp = atoi(instr);
  if (temp < 0 || temp > 0xFFFF)
    {
    throw Exception("Invalid input string for what should be a 2 bytes integer value");
    }
  return (ushort) temp;
  }

ushort atob(char* instr) throw (Exception)
  {
  int temp = atoi(instr);
  if (temp < 0 || temp > 0xFF)
    {
    throw Exception("Invalid input string for what should be a 1 byte integer value");
    }
  return (uchar) temp;
  }

// Use with care: caller must make sure to provide a valid char buffer for two chars
void char2HexString(char* toString, uchar value)
  {
  char upper = value/16;
  if (upper < 10)
    {
    *toString++ = upper + '0';
    }
  else
    {
    *toString++ = upper + 'a' - 10;
    }
  char lower = value - (upper * 16);
  if (lower < 10)
    {
    *toString++ = lower + '0';
    }
  else
    {
    *toString++ = lower + 'a' - 10;
    }
  }

std::string longToHexString(ulong i)
  {
  ulong j = htonl(i);
  uchar* bytes = (uchar*) &j;
  char retval[12];
  char* curPos = retval;
  int k;
  for (k=0; k<4 && bytes[k] == 0;k++)
    { }
  if (k==4)
    {
    return "0";
    }
  for (;k<4;k++)
    {
    char2HexString(curPos,bytes[k]);
    curPos+=2;
    *curPos++ = ':';
    }
  curPos--;
  *curPos = 0; // end of the string

  return string(retval);
  }


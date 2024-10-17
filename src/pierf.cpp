// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include <iostream> // for cout and cin
#include <fstream>
#include <string>
#include "IpAddress.hpp"
#include "MacAddress.hpp"
#include "ParseConfig.hpp"
#include "Exception.hpp"

extern "C"
  {
#include "expat.h"
#include <pcap.h>
  }

#ifndef VERSION
#define VERSION "Makefile bug. No version specified"
#endif

using namespace std;

int main(int argc, char **argv)
  {
  if (argc != 2)
    {
    cout << "usage: " << argv[0] << " [configfile]" << endl;
    cout << "Version: " << VERSION << endl;
    cout << "Copyright (c) 2006-2007, Pieter Blommaert: see license.txt" << endl;
    return 1;
    }

  try
    {
    ParseConfig myConfig;
    myConfig.parse(argv[1]);
    }
  catch (Exception& e)
    {
    cout << "Error: " << e.what() << endl;
    }
  catch (exception& e)
    {
    cout << "Error: Unforeseen exception" << e.what() << endl;
    }
  catch (...)
    {
    cout << "Error: Unidentiable exception" << endl;
    }

  // Check the validity of the command line 

  return 0;
  }

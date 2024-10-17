// Copyright (c) 2006-2011, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "VarContainer.hpp"
#include "Counter.hpp"
#include "StringVar.hpp"
#include "string.h"
#include <typeinfo>
#include <sstream>

map<string,Var*>* VarContainer::mVars = NULL; // Initial value

VarContainer::VarContainer()
  {
  if (mVars == NULL)
    {
    mVars = new map<string,Var*>;
    }
  }

VarContainer::~VarContainer()
  {
  // Cleanup the entire vars container
  map<string,Var*>::iterator iter;
  for (iter=mVars->begin(); iter != mVars->end(); iter++)
    {
    Var* var = (*iter).second;
    delete var;
    }
  }


Var* VarContainer::createVar(const char** attr)
  {
  Var* var = NULL;
  char* strId=NULL; 
  char* strType=NULL;
  char* strValue=NULL;
  char* strConst=NULL;
  int i=0;
  while (attr[i] != NULL)
    {
    if (!strcmp(attr[i],"id"))
      {
      i++;
      strId = (char*) attr[i++];
      }
    else if (!strcmp(attr[i],"type"))
      {
      i++;
      strType = (char*) attr[i++];
      }
    else if (!strcmp(attr[i],"value"))
      {
      i++;
      strValue = (char*) attr[i++];
      }
    else if (!strcmp(attr[i],"const"))
      {
      i++;
      strConst = (char*) attr[i++];
      }
    else
      {
      throw Exception("Unexpected attribute: " + string(attr[i]) + " in <var> tag.");
      i++;
      }
    }

  if (strId != NULL && strType != NULL)
    {
    map<string,Var*>::iterator check = mVars->find(strId);
    if (check != mVars->end())
      {
      throw Exception("A variable with this id has been defined before: " + string(strId));
      }
    if (!strcmp(strType,"counter"))
      {
      if (strConst != NULL)
        {
        if (strcmp(strConst,"no")) // must be no
          {
          throw Exception("A counter can only have const attribute = no. Reading: " + string(strConst));
          }
        }
      var = new Counter(strId);
      var->setConst(false);
      }
    else if (!strcmp(strType,"string"))
      {
      var = new StringVar(strId);
      var->setConst(true);
      if (strConst != NULL)
        {
        if (!strcmp(strConst,"no")) // must be no
          {
          var->setConst(false);
          }
        }
      }
    else
      {
      throw Exception("Unsupported type in <var> tag: " + string(strType));
      }

    }
  else
    {
    string missing = "Missing mandatory attribute(s) to <var> tag: ";
    if (strId == NULL)
      {
      missing += "id ";
      }
    if (strType == NULL)
      {
      missing += "type";
      }
    throw Exception(missing);
    }

  (*mVars)[strId] = var;
  if (strValue != NULL)
    {
    var->setStringValue(strValue);
    }
  return var;
  }

Var* VarContainer::getVar (const char* var)
  {
  map<string,Var*>::iterator check = mVars->find(var);
  if (check == mVars->end())
    {
    return NULL;
    }
  // else
  return (*mVars)[var];

  }

Var* VarContainer::getVar (const string& var)
  {
  return getVar(var.c_str());
  }

VarContainer::Type VarContainer::getVarType (const char* var)
  {
  map<string,Var*>::iterator check = mVars->find(var);
  if (check == mVars->end())
    {
    return eUnexisting;
    }

  Var* checkVar = (*check).second;

  if (typeid(*checkVar) == typeid(Counter))
    { 
    return eCounter; 
    }
  else if (typeid(*checkVar) == typeid(StringVar))
    {
    return eString;
    }

  // should never get here  
  return eUnexisting;
  }

string VarContainer::getString() const
  {
  stringstream retval;
  
  map<string,Var*>::const_iterator varIter;
  for (varIter=mVars->begin(); varIter != mVars->end(); varIter++)
    {
    Var* var = (*varIter).second;
    retval << var->getString();
    }
  retval << endl << flush;
  return retval.str();  
  }

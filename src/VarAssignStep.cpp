// Copyright (c) 2006, Pieter Blommaert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "VarAssignStep.hpp"

#include <iostream> // for cout and cin
#include <fstream>
#include <sstream>
#include <string.h>
#include <typeinfo>
#include "VarContainer.hpp"

VarAssignStep::VarAssignStep()
  :mVar(NULL), mElement(NULL)
  {
  }

VarAssignStep::~VarAssignStep()
  {
  }

void VarAssignStep::setVar(const char* varName) noexcept(false)
  {
  Var* var = VarContainer::getVar(varName);
  if (var == NULL)
    {
    throw Exception("Unexisting variable with name: " + string(varName));
    }
  if (var->isConst())
    {
    throw Exception("Cannot assign to a constant: " + string(varName));
    }
  mVar = var;
  }

void VarAssignStep::setFormula(const char* formula)
  {
  mFormula = formula;
  }

void VarAssignStep::setElement(Element* element)
  {
  mElement = element;
  }

void VarAssignStep::copyFormula(VarAssignStep* other)
  {
  other->mFormula = mFormula;
  }

void VarAssignStep::play()
  {
  // tbd: regexp and proper formula handling
  // tbd: define a canonic, easily parsable formula and translate formula's to that form in setFormula (maybe stop using single string format, but a vector of (string+operator)).
  string newValue;
  char* curPos = (char*) mFormula.c_str();
  if (!strncmp(curPos,"concat(",7))
    {
    curPos +=7;
    while (*curPos != ')' && *curPos != 0)
      {
      if (*curPos == '$')
        {
        // Variable
        string varName;
        while (*curPos != ',' && *curPos != ')' && *curPos != 0)
          {
          varName.push_back(*curPos++);
          }

        Var* var = VarContainer::getVar(varName.c_str());
        if (var == NULL)
          {
          throw Exception("Use of unexisting variable in variable-assign: " + varName);
          }
        newValue += var->getStringValue();
        }
      else if (*curPos == '"' or *curPos == '\'')
        {
        curPos++;
        while (*curPos != '"' && *curPos != 0)
          {
          newValue.push_back(*curPos++);
          }
        if (*curPos == '"')
          {
          curPos++;
          }
        else
          {
          // tbd: improve error feedback
          throw Exception("Syntax error in variable-assign formula (quote missing)");
          }
        }
      if (*curPos == ',')
        {
        curPos++;
        }
      else if (*curPos != ')')
        {
        throw Exception("Syntax error in variable-assign formula (closing bracket missing)");
        }
      }
    if (*curPos++ != ')')
      {
      throw Exception("Syntax error in variable-assign formula (closing bracket missing)");
      }
    if (*curPos != 0)
      {
      throw Exception("Syntax error in variable-assign formula (unexpected end of formula)");      
      }
    }
  else if (!strncmp(curPos,"hex($",5))
    {
    curPos += 4;
    string varName;
    while (*curPos != ')' && *curPos != 0)
      {
      varName.push_back(*curPos++);
      }
    if (*curPos++ != ')')
      {
      throw Exception("Syntax error in variable-assign formula (closing bracket missing)");
      }
    if (*curPos != 0)
      {
      throw Exception("Syntax error in variable-assign formula (unexpected end of formula)");      
      }

    Var* var = VarContainer::getVar(varName.c_str());
    if (var == NULL)
      {
      throw Exception("Use of unexisting variable in variable-assign: " + varName);
      }
    string integerStr = var->getStringValue();
    ulong32 integer = textToLong(integerStr.c_str());
    newValue += longToHexString(integer);
    }
  else if (!strncmp(curPos,"field(",6))
    {
    curPos += 6;
    if (mElement == NULL)
      {
      throw Exception("Cannot use field assignment outside packet receive\n");
      }

    string fieldName;
    while (*curPos != ')' && *curPos != 0)
      {
      fieldName.push_back(*curPos++);
      }

    if (*curPos++ != ')')
      {
      throw Exception("Syntax error in variable-assign formula (closing bracket missing)");
      }
    if (*curPos != 0)
      {
      throw Exception("Syntax error in variable-assign formula (unexpected end of formula)");      
      }
    
    mElement->getString(newValue,fieldName.c_str());
    }
  else
    {
    throw Exception ("Syntax error in variable-assing formula (bad start: must start with function)");
    }

  mVar->setStringValue(newValue.c_str());
  }

string VarAssignStep::getString() const // return string, no checks
  {
  stringstream retval;
  retval << "<assign-variable ";
  if (mVar != NULL)
    {
    retval << "name=\"" << mVar->getName() << "\" ";
    }
  if (mFormula.size() != 0)
    {
    retval << "value=\'" << mFormula << "\' ";
    }
  retval << "/>" << endl << flush;
  return retval.str();  
  }

Var* VarAssignStep::getVar()
  {
  return mVar;
  }
   
string VarAssignStep::getFormula()
  {
  return mFormula;
  }


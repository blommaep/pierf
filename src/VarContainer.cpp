#include "VarContainer.hpp"
#include "Counter.hpp"
#include "StringVar.hpp"
#include "string.h"
#include <typeinfo>

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


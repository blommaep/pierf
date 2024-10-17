#include "Field.hpp"
#include "VarContainer.hpp"
#define PCRE_STATIC 1
extern "C"
  {
#include "pcre.h"
  }

Field::Field()
  :mValueState(eUndef), mVar(NULL)
  {
  }

bool Field::hasValue() const
  {
  if (mValueState == eUndef)
    {
    return false;
    }
  return true; // any other case
  }

bool Field::needsAuto() const
  {
  if (mValueState == eUndef || mValueState == eAuto)
    {
    return true;
    }
  return false;
  }

bool Field::isManual() const
  {
  if (mValueState == eManual || mValueState == eVar)
    {
    return true;
    }
  return false;
  }

bool Field::isCaptured() const
  {
  if (mValueState == eCaptured)
    {
    return true;
    }
  return false;
  }

bool Field::isPrintable() const
  {
  switch (mValueState)
    {
    case eCaptured:
    case eManual:
    case eVar: // added eVar and eString, was apparently missing, not 100% sure if any side effects.
    case eString:
      return true;
    default:
      return false;
    };
  }

bool Field::isComparable() const
  {
  switch (mValueState)
    {
    case eCaptured:
    case eManual:
    case eString:
    case eVar:
      return true;
    default:
      return false;
    };
  }

bool Field::isVar() const
  {
  return mValueState == eVar;
  }

void Field::wasDefaulted() throw (Exception)
  {
  if (mValueState == eManual || mValueState == eAuto)
    {
    throw Exception ("A default value to a field after it has been given a value. May be a bug");
    }

  mValueState = eDefault;
  }

void Field::wasCaptured() 
  {
  mValueState = eCaptured;
  }

void Field::wasManuallySet()
  {
  if (mValueState != eVar) // in case of var, the "setmanual" is called with the variable content... but it stays var
    {
    mValueState = eManual;
    }
  }

void Field::wasAutoSet() throw (Exception)
  {
  mValueState = eAuto;
  }

void Field::setManual(const char* varStr, bool storeAsString) throw (Exception)
  {
  if (storeAsString)
    {
    //tbd: variable containing regexp? (not supported now)
    setString(varStr);
    }
  else
    {
    if (varStr[0] == '$') // variable
      {
      setManualFromVar(varStr);
      }
    else
      {
      setManualFromValue(varStr);
      }
    }
  }

void Field::setManualFromVar(const char* varStr) throw (Exception)
  {
  Var* var = VarContainer::getVar(varStr);
  if (var==NULL)
    {
    throw Exception ("Assigning undefined variable: " + string(varStr));
    }
  mVar = var; // always keep a reference to the var
  if (var->isConst())
    {
    setManualFromValue(var->getStringValue().c_str());
    }
  else
    {
    mValueState = eVar;
    copyVar(); // Always copy initial value
    }
  }

Var* Field::getVar()
  {
  return mVar;
  }

bool Field::copyVar() throw (Exception)
  {
  if (mValueState == eVar)
    {
    setManualFromValue(mVar->getStringValue().c_str());
    return true;
    }
  return false;
  }

void Field::setString(const char* inputString)
  {
  mString=new string(inputString);
  mValueState=eString;
  }

bool Field::isString() const
  {
  return (mValueState == eString);
  }

bool Field::getString(string& stringval) const
  {
  if (isString())
    {
    stringval = *mString;
    return true;
    }
  else
    return getStringFromBinary(stringval);
  }

string Field::getString() const
  {
  if (isString())
    {
    return *mString;
    }
  return getStringFromBinary();
  }

string Field::getConfigString() const
  {
  if (isString())
    {
    return *mString;
    }
  else if (isVar())
    {
    return mVar->getName();
    }
  else
    {
    return getStringFromBinary();
    }
  }

bool Field::matchByString(const Field& field)
  {
  if (isString())
    {
    pcre *re;
    const char *error;
    int erroffset;
    re = pcre_compile(
           mString->c_str(),          /* the pattern */
           0,                /* default options */
           &error,           /* for error message */
           &erroffset,       /* for error offset */
           NULL);            /* use default character tables */

    int rc;
    int ovector[30];
    string matchString;
    matchString = field.getString();
    rc = pcre_exec(
           re,             /* result of pcre_compile() */
           NULL,           /* we didn't study the pattern */
           matchString.c_str(),  /* the subject string */
           matchString.length(),             /* the length of the subject string */
           0,              /* start at offset 0 in the subject */
           0,              /* default options */
           ovector,        /* vector of integers for substring information */
           30);            /* number of elements (NOT size in bytes) */
    if (rc != 0)
      {
      return true;
      }
    return false;
    }
  else if (field.isString())
    {
//    pcrecpp::RE re(*(field.mString));
//    return re.FullMatch(getString());
    pcre *re;
    const char *error;
    int erroffset;
    re = pcre_compile(
           field.mString->c_str(),          /* the pattern */
           0,                /* default options */
           &error,           /* for error message */
           &erroffset,       /* for error offset */
           NULL);            /* use default character tables */

    int rc;
    int ovector[30];
    string matchString;
    matchString = getString();
    rc = pcre_exec(
           re,             /* result of pcre_compile() */
           NULL,           /* we didn't study the pattern */
           matchString.c_str(),  /* the subject string */
           matchString.length(),             /* the length of the subject string */
           0,              /* start at offset 0 in the subject */
           0,              /* default options */
           ovector,        /* vector of integers for substring information */
           30);            /* number of elements (NOT size in bytes) */
    if (rc != 0)
      {
      return true;
      }
    return false;
    }
  else
    {
    return false;
    }

  }

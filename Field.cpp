#include "Field.hpp"
#include "VarContainer.hpp"

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

void Field::setManual(const char* varStr) throw (Exception)
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


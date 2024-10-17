#include "Var.hpp"

Var::Var(const char* name)
  : mConst(false)
  {
  mName = name;
  }

Var::Var(const string& name)
  : mConst(false)
  {
  mName = name;
  }

string Var::getName()
  {
  return mName;
  }

void Var::setConst(bool isConst)
  {
  mConst = isConst;
  }

bool Var::isConst()
  {
  return mConst;
  }


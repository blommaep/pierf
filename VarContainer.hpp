#ifndef VARCONTAINER_HPP__
#define VARCONTAINER_HPP__

#include "generics.hpp"
#include "Var.hpp"
#include <string>
#include <map>

using namespace std;

class VarContainer
  {
  private:
    static map<string,Var*>* mVars;
  public:
    enum Type {eUnexisting, eCounter, eString};
    VarContainer();
    ~VarContainer();
    Var* createVar(const char** attr);
    static Var* getVar (const char* var);
    static Var* getVar (const string& var);
    static Type getVarType (const char* var);
  };

#endif

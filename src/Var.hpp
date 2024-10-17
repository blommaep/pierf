#ifndef VAR_HPP__
#define VAR_HPP__

#include "generics.hpp"
#include <string>

using namespace std;

// This class is supposed to be derived from for all Variable types
class Var
  {
  protected:
    string mName;
    bool mConst;
  public:
    Var(const char* name);
    Var(const string& name);
    virtual ~Var()
      {}

    string getName();
    // Set the value from an input string
    virtual void setStringValue(const char* inString) throw (Exception) = 0;
    // Get the value in string format (this is the most generic way, possibly the only way that can be really common to all variables)
    virtual string getStringValue() = 0;
    void setConst(bool isConst);
    bool isConst();
  };

#endif

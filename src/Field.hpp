#ifndef FIELD_HPP__
#define FIELD_HPP__

#include "generics.hpp"
#include "Var.hpp"
#include <string>

using namespace std;

class Field
  {
  protected:
    enum ValueState {eUndef, eDefault, eManual, eAuto, eCaptured, eVar, eString};
      // eUndef: the field has not been assigned any value => not ready to send
      // eDefault: the field has been given a default value, but (so far) no user configured value
      // eManual: the field has a user configured value => this is a definitive state
      // eAuto: there was no user configured value and no default, but could be completed by auto guessing
      // eString: the field has no full value, but is no more then a string saved from config file. Used for match by String
    ValueState mValueState;
    Var* mVar;
    string* mString;
  public:
    Field();
    virtual ~Field()
      {}

    bool hasValue() const;
    bool needsAuto() const; // undef or auto: auto needs auto again
    bool isManual() const;
    bool isCaptured() const;
    bool isPrintable() const;
    bool isComparable() const;
    bool isString() const;
    bool isVar() const;

    // These functions set the fields state
    void wasDefaulted() throw (Exception);
    void wasManuallySet();
    void wasCaptured();
    void wasAutoSet() throw (Exception);

    // These functions must be available for setting the fields value and 
    // must call the appropriate was... function.
    virtual void setDefault(const char* inString) throw (Exception) = 0;
    virtual void setManualFromValue(const char* inString) throw (Exception) = 0;
    virtual void setAuto(const char* inString) throw (Exception) = 0;
    virtual uchar* copyTo(uchar* toPtr) = 0;
    virtual bool analyze(uchar*& fromPtr, ulong32& remainingSize) = 0;
    void setManual(const char* varStr, bool storeAsString) throw (Exception);
    void setManualFromVar(const char* varStr) throw (Exception);
    void setString(const char* inputString);
    virtual string getStringFromBinary() const = 0 ; // return string, no checks
    virtual bool getStringFromBinary(string& stringval) const = 0; // return the value in string format
    string getString() const; // return string, no checks
    string getConfigString() const;
    bool getString(string& stringval) const;
    bool matchByString(const Field& field); // generic implementation, if fields are of different type,will also work but likely should never occur
    Var* getVar();
    bool copyVar() throw (Exception);
  };

#endif

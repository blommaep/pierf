#ifndef AUTOOBJECT_HPP__
#define AUTOOBJECT_HPP__

#include "generics.hpp"
#include "Exception.hpp"
#include <string>

using namespace std;

// The auto-object parent class gives the child classes proper tracking and
// handling of auto option

class AutoObject;

class AutoObject
  {
  public:
  enum AutoComplete {eManualConfig, eManualEnherit, eAutoConfig, eAutoEnherit};

  protected:
    AutoComplete mAuto;

  public:
    AutoObject();
    virtual ~AutoObject();
    void setAuto(AutoComplete newval);
    void setAuto(char* instr) noexcept(false);
    void enheritAuto(AutoObject* mother);
    void setOrEnheritAuto(char* instr, AutoObject* mother) noexcept(false);
    AutoComplete getAuto();
    bool isAuto(); // true if auto is set
  };

#endif

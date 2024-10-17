#ifndef BITFIELD13_HPP__
#define BITFIELD13_HPP__

#include "generics.hpp"
#include "Exception.hpp"
#include "Bitfield16.hpp"
#include <string>

using namespace std;

class Bitfield13: public Bitfield16
  {
  private:
    uchar mOffset;
    void setVal(ushort val) throw (Exception);
  public:
    Bitfield13();
    void setOffset(uchar offset) throw (Exception);
    uchar* copyTo(uchar* toPtr);
    bool analyze(uchar*& fromPtr, ulong& remainingSize); 
  };

#endif

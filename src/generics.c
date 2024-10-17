#include "generics.h"
#include "assert.h"

/*
ushort htons(ushort val) 
  {
#if __BYTE_ORDER==__LITTLE_ENDIAN
  return ((val>>8)&0xff) | (val<<8);
#else
  return val;
#endif
  }

ushort ntohs(ushort val)
  {
  return htons(val);
  }
*/

ulong32 htonl32(ulong32 val) 
  {
#if __BYTE_ORDER==__LITTLE_ENDIAN
  return (val>>24) | ((val&0xff0000)>>8) |
  ((val&0xff00)<<8) | (val<<24);
#else
  return hostlong;
#endif
  }

ulong32 ntohl32(ulong32 val) 
  {
  return htonl32(val);
  }

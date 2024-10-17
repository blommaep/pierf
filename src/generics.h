#ifndef GENERIC_H__
#define GENERIC_H__

#include "stdint.h"
#include "netinet/in.h"

typedef unsigned char uchar; // 8 bytes unsigned
typedef uint16_t ushort; // 16 bytes unsigned
typedef uint32_t ulong32; // 32 bytes unsigned

/*
ushort htons(ushort val) noexcept(false);
ushort ntohs(ushort val) noexcept(false);
*/
ulong32 htonl32(ulong32 val);
ulong32 ntohl32(ulong32 val);

#endif

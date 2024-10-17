#ifndef GENERIC_H__
#define GENERIC_H__

typedef unsigned char uchar; // 8 bytes unsigned
typedef unsigned short ushort; // 16 bytes unsigned
typedef unsigned long ulong; // 32 bytes unsigned

ushort htons(ushort val);
ushort ntohs(ushort val);
ulong htonl(ulong val);
ulong ntohl(ulong val);

#endif

//==========================================================================
// ObTools::Misc: md5.cc
//
// MD5 hash function
// MD5 message-digest code taken from Colin Plumb's 1993 public domain
// version, heavily modified into C++.
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-misc.h"
#include <stdint.h>

#if defined(__WIN32__)
// Assume Windows is always little-endian
#define __LITTLE_ENDIAN 1
#define __BYTE_ORDER __LITTLE_ENDIAN
#else
#if defined(__BSD__)
#include <machine/endian.h>
#else
#include <endian.h>
#endif
#endif

#include <sstream>
#include <iomanip>

namespace ObTools { namespace Misc {

//------------------------------------------------------------------------
// Byte reverse a block of 4-byte integers
// Note: this code is harmless on little-endian machines.
void MD5::byte_reverse(char *buf, unsigned longs)
{
  uint32_t t;
  do 
  {
    t = (uint32_t) ((unsigned) buf[3] << 8 | buf[2]) << 16 |
      ((unsigned) buf[1] << 8 | buf[0]);
    *(uint32_t *) buf = t;
    buf += 4;
  } 
  while (--longs);
}

// Make null if byte order known to be little-endian already
#if defined(__BYTE_ORDER) && defined (__LITTLE_ENDIAN) && \
                          (__BYTE_ORDER == __LITTLE_ENDIAN)
#define byte_reverse(a,b)
#endif

//------------------------------------------------------------------------
// Initialise (taken from MD5Init())
// Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
// initialization constants.
void MD5::initialise()
{
  ctx_buf[0] = 0x67452301;
  ctx_buf[1] = 0xefcdab89;
  ctx_buf[2] = 0x98badcfe;
  ctx_buf[3] = 0x10325476;
  
  ctx_bits[0] = 0;
  ctx_bits[1] = 0;
}

//------------------------------------------------------------------------
// Update to reflect the concatenation of another buffer full
// of bytes. (taken from MD5Update())
void MD5::update(const char *buf, unsigned len)
{
  uint32_t t;

  /* Update bitcount */
  t = ctx_bits[0];
  if ((ctx_bits[0] = t + ((uint32_t) len << 3)) < t)
    ctx_bits[1]++; 	/* Carry from low to high */
  ctx_bits[1] += len >> 29;

  t = (t >> 3) & 0x3f;	

  /* Handle any leading odd-sized chunks */
  if (t) 
  {
    char *p = (char *)ctx_in + t;

    t = 64 - t;
    if (len < t) 
    {
      memcpy(p, buf, len);
      return;
    }
    memcpy(p, buf, t);
    byte_reverse(ctx_in, 16);
    transform(ctx_buf, (uint32_t *)ctx_in);
    buf += t;
    len -= t;
  }
   
  /* Process data in 64-byte chunks */
  while (len >= 64) 
  {
    memcpy(ctx_in, buf, 64);
    byte_reverse(ctx_in, 16);
    transform(ctx_buf, (uint32_t *)ctx_in);
    buf += 64;
    len -= 64;
  }

  /* Handle any remaining bytes of data. */
  memcpy(ctx_in, buf, len);
}

//------------------------------------------------------------------------
// Final wrapup - pad to 64-byte boundary with the bit pattern 
// 1 0* (64-bit count of bits processed, MSB-first)
// Taken from MD5Final()
// Fills 'digest' with 16 binary bytes representing the MD5 sum 
void MD5::finalise(unsigned char digest[16])
{
  unsigned count;
  char *p;

  /* Compute number of bytes mod 64 */
  count = (ctx_bits[0] >> 3) & 0x3F;

  /* Set the first char of padding to 0x80.  This is safe since there is
     always at least one byte free */
  p = ctx_in + count;
  *p++ = 0x80;

  /* Bytes of padding needed to make 64 bytes */
  count = 64 - 1 - count;

  /* Pad out to 56 mod 64 */
  if (count < 8) 
  {
    /* Two lots of padding:  Pad the first block to 64 bytes */
    memset(p, 0, count);
    byte_reverse(ctx_in, 16);
    transform(ctx_buf, (uint32_t *)ctx_in);

    /* Now fill the next block with 56 bytes */
    memset(ctx_in, 0, 56);
  } 
  else 
  {
    /* Pad block to 56 bytes */
    memset(p, 0, count - 8);
  }

  byte_reverse(ctx_in, 14);

  /* Append length in bits and transform */
  ((uint32_t *)ctx_in)[14] = ctx_bits[0];
  ((uint32_t *)ctx_in)[15] = ctx_bits[1];

  transform(ctx_buf, (uint32_t *)ctx_in);
  byte_reverse((char *)ctx_buf, 4);
  memcpy(digest, ctx_buf, 16);
}

/* The four core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#define MD5STEP(f, w, x, y, z, data, s) \
	( w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x )

//------------------------------------------------------------------------
// MD5 Transform
// The core of the MD5 algorithm, this alters an existing MD5 hash to
// reflect the addition of 16 longwords of new data.  'update' blocks
// the data and converts bytes into longwords for this routine.
// Taken from MD5Transform()
void MD5::transform(uint32_t buf[4], uint32_t in[16])
{
  register uint32_t a, b, c, d;

  a = buf[0];
  b = buf[1];
  c = buf[2];
  d = buf[3];

  MD5STEP(F1, a, b, c, d, in[0] + 0xd76aa478, 7);
  MD5STEP(F1, d, a, b, c, in[1] + 0xe8c7b756, 12);
  MD5STEP(F1, c, d, a, b, in[2] + 0x242070db, 17);
  MD5STEP(F1, b, c, d, a, in[3] + 0xc1bdceee, 22);
  MD5STEP(F1, a, b, c, d, in[4] + 0xf57c0faf, 7);
  MD5STEP(F1, d, a, b, c, in[5] + 0x4787c62a, 12);
  MD5STEP(F1, c, d, a, b, in[6] + 0xa8304613, 17);
  MD5STEP(F1, b, c, d, a, in[7] + 0xfd469501, 22);
  MD5STEP(F1, a, b, c, d, in[8] + 0x698098d8, 7);
  MD5STEP(F1, d, a, b, c, in[9] + 0x8b44f7af, 12);
  MD5STEP(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
  MD5STEP(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
  MD5STEP(F1, a, b, c, d, in[12] + 0x6b901122, 7);
  MD5STEP(F1, d, a, b, c, in[13] + 0xfd987193, 12);
  MD5STEP(F1, c, d, a, b, in[14] + 0xa679438e, 17);
  MD5STEP(F1, b, c, d, a, in[15] + 0x49b40821, 22);
  
  MD5STEP(F2, a, b, c, d, in[1] + 0xf61e2562, 5);
  MD5STEP(F2, d, a, b, c, in[6] + 0xc040b340, 9);
  MD5STEP(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
  MD5STEP(F2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
  MD5STEP(F2, a, b, c, d, in[5] + 0xd62f105d, 5);
  MD5STEP(F2, d, a, b, c, in[10] + 0x02441453, 9);
  MD5STEP(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
  MD5STEP(F2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
  MD5STEP(F2, a, b, c, d, in[9] + 0x21e1cde6, 5);
  MD5STEP(F2, d, a, b, c, in[14] + 0xc33707d6, 9);
  MD5STEP(F2, c, d, a, b, in[3] + 0xf4d50d87, 14);
  MD5STEP(F2, b, c, d, a, in[8] + 0x455a14ed, 20);
  MD5STEP(F2, a, b, c, d, in[13] + 0xa9e3e905, 5);
  MD5STEP(F2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
  MD5STEP(F2, c, d, a, b, in[7] + 0x676f02d9, 14);
  MD5STEP(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);
  
  MD5STEP(F3, a, b, c, d, in[5] + 0xfffa3942, 4);
  MD5STEP(F3, d, a, b, c, in[8] + 0x8771f681, 11);
  MD5STEP(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
  MD5STEP(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
  MD5STEP(F3, a, b, c, d, in[1] + 0xa4beea44, 4);
  MD5STEP(F3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
  MD5STEP(F3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
  MD5STEP(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
  MD5STEP(F3, a, b, c, d, in[13] + 0x289b7ec6, 4);
  MD5STEP(F3, d, a, b, c, in[0] + 0xeaa127fa, 11);
  MD5STEP(F3, c, d, a, b, in[3] + 0xd4ef3085, 16);
  MD5STEP(F3, b, c, d, a, in[6] + 0x04881d05, 23);
  MD5STEP(F3, a, b, c, d, in[9] + 0xd9d4d039, 4);
  MD5STEP(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
  MD5STEP(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
  MD5STEP(F3, b, c, d, a, in[2] + 0xc4ac5665, 23);
  
  MD5STEP(F4, a, b, c, d, in[0] + 0xf4292244, 6);
  MD5STEP(F4, d, a, b, c, in[7] + 0x432aff97, 10);
  MD5STEP(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
  MD5STEP(F4, b, c, d, a, in[5] + 0xfc93a039, 21);
  MD5STEP(F4, a, b, c, d, in[12] + 0x655b59c3, 6);
  MD5STEP(F4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
  MD5STEP(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
  MD5STEP(F4, b, c, d, a, in[1] + 0x85845dd1, 21);
  MD5STEP(F4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
  MD5STEP(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
  MD5STEP(F4, c, d, a, b, in[6] + 0xa3014314, 15);
  MD5STEP(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
  MD5STEP(F4, a, b, c, d, in[4] + 0xf7537e82, 6);
  MD5STEP(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
  MD5STEP(F4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
  MD5STEP(F4, b, c, d, a, in[9] + 0xeb86d391, 21);

  buf[0] += a;
  buf[1] += b;
  buf[2] += c;
  buf[3] += d;
}

//--------------------------------------------------------------------------
// C++ friendly version: MD5 sum a string (returns hex string)
string MD5::sum(const string& text)
{
  char buf[64];

  initialise();

  // Get bytes 64 at a time to help the blocking
  string::size_type size = text.size();
  string::size_type n = 0;

  while (size-n >= 64)
  {
    text.copy(buf, 64, n);
    update(buf, 64);
    n+=64;
  }

  // Do the rest, if any
  if (n<size)
  {
    text.copy(buf, size-n, n);
    update(buf, size-n);
  }

  // Get the digest
  unsigned char digest[16];
  finalise(digest);

  // Turn it back into a hex string
  ostringstream oss;
  for(int i=0; i<16; i++)
    oss << hex << setfill('0') << setw(2) << (unsigned)digest[i];
  return oss.str();
}

//------------------------------------------------------------------------
// Destructor - like the original, nukes the input 'in case it's sensitive'
MD5::~MD5() 
{ 
  memset(ctx_in, 0, 64); 
};

}} // namespaces

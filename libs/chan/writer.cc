//==========================================================================
// ObTools::Chan: writer.cc
//
// Generic protocol/format writer
//
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-chan.h"

// Network headers for htonl
#if defined(__WIN32__)
#include <winsock.h>
#else
#include <netinet/in.h>
#endif

namespace ObTools { namespace Channel {

//--------------------------------------------------------------------------
// Write a network byte order (MSB-first) 4-byte integer to the channel
// Throws Error on failure
void Writer::write_nbo_32(uint32_t i) throw (Error)
{
  uint32_t n = htonl(i);
  write(&n, 4);
}

//--------------------------------------------------------------------------
// Write a network byte order (MSB-first) 8-byte integer to the channel
// Throws Error on failure
void Writer::write_nbo_64(uint64_t i) throw (Error)
{
  write_nbo_32((uint32_t)(i >> 32));
  write_nbo_32((uint32_t)(i & 0xFFFFFFFF));
}

}} // namespaces




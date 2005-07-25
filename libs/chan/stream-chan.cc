//==========================================================================
// ObTools::Chan: stream-chan.cc
//
// std::stream channels (StreamReader & StreamWriter)
//
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-chan.h"
#include "ot-net.h"

#define SKIP_BUF_SIZE 4096

namespace ObTools { namespace Channel {

//==========================================================================
// Stream Reader

// Read implementation
size_t StreamReader::basic_read(void *buf, size_t count) throw (Error)
{
  if (!sin) throw Error(1, "Stream failed");

  if (buf)
    sin.read((char *)buf, count);
  else
    sin.ignore(count);

  size_t n = sin.gcount();
  offset += n;
  return n;
}

//==========================================================================
// Stream Writer

// Write implementation
void StreamWriter::basic_write(const void *buf, size_t count) throw (Error)
{
  if (!sout) throw Error(1, "Stream failed");

  sout.write((char *)buf, count);
  offset += count;
}

}} // namespaces



//==========================================================================
// ObTools::Chan: fd-chan.cc
//
// Direct file descriptor channels (FDReader & FDWriter)
//
// Copyright (c) 2010 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-chan.h"
#include "ot-net.h"
#include <errno.h>

#define SKIP_BUF_SIZE 4096

namespace ObTools { namespace Channel {

//==========================================================================
// FD Reader

// Read implementation
size_t FDReader::basic_read(void *buf, size_t count) throw (Error)
{
  ssize_t n;
  if (buf)
  {
    n = ::read(fd, buf, count);
  }
  else
  {
    char temp[SKIP_BUF_SIZE];  // Skip up to this many bytes
    n = ::read(fd, temp, count);
  }
  
  if (n<0) throw Error(2, strerror(errno));

  offset += n;
  return n;
}

//==========================================================================
// FD Writer

// Write implementation
void FDWriter::basic_write(const void *buf, size_t count) throw (Error)
{
  ssize_t n = ::write(fd, buf, count);
  if (n<0 || (size_t)n != count) throw Error(2, strerror(errno));
  offset += count;
}

}} // namespaces




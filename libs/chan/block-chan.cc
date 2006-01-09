//==========================================================================
// ObTools::Chan: block-chan.cc
//
// Memory block channels (BlockReader & BlockWriter)
//
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-chan.h"

namespace ObTools { namespace Channel {

//==========================================================================
// Block Reader

// Read implementation
size_t BlockReader::basic_read(void *buf, size_t count) throw (Error)
{
  // Limit to length available
  if (length-offset < count) count = length-offset;

  if (count)
  {
    if (buf) memcpy(buf, data, count);
    data += count;
    offset += count;
  }

  return count;
}

//--------------------------------------------------------------------------
// Skip N bytes
void BlockReader::skip(int n) throw (Error)
{
  if (offset+n > length)
  {
    n = length-offset;
    throw Error(1, "Skip beyond end of block");
  }

  offset += n;
  data += n;
}

//==========================================================================
// Block Writer

// Write implementation
void BlockWriter::basic_write(const void *buf, size_t count) throw (Error)
{
  // Must fit, or error
  if (count <= length-offset)
  {
    memcpy(data, buf, count);
    data += count;
    offset += count;
  }
  else throw Error(1, "Data block overflowed");
}

//--------------------------------------------------------------------------
// Skip N bytes
void BlockWriter::skip(int n) throw (Error)
{
  // Must fit, or error
  if (offset+n <= length)
  {
    memset(data, 0, n);
    data += n;
    offset += n;
  }
  else throw Error(1, "Data block overflowed in skip");
}


}} // namespaces




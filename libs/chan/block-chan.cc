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
  if (length < count) count = length;

  if (count)
  {
    memcpy(buf, data, count);
    data += count;
    length -= count;
  }

  return count;
}

//==========================================================================
// Block Writer

// Write implementation
void BlockWriter::basic_write(const void *buf, size_t count) throw (Error)
{
  // Must fit, or error
  if (count <= length)
  {
    memcpy(data, buf, count);
    data += count;
    length -= count;
  }
  else throw Error(1, "Data block overflowed");
}

}} // namespaces




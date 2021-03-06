//==========================================================================
// ObTools::Chan: block-chan.cc
//
// Memory block channels (BlockReader & BlockWriter)
//
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-chan.h"

namespace ObTools { namespace Channel {

//==========================================================================
// Block Reader

// Read implementation
size_t BlockReader::basic_read(void *buf, size_t count)
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

// Skip N bytes
void BlockReader::skip(size_t n )
{
  if (offset+n > length)
  {
    n = length-offset;
    throw Error(1, "Skip beyond end of block");
  }

  offset += n;
  data += n;
}

// Rewind implementation
void BlockReader::rewind(size_t n)
{
  if (n<=offset)
  {
    data-=n;
    offset-=n;
  }
  else throw Error(1, "Rewound too far");
}

//==========================================================================
// Block Writer

// Write implementation
void BlockWriter::basic_write(const void *buf, size_t count)
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

// Skip N bytes
void BlockWriter::skip(size_t n)
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

// Rewind implementation
void BlockWriter::rewind(size_t n)
{
  if (n<=offset)
  {
    data-=n;
    offset-=n;
  }
  else throw Error(1, "Rewound too far");
}

}} // namespaces




//==========================================================================
// ObTools::Chan: string-chan.cc
//
// Memory string channels (StringReader & StringWriter)
//
// Copyright (c) 2007 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-chan.h"

namespace ObTools { namespace Channel {

//==========================================================================
// String Reader

// Read implementation
size_t StringReader::basic_read(void *buf, size_t count)
{
  size_t length = data.size();

  // Limit to length available
  if (length-offset < count) count = length-offset;

  if (count)
  {
    if (buf) data.copy(static_cast<char *>(buf), count, offset);
    offset += count;
  }

  return count;
}

// Skip N bytes
void StringReader::skip(size_t n)
{
  size_t length = data.size();

  if (offset+n > length)
    throw Error(1, "Skip beyond end of string");

  offset += n;
}

// Rewind implementation
void StringReader::rewind(size_t n)
{
  if (n<=offset)
    offset-=n;
  else
    throw Error(1, "Rewound too far");
}

//==========================================================================
// String Writer

// Write implementation
void StringWriter::basic_write(const void *buf, size_t count)
{
  // Just append - no limit!
  data.append(static_cast<const char *>(buf), count);
  offset += count;
}

// Skip N bytes
void StringWriter::skip(size_t n)
{
  // Append zero bytes
  data.append(n, 0);
  offset += n;
}

// Rewind implementation
void StringWriter::rewind(size_t n)
{
  if (n<=offset)
  {
    offset-=n;

    // Truncate to offset
    data.erase(offset);
  }
  else throw Error(1, "Rewound too far");
}

}} // namespaces




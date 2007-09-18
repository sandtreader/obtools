//==========================================================================
// ObTools::Chan: string-chan.cc
//
// Memory string channels (StringReader & StringWriter)
//
// Copyright (c) 2007 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-chan.h"

namespace ObTools { namespace Channel {

//==========================================================================
// String Reader

// Read implementation
size_t StringReader::basic_read(void *buf, size_t count) throw (Error)
{
  size_t length = data.size();

  // Limit to length available
  if (length-offset < count) count = length-offset;

  if (count)
  {
    if (buf) data.copy((char *)buf, count, offset);
    offset += count;
  }

  return count;
}

// Skip N bytes
void StringReader::skip(size_t n) throw (Error)
{
  size_t length = data.size();

  if (offset+n > length)
    throw Error(1, "Skip beyond end of string");

  offset += n;
}

// Rewind implementation
void StringReader::rewind(size_t n) throw (Error)
{
  if (n<=offset)
    offset-=n;
  else 
    throw Error(1, "Rewound too far");
}

//==========================================================================
// String Writer

// Write implementation
void StringWriter::basic_write(const void *buf, size_t count) throw (Error)
{
  // Just append - no limit!
  data.append((const char *)buf, count);
  offset += count;
}

// Skip N bytes
void StringWriter::skip(size_t n) throw (Error)
{
  // Append zero bytes
  data.append(n, 0);
  offset += n;
}

// Rewind implementation
void StringWriter::rewind(size_t n) throw (Error)
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




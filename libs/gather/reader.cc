//==========================================================================
// ObTools::Gather: reader.cc
//
// Gather buffer channel (Gather::Reader)
//
// Copyright (c) 2012 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-gather.h"

namespace ObTools { namespace Gather {

//==========================================================================
// Gather reader

// Read implementation
size_t Reader::basic_read(void *buf, size_t count)
{
  if (count)
  {
    count = buffer.copy(reinterpret_cast<data_t *>(buf), it, count);
    it += count;
    offset += count;
  }
  return count;
}

// Skip N bytes
void Reader::skip(size_t n)
{
  if (offset + n > buffer.get_length())
    throw Channel::Error(1, "Skip beyond end of block");

  offset += n;
  it += n;
}

// Rewind implementation
void Reader::rewind(size_t n)
{
  if (n <= offset)
  {
    offset -= n;
    it -= n;
  }
  else throw Channel::Error(1, "Rewound too far");
}

}} // namespaces

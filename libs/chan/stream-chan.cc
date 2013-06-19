//==========================================================================
// ObTools::Chan: stream-chan.cc
//
// std::stream channels (StreamReader & StreamWriter)
//
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
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
    sin.read(static_cast<char *>(buf), count);
  else
    sin.ignore(count);

  size_t n = sin.gcount();
  offset += n;
  return n;
}

// Rewind implementation
void StreamReader::rewind(size_t n) throw (Error)
{
  if (n<=offset)
  {
    offset-=n;
    sin.seekg(-static_cast<streamoff>(n), ios_base::cur);
    if (!sin) throw Error(2, "Can't rewind");
  }
  else throw Error(1, "Rewound too far");
}

//==========================================================================
// Stream Writer

// Write implementation
void StreamWriter::basic_write(const void *buf, size_t count) throw (Error)
{
  if (!sout) throw Error(1, "Stream failed");

  sout.write(static_cast<const char *>(buf), count);
  offset += count;
}

// Rewind implementation
void StreamWriter::rewind(size_t n) throw (Error)
{
  if (n<=offset)
  {
    offset-=n;
    sout.seekp(-static_cast<streamoff>(n), ios_base::cur);
    if (!sout) throw Error(2, "Can't rewind");
  }
  else throw Error(1, "Rewound too far");
}

}} // namespaces




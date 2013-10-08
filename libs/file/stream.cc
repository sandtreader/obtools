//==========================================================================
// ObTools::File: stream.cc
//
// File stream implementation
//
// Copyright (c) 2013 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-file.h"
#include <algorithm>

#if defined(__WIN32__)
// Note: stati versions, still 32-bit time_t
#define CPATH(fn) Path::utf8_to_wide(fn).c_str()
#define OPEN _wopen
#include <windows.h>
#else
#if defined(__APPLE__)
// No LARGEFILE, otherwise sensible
#define O_LARGEFILE 0
#endif
#include <unistd.h>
#include <fcntl.h>
#define O_BINARY 0
#define CPATH(fn) fn.c_str()
#define OPEN ::open
#endif
#include <iostream>

namespace ObTools { namespace File {

//==========================================================================
// BufferedOutFileBuf

//--------------------------------------------------------------------------
// Handle characters one at a time
streamsize BufferedOutFileBuf::xsputn(const char *s, streamsize n)
{
  streamsize avail = this->epptr() - this->pptr();
  if (avail >= n)
    return __streambuf_type::xsputn(s, n);
  else
    return filebuf::xsputn(s, n);
}

//==========================================================================
// BufferedOutStream

//--------------------------------------------------------------------------
// Constructors
BufferedOutStream::BufferedOutStream()
{
  file_buf.pubsetbuf(0, 0);
  init(&file_buf);
}

BufferedOutStream::BufferedOutStream(const char *filename,
                                     ios::openmode mode,
                                     uint64_t buffer_size):
  buffer(buffer_size)
{
  file_buf.pubsetbuf(&buffer[0], buffer.size());
  init(&file_buf);
  open(filename, mode);
}

BufferedOutStream::BufferedOutStream(const string& filename,
                                     ios::openmode mode,
                                     uint64_t buffer_size):
  buffer(buffer_size)
{
  file_buf.pubsetbuf(&buffer[0], buffer.size());
  init(&file_buf);
  open(CPATH(filename), mode);
}

//--------------------------------------------------------------------------
// Set buffer size
void BufferedOutStream::set_buffer_size(uint64_t buffer_size)
{
  if (buffer_size > buffer.size())
  {
    buffer.resize(buffer_size);
    file_buf.pubsetbuf(&buffer[0], buffer_size);
  }
  else
  {
    file_buf.pubsetbuf(&buffer[0], buffer_size);
    buffer.resize(buffer_size);
  }
}

//--------------------------------------------------------------------------
// Get buffer size
uint64_t BufferedOutStream::get_buffer_size() const
{
  return buffer.size();
}

}} // namespaces

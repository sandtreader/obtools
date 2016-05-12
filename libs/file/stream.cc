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
#include <memory>

namespace ObTools { namespace File {

//==========================================================================
// BufferedOutFileBuf

//--------------------------------------------------------------------------
// Handle characters one at a time
streamsize BufferedOutFileBuf::xsputn(const char *s, streamsize n)
{
  auto avail = this->epptr() - this->pptr();
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

//==========================================================================
// MultiOutFileBuf

//--------------------------------------------------------------------------
// Handle characters one at a time
streamsize MultiOutFileBuf::xsputn(const char *s, streamsize n)
{
  auto result = streamsize{0};
  for (auto it = file_bufs.begin(); it != file_bufs.end(); ++it)
  {
    result = (*it)->sputn(s, n);
  }
  return result;
}

//--------------------------------------------------------------------------
// Put character on overflow
int MultiOutFileBuf::overflow(int c)
{
  for (auto it = file_bufs.begin(); it != file_bufs.end(); ++it)
  {
    (*it)->sputn(reinterpret_cast<const char *>(&c), 1);
  }
  return c;
}

//--------------------------------------------------------------------------
// Set internal position pointer to relative position
// Implemented for sake of MultiOutStream::tellp() functionality
streampos MultiOutFileBuf::seekoff(streamoff off, ios_base::seekdir way,
                                   ios_base::openmode which)
{
  auto result = streampos{-1};
  for (auto it = file_bufs.begin(); it != file_bufs.end(); ++it)
  {
    result = (*it)->pubseekoff(off, way, which);
  }
  return result;
}

//==========================================================================
// MultiOutStream

//--------------------------------------------------------------------------
// Constructors
MultiOutStream::MultiOutStream():
  file_buf(file_bufs)
{
  file_buf.pubsetbuf(0, 0);
  init(&file_buf);
}

//--------------------------------------------------------------------------
// Test for file being open
bool MultiOutStream::is_open() const
{
  for (auto it = file_bufs.begin(); it != file_bufs.end(); ++it)
    if ((*it)->is_open())
      return true;
  return false;
}

//--------------------------------------------------------------------------
// Open a file
void MultiOutStream::open(const char *filename, ios_base::openmode mode)
{
  open_back(filename, mode);
}

//--------------------------------------------------------------------------
// Open a file
bool MultiOutStream::open_back(const char *filename, ios_base::openmode mode)
{
  unique_ptr<filebuf> buf(new filebuf);
  if (buf->open(filename, mode))
  {
    file_bufs.push_back(move(buf));
    return true;
  }
  return false;
}

//--------------------------------------------------------------------------
// Close file
void MultiOutStream::close()
{
  file_buf.pubsync();
  for (auto it = file_bufs.cbegin(); it != file_bufs.cend(); ++it)
  {
    (*it)->pubsync();
    if (!(*it)->close())
      setstate(ios_base::failbit);
  }
  file_bufs.clear();
}

}} // namespaces

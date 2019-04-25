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
#define OPEN open
#endif
#include <iostream>
#include <memory>

namespace ObTools { namespace File {

//==========================================================================
// BufferedOutFileBuf

//--------------------------------------------------------------------------
// Constructor
BufferedOutFileBuf::BufferedOutFileBuf(uint64_t size):
  buffer(size)
{
  if (size)
    pubsetbuf(&buffer[0], size);
  else
    pubsetbuf(nullptr, 0);
}

//--------------------------------------------------------------------------
// Handle characters one at a time
streamsize BufferedOutFileBuf::xsputn(const char *s, streamsize n)
{
  // stdc++ filebuf has a limit of 1K over which it will not write to the
  // buffer, but the base streambuf does not
  auto avail = this->epptr() - this->pptr();
  if (avail >= n)
    return streambuf::xsputn(s, n);
  else
    return filebuf::xsputn(s, n);
}

//--------------------------------------------------------------------------
// Resize buffer
void BufferedOutFileBuf::resize(uint64_t size)
{
  if (!size)
  {
    pubsetbuf(nullptr, 0);
    buffer.resize(0);
  }
  else if (size > buffer.size())
  {
    buffer.resize(size);
    pubsetbuf(&buffer[0], size);
  }
  else
  {
    pubsetbuf(&buffer[0], size);
    buffer.resize(size);
  }
}

//==========================================================================
// BufferedOutStream

//--------------------------------------------------------------------------
// Constructors
BufferedOutStream::BufferedOutStream():
  file_buf{0}
{
  init(&file_buf);
}

BufferedOutStream::BufferedOutStream(const string& filename,
                                     uint64_t buffer_size,
                                     ios::openmode mode):
  file_buf{buffer_size}
{
  init(&file_buf);
  OPEN(CPATH(filename), mode);
}

//==========================================================================
// MultiOutFileBuf

//--------------------------------------------------------------------------
// Handle characters one at a time
streamsize MultiOutFileBuf::xsputn(const char *s, streamsize n)
{
  auto result = streamsize{0};
  for (auto& it: file_bufs)
    result = it->sputn(s, n);
  return result;
}

//--------------------------------------------------------------------------
// Put character on overflow
int MultiOutFileBuf::overflow(int c)
{
  for (auto& it: file_bufs)
    it->sputn(reinterpret_cast<const char *>(&c), 1);
  return c;
}

//--------------------------------------------------------------------------
// Set internal position pointer to relative position
// Implemented for sake of MultiOutStream::tellp() functionality
streampos MultiOutFileBuf::seekoff(streamoff off, ios_base::seekdir way,
                                   ios_base::openmode which)
{
  auto result = streampos{-1};
  for (auto& it: file_bufs)
    result = it->pubseekoff(off, way, which);
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
  for (const auto& it: file_bufs)
    if (it->is_open())
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
  auto buf = make_unique<filebuf>();
  if (buf->open(filename, mode))
  {
    file_bufs.emplace_back(buf.release());
    return true;
  }
  return false;
}

//--------------------------------------------------------------------------
// Close file
void MultiOutStream::close()
{
  file_buf.pubsync();
  for (const auto& it: file_bufs)
  {
    it->pubsync();
    if (!it->close())
      setstate(ios_base::failbit);
  }
  file_bufs.clear();
}

//==========================================================================
// BufferedMultiOutFileBuf

//--------------------------------------------------------------------------
// Handle characters one at a time
streamsize BufferedMultiOutFileBuf::xsputn(const char *s, streamsize n)
{
  auto result = streamsize{0};
  for (auto& it: file_bufs)
    result = it->sputn(s, n);
  return result;
}

//--------------------------------------------------------------------------
// Put character on overflow
int BufferedMultiOutFileBuf::overflow(int c)
{
  for (auto& it: file_bufs)
    it->sputn(reinterpret_cast<const char *>(&c), 1);
  return c;
}

//--------------------------------------------------------------------------
// Set internal position pointer to relative position
// Implemented for sake of MultiOutStream::tellp() functionality
streampos BufferedMultiOutFileBuf::seekoff(streamoff off,
                                           ios_base::seekdir way,
                                           ios_base::openmode which)
{
  auto result = streampos{-1};
  for (auto& it: file_bufs)
    result = it->pubseekoff(off, way, which);
  return result;
}

//==========================================================================
// BufferedMultiOutStream

//--------------------------------------------------------------------------
// Constructors
BufferedMultiOutStream::BufferedMultiOutStream(uint64_t _buffer_size):
  file_buf{file_bufs}, buffer_size{_buffer_size}
{
  file_buf.pubsetbuf(nullptr, 0);
  init(&file_buf);
}

//--------------------------------------------------------------------------
// Set buffer size
void BufferedMultiOutStream::set_buffer_size(uint64_t _buffer_size)
{
  buffer_size = _buffer_size;
  for (auto& it: file_bufs)
    it->resize(buffer_size);
}

//--------------------------------------------------------------------------
// Test for file being open
bool BufferedMultiOutStream::is_open() const
{
  for (const auto& it: file_bufs)
    if (it->is_open())
      return true;
  return false;
}

//--------------------------------------------------------------------------
// Open a file
void BufferedMultiOutStream::open(const char *filename,
                                  ios_base::openmode mode)
{
  open_back(filename, mode);
}

//--------------------------------------------------------------------------
// Open a file
bool BufferedMultiOutStream::open_back(const char *filename,
                                       ios_base::openmode mode)
{
  auto buf = make_unique<BufferedOutFileBuf>(buffer_size);
  if (buf->open(filename, mode))
  {
    file_bufs.emplace_back(buf.release());
    return true;
  }
  return false;
}

//--------------------------------------------------------------------------
// Close file
void BufferedMultiOutStream::close()
{
  file_buf.pubsync();
  for (const auto& it: file_bufs)
  {
    it->pubsync();
    if (!it->close())
      setstate(ios_base::failbit);
  }
  file_bufs.clear();
}

}} // namespaces

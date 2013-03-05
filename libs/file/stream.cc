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

namespace ObTools { namespace File {

//--------------------------------------------------------------------------
// Constructor
BufferedOutStream::BufferedOutStream(const string& fn,
                                     uint64_t buffer_size,
                                     ios::openmode mode):
  fd(OPEN(CPATH(fn), O_LARGEFILE |
          ((mode & ios::app) ? O_APPEND : 0) |
          ((mode & ios::binary) ? O_BINARY : 0) |
          ((mode & ios::out) ? O_CREAT : 0) |
          ((mode & ios::trunc) ? O_TRUNC : 0) |
          ((mode & ios::out) ? ((mode & ios::in) ? O_RDWR : O_WRONLY)
                             : ((mode & ios::in) ? O_RDONLY : 0)),
          S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH
          )),
  buffer(buffer_size), buffer_pos(0), failbit(false)
{
  if (is_open() && (mode & ios::ate))
  {
    if (lseek64(fd, 0, SEEK_END) < 0)
      failbit = true;
  }
}

//--------------------------------------------------------------------------
// Evaluate stream
bool BufferedOutStream::operator!() const
{
  return failbit;
}

//--------------------------------------------------------------------------
// Is this stream open?
bool BufferedOutStream::is_open()
{
  return fd >= 0;
}

//--------------------------------------------------------------------------
// Current file position
streampos BufferedOutStream::tellp()
{
  if (is_open())
    return lseek64(fd, 0, SEEK_CUR);
  else
    return -1;
}

//--------------------------------------------------------------------------
// Write data to stream
BufferedOutStream& BufferedOutStream::write(const char* s, streamsize n)
{
  if (!failbit && n > 0)
  {
    if (buffer.size() < buffer_pos + n)
    {
      if (::write(fd, &buffer[0], buffer_pos)
          != static_cast<ssize_t>(buffer_pos))
        failbit = true;
      buffer_pos = 0;
    }
    if (buffer.size() < static_cast<vector<char>::size_type>(n))
    {
      if (::write(fd, s, n) != static_cast<ssize_t>(buffer_pos))
        failbit = true;
    }
    else
    {
      copy(s, s + n, &buffer[buffer_pos]);
      buffer_pos += n;
    }
  }
  return *this;
}

//--------------------------------------------------------------------------
// Destructor
BufferedOutStream::~BufferedOutStream()
{
  if (is_open())
  {
    if (!failbit)
      ::write(fd, &buffer[0], buffer_pos);
    close(fd);
  }
  buffer_pos = 0;
  fd = -1;
  failbit = false;
}

}} // namespaces

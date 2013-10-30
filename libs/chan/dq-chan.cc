//==========================================================================
// ObTools::Chan: dq-chan.cc
//
// Data Queue channels (DataQueueReader & DataQueueWriter)
//
// Copyright (c) 2010 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-chan.h"

#define SKIP_BUF_SIZE 4096

namespace ObTools { namespace Channel {

//==========================================================================
// Data Queue Reader

// Read implementation
size_t DataQueueReader::basic_read(void *buf, size_t count)
{
  // Note: DataQueue::read handles data=0 OK
  size_t n = dq.read(static_cast<MT::DataBlock::data_t *>(buf), count);
  offset += n;
  return n;
}

//==========================================================================
// Data Queue Writer

// Write implementation
void DataQueueWriter::basic_write(const void *buf, size_t count)
{
  dq.write(static_cast<const MT::DataBlock::data_t *>(buf), count);
  offset += count;
}

}} // namespaces




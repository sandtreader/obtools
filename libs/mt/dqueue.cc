//==========================================================================
// ObTools::MT: dqueue.cc
//
// Data queue implementation
//
// Copyright (c) 2010 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-mt.h"
#include "stdlib.h"
#include "string.h"

namespace ObTools { namespace MT {

//--------------------------------------------------------------------------
// Write a block to the queue
// Data is copied into the queue
void DataQueue::write(const DataBlock::data_t *data, DataBlock::size_t length) 
{
  DataBlock::data_t *copy = (DataBlock::data_t *)malloc(length);
  memcpy(copy, data, length);
  send(DataBlock(copy, length)); 
}

//--------------------------------------------------------------------------
// Read data from the queue - blocking
// Reads data to the amount requested, or to EOF
// If data is 0, just skips it
// Returns amount of data read
DataBlock::size_t DataQueue::read(DataBlock::data_t *data, 
				  DataBlock::size_t length)
{
  DataBlock::size_t n = 0;

  while (n<length && !eof)
  {
    // If we have a working block, see how much we can satisfy with it
    if (working_block.length)
    {
      DataBlock::size_t to_copy = length-n;
      DataBlock::size_t wb_avail = working_block.length - working_block_used;
      if (wb_avail < to_copy) to_copy = wb_avail;
      if (data) memcpy(data+n, working_block.data+working_block_used, to_copy);
      working_block_used += to_copy;
      n += to_copy;

      // Have we used it up?  Delete it
      if (working_block_used >= working_block.length)
      {
	free((void *)working_block.data);
	working_block.length = 0;
      }
    }

    // If not, or we just used it up, get another one
    if (!working_block.length)
    {
      working_block = wait();

      // Note EOF if we see an empty marker
      if (!working_block.data) eof=true;
      working_block_used = 0;
    }
  }

  return n;
}

}} // namespaces




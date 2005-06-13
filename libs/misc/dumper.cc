//==========================================================================
// ObTools::Misc: dumper.cc
//
// Hex dump output
//
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-misc.h"
#include <iomanip>

namespace ObTools { namespace Misc {

//------------------------------------------------------------------------
// Dump a block
void Dumper::dump(void *block, int length)
{
  unsigned char *p = (unsigned char *)block;
  int offset = 0;

  while (offset < length)
  {
    int w = length-offset;
    if (w>width) w=width;
    
    sout << hex << setw(4) << setfill('0') << offset << ':';
    for(int i=0; i<w; i++)
    {
      if (split && !(i%split)) sout << ' ';
      sout << hex << setw(2) << setfill('0') << (int)(p[offset+i]);
    }
    sout << endl;

    offset+=w;
  }

  // Restore channel to 'normal'
  sout << dec << setw(0) << setfill(' ');
}


}} // namespaces

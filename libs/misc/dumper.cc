//==========================================================================
// ObTools::Misc: dumper.cc
//
// Hex dump output
//
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-misc.h"
#include <iomanip>
#include <ctype.h>

namespace ObTools { namespace Misc {

//------------------------------------------------------------------------
// Dump a block
void Dumper::dump(const void *block, int length)
{
  const unsigned char *p = (const unsigned char *)block;
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
    if (ascii)
    {
      // Pad if last line
      for(int i=w; i<width; i++)
      {
	if (split && !(i%split)) sout << ' ';
	sout << "  ";
      }
	
      sout << " | ";
      for(int i=0; i<w; i++)
      {
	unsigned char c=p[offset+i];
	if (!isprint(c)) c='.';
	sout << c;
      }
    }

    sout << endl;

    offset+=w;
  }

  // Restore channel to 'normal'
  sout << dec << setw(0) << setfill(' ');
}


}} // namespaces

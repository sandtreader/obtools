//==========================================================================
// ObTools::Net: test-block.cc
//
// Test harness for memory block channel reader/writer
//
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-chan.h"
using namespace std;

//--------------------------------------------------------------------------
// Main

int main()
{
  char buf[40];
  ObTools::Channel::BlockWriter bw(buf, 40);

  bw.write_nbo_32(0x5041554C);
  bw.write_nbo_64(0x0102030405060708LL);
  bw.write("end");

  cout << "Bytes remaining: " << bw.remaining() << endl;

  ObTools::Channel::BlockReader br(buf, 40-bw.remaining());
  cout << hex << br.read_nbo_32() << endl;
  cout << hex << br.read_nbo_64() << endl;
  string s;
  if (!br.read(s, 3))
    cerr << "Can't read 'end' string\n";
  else
    cout << s << endl;
   
  return 0;  
}





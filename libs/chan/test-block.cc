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

  bw.write_byte(0x2A);
  bw.write_nbo_16(0x55AA);
  bw.align(2);
  bw.write_nbo_24(0xF1F2F3);
  bw.align(4);
  bw.write_nbo_32(0x5041554C);
  bw.write_nbo_64(0x0102030405060708LL);
  bw.write("end");

  cout << "Bytes written: " << bw.get_offset() << endl;

  ObTools::Channel::BlockReader br(buf, bw.get_offset());
  cout << hex << br.read_byte() << endl;
  cout << hex << br.read_nbo_16() << endl;
  br.align(2);
  cout << hex << br.read_nbo_24() << endl;
  br.align(4);
  cout << hex << br.read_nbo_32() << endl;
  cout << hex << br.read_nbo_64() << endl;
  string s;
  if (!br.read(s, 3))
    cerr << "Can't read 'end' string\n";
  else
    cout << s << endl;
   
  uint32_t n;
  if (br.read_nbo_32(n))
    cerr << "Block didn't end!\n";
  else
    cout << "Block ended OK\n";

  return 0;  
}





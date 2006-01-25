//==========================================================================
// ObTools::Net: test-stream.cc
//
// Test harness for stream channel reader/writer
//
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-chan.h"
#include <fstream>
using namespace std;

//--------------------------------------------------------------------------
// Main

int main()
{
  ofstream sout("test.out");
  ObTools::Channel::StreamWriter sw(sout);

  sw.write_byte(0x2A);
  sw.write_nbo_16(0x55AA);
  sw.align(2);
  sw.write_nbo_24(0xF1F2F3);
  sw.align(4);
  sw.skip(4);
  sw.write_nbo_32(0xDEADBEEF);
  sw.rewind(4);
  sw.write_nbo_32(0x5041554C);
  sw.write_nbo_64(0x0102030405060708LL);
  sw.write("end");

  cout << "Bytes written: " << sw.get_offset() << endl;
  sout.close();
  
  ifstream sin("test.out");
  ObTools::Channel::StreamReader sr(sin);
  cout << hex << sr.read_byte() << endl;
  cout << hex << sr.read_nbo_16() << endl;
  sr.align(2);
  cout << hex << sr.read_nbo_24() << endl;
  sr.align(4);
  sr.skip(8);
  sr.rewind(4);
  cout << hex << sr.read_nbo_32() << endl;
  cout << hex << sr.read_nbo_64() << endl;
  string s;
  sr.read(s, 3);
  cout << s << endl;
   
  uint32_t n;
  if (sr.read_nbo_32(n))
    cerr << "Stream didn't end!\n";
  else
    cout << "Stream ended OK\n";

  return 0;  
}





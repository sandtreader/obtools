//==========================================================================
// ObTools::Net: test-dq.cc
//
// Test harness for data queue channel reader/writer
//
// Copyright (c) 2010 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-chan.h"
using namespace std;

//--------------------------------------------------------------------------
// Main

int main()
{
  ObTools::MT::DataQueue dq;

  // Note:  We read and write in the same thread here - we're only testing
  // the channel bindings, not the blocking
  ObTools::Channel::DataQueueWriter bw(dq);

  bw.write_byte(0x2A);
  bw.write_nbo_16(0x55AA);
  bw.align(2);
  bw.write_nbo_24(0xF1F2F3);
  bw.align(4);
  bw.skip(4);
  bw.write_nbo_32(0x5041554C);
  bw.write_nbo_64(0x0102030405060708LL);
  bw.write("end");
  dq.close();

  cout << "Bytes written: " << bw.get_offset() << endl;

  ObTools::Channel::DataQueueReader br(dq);
  cout << hex << br.read_byte() << endl;
  cout << hex << br.read_nbo_16() << endl;
  br.align(2);
  cout << hex << br.read_nbo_24() << endl;
  br.align(4);
  br.skip(4);
  cout << hex << br.read_nbo_32() << endl;
  cout << hex << br.read_nbo_64() << endl;
  string s;
  br.read(s, 3);
  cout << s << endl;

  uint32_t n;
  if (br.read_nbo_32(n))
    cerr << "Block didn't end!\n";
  else
    cout << "Block ended OK\n";

  return 0;
}





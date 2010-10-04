//==========================================================================
// ObTools::Net: test-fd.cc
//
// Test harness for FD channel reader/writer
//
// Copyright (c) 2010 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-chan.h"
#include <fcntl.h>
#include <errno.h>
using namespace std;

//--------------------------------------------------------------------------
// Main

int main()
{
  int fd = open("test.out", O_RDWR | O_CREAT | O_TRUNC, 0644);
  if (fd < 0)
  {
    cerr << "Can't create test.out: " << strerror(errno) << endl;
    return 4;
  }

  ObTools::Channel::FDWriter sw(fd);

  sw.write_byte(0x2A);
  sw.write_nbo_16(0x55AA);
  sw.align(2);
  sw.write_nbo_24(0xF1F2F3);
  sw.align(4);
  sw.skip(4);
  sw.write_nbo_32(0x5041554C);
  sw.write_nbo_64(0x0102030405060708LL);
  sw.write("end");

  cout << "Bytes written: " << sw.get_offset() << endl;
  close(fd);
  
  fd = open("test.out", O_RDONLY);
  if (fd < 0)
  {
    cerr << "Can't read test.out: " << strerror(errno) << endl;
    return 4;
  }

  ObTools::Channel::FDReader sr(fd);
  cout << hex << sr.read_byte() << endl;
  cout << hex << sr.read_nbo_16() << endl;
  sr.align(2);
  cout << hex << sr.read_nbo_24() << endl;
  sr.align(4);
  sr.skip(4);
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





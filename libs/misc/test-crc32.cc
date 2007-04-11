//==========================================================================
// ObTools::Misc: test-crc32.cc
//
// Test harness for Misc library CRC32 functions
//
// Copyright (c) 2007 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-misc.h"
#include <iostream>
#include <iomanip>

using namespace std;
using namespace ObTools;

#define TEST_SIZE 256

//--------------------------------------------------------------------------
// CRC test
void test(unsigned char *data, int size, Misc::CRC::Algorithm algorithm,
	  char *name)
{
  cout << name << '\t';

  Misc::CRC crcer(algorithm);
  Misc::CRC::crc_t crc = crcer.calculate(data, size);
  cout << '\t' << hex << setw(4) << setfill('0') << crc;

  Misc::CRC crcer_r(algorithm, true);
  Misc::CRC::crc_t crc_r = crcer_r.calculate(data, size);
  cout << '\t' << hex << setw(4) << setfill('0') << crc_r;

  Misc::CRC crcer_f(algorithm, false, true);
  Misc::CRC::crc_t crc_f = crcer_f.calculate(data, size);
  cout << '\t' << hex << setw(4) << setfill('0') << crc_f;

  Misc::CRC crcer_rf(algorithm, true, true);
  Misc::CRC::crc_t crc_rf = crcer_rf.calculate(data, size);
  cout << '\t' << hex << setw(4) << setfill('0') << crc_rf << endl;
}

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  unsigned char data[TEST_SIZE];
  int size;

  if (argc > 1)
  {
    size = strlen(argv[1]);
    if (size > TEST_SIZE) abort();
    memcpy(data, argv[1], size);
  }
  else
  {
    size = 0;
    while (cin && size<TEST_SIZE)
    {
      int c = cin.get();
      if (c>=0) data[size++] = c;
    }
  }

  cout << "CRC32 of:\n";
  Misc::Dumper dumper(cout);
  dumper.dump(data, size);

  Misc::CRC32 crcer;
  Misc::CRC32::crc_t crc = crcer.calculate(data, size);

  cout << hex << setw(8) << setfill('0') << crc << endl;

  return 0;  
}




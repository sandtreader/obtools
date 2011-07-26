//==========================================================================
// ObTools::Misc: test-crc32.cc
//
// Test harness for Misc library CRC32 functions
//
// Copyright (c) 2007 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-misc.h"
#include <iostream>
#include <iomanip>

using namespace std;
using namespace ObTools;

#define TEST_SIZE 256

//--------------------------------------------------------------------------
// CRC32 test
void test(unsigned char *data, int size, Misc::CRC32::Algorithm algorithm,
	  const char *name)
{
  cout << name << '\t';

  Misc::CRC32 crcer(algorithm, false, false);
  Misc::CRC32::crc_t crc = crcer.calculate(data, size);
  cout << '\t' << hex << setw(8) << setfill('0') << crc;

  Misc::CRC32 crcer_r(algorithm, true, false);
  Misc::CRC32::crc_t crc_r = crcer_r.calculate(data, size);
  cout << '\t' << hex << setw(8) << setfill('0') << crc_r;

  Misc::CRC32 crcer_f(algorithm, false, true);
  Misc::CRC32::crc_t crc_f = crcer_f.calculate(data, size);
  cout << '\t' << hex << setw(8) << setfill('0') << crc_f;

  Misc::CRC32 crcer_rf(algorithm, true, true);
  Misc::CRC32::crc_t crc_rf = crcer_rf.calculate(data, size);
  cout << '\t' << hex << setw(8) << setfill('0') << crc_rf << endl;
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

  cout << endl;
  cout << "Algorithm\t\tBase\t\tRev\t\tFlip\t\tFlip+Rev\n";
  cout << "---------\t\t----\t\t---\t\t----\t\t--------\n";

  test(data, size, Misc::CRC32::ALGORITHM_CRC32, "CRC32     ");
  test(data, size, Misc::CRC32::ALGORITHM_CRC32C, "CRC32C    ");

  return 0;
}





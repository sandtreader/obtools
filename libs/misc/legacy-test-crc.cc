//==========================================================================
// ObTools::Misc: test-crc.cc
//
// Test harness for Misc library md5 functions
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
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
	  const char *name)
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

  cout << "CRCs of:\n";
  Misc::Dumper dumper(cout);
  dumper.dump(data, size);

  cout << endl;
  cout << "Algorithm\t\tBase\tRev\tFlip\tFlip+Rev\n";
  cout << "---------\t\t----\t---\t----\t--------\n";

  test(data, size, Misc::CRC::ALGORITHM_CRC16, "CRC16     ");
  test(data, size, Misc::CRC::ALGORITHM_CCITT, "CCITT     ");
  test(data, size, Misc::CRC::ALGORITHM_CCITT_ZERO, "CCITT_ZERO");
  test(data, size, Misc::CRC::ALGORITHM_CCITT_MOD,  "CCITT_MOD");

  return 0;  
}





//==========================================================================
// ObTools::Crypto: test-pkcs5.cc
//
// Test harness for Crypto library PKCS5 padding functions
//
// Copyright (c) 2006 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-crypto.h"
#include "ot-misc.h"
#include <iostream>

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  Misc::Dumper dumper(cout, 16, 4, true);

  const char *s = (argc>1)?argv[1]:"ABCD";
  int length = strlen(s);

  unsigned char *ps = Crypto::PKCS5::pad(
                                  reinterpret_cast<const unsigned char *>(s),
                                  length, 8);
  cout << "Padded:\n";
  dumper.dump(ps, length);

  cout << "Original length is " << Crypto::PKCS5::original_length(ps, length)
       << endl;

  free(ps);
  return 0;
}





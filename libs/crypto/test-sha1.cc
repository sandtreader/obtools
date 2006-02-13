//==========================================================================
// ObTools::Crypto: test-sha1.cc
//
// Test harness for Crypto library SHA1 digest functions
//
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-crypto.h"
#include "ot-misc.h"
#include <iostream>

using namespace std;
using namespace ObTools;

#define DATA_LEN 512
#define BLOCK_LEN 64

//--------------------------------------------------------------------------
// Main

int main()
{
  Misc::Random random;

  unsigned char buf[DATA_LEN];
  random.generate_binary(buf, DATA_LEN);

  // Do it with static function
  string d1 = Crypto::SHA1::digest(buf, DATA_LEN);
  cout << "One-block digest: " << d1 << endl;

  // Now do it with multiple-block object
  Crypto::SHA1 sha1;
  for(int i=0; i<DATA_LEN; i+=BLOCK_LEN)
    sha1.update(buf+i, BLOCK_LEN);

  string d2 = sha1.get_result();
  cout << "  N-block digest: " << d2 << endl;

  if (d1 == d2)
  {
    cout << "Digests match\n";
    return 0;
  }
  else
  {
    cerr << "Digests differ!\n";
    return 2;
  }
}





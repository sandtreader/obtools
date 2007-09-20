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
#include "ot-text.h"
#include <iostream>

using namespace std;
using namespace ObTools;

#define DATA_LEN 512
#define BLOCK_LEN 64

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  Misc::Random random;

  // By default, generate our own data
  unsigned char buf[DATA_LEN];
  random.generate_binary(buf, DATA_LEN);
  unsigned char *p = buf;
  int length = DATA_LEN;

  // Check for explicit string to SHA1
  if (argc > 1) 
  {
    p = (unsigned char *)argv[1];
    length = strlen(argv[1]);
    cout << "Input: [" << argv[1] << "]\n";
  }
  else cout << "Input: " << length << " bytes of random data\n";

  // Do it with static function
  string d1 = Crypto::SHA1::digest(p, length);
  cout << "One-block digest: " << d1 << endl;

  // Now do it with multiple-block object
  Crypto::SHA1 sha1;
  for(int i=0; i<length; i+=BLOCK_LEN)
    sha1.update(p+i, (length-i<BLOCK_LEN)?length-i:BLOCK_LEN);

  string d2 = sha1.get_result();
  cout << "  N-block digest: " << d2 << endl;

  if (d1 == d2)
  {
    cout << "Digests match\n";
  }
  else
  {
    cerr << "Digests differ!\n";
    return 2;
  }

  // Get Base64
  Text::Base64 base64;
  unsigned char shabuf[Crypto::SHA1::DIGEST_LENGTH];
  Crypto::SHA1::digest(p, length, shabuf);
  cout << "  Base64: " << base64.encode(shabuf, Crypto::SHA1::DIGEST_LENGTH, 0)
       << endl;

  return 0;
}





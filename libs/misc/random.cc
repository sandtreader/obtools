//==========================================================================
// ObTools::Misc: random.cc
//
// Randomiser function - generate random strings based on best random
// input available
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-misc.h"
#include <sstream>
#include <fstream>
#include <iomanip>

namespace ObTools { namespace Misc {

//------------------------------------------------------------------------
// Get some random bytes in the given buffer
static void _get_random_bytes(int n, unsigned char *p)
{
  // In Real Operating Systems (tm), /dev/urandom gives us an unlimited 
  // supply of random bytes from a hardware-assisted entropy pool
  ifstream f("/dev/urandom", ios_base::binary);  

  if (f)
  {
    f.read((char *)p, n);

    // Check we read it, otherwise drop through to zero
    if (f.gcount() == n) return;
  }

  // It's tempting to try a software version here, a la UUID algorithm,
  // but it's safer (and easier ;-) to generate zeros to force the user
  // to fix the RNG for their platform
  memset(p, 0, n);
}

//------------------------------------------------------------------------
// Get a random hex string up to N bytes long
string Random::generate_hex(int n)
{
  unsigned char *p = new unsigned char[n];
  _get_random_bytes(n, p);

  ostringstream oss;
  for(int i=0; i<n; i++)
    oss << hex << setfill('0') << setw(2) << (unsigned)p[i];

  delete[] p;
  return oss.str();
}


}} // namespaces

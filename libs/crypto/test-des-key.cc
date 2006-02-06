//==========================================================================
// ObTools::Crypto: test-des-key.cc
//
// Test harness for Crypto library DES key functions
//
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-crypto.h"
#include "ot-misc.h"
#include <iostream>

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main

int main()
{
  Crypto::DESKey k;
  k.create();

  if (k.valid)
  {
    cout << "Created DES key: " << k << endl;
    string ks = k.str();
    if (ks.size() != 16)
    {
      cerr << "Key output is only " << ks.size() << " characters!\n";
      return 2;
    }

    Crypto::DESKey k2(ks);
    if (k2.valid)
      cout << "Read back: " << k2 << endl;
    else
    {
      cerr << "Can't read back key\n";
      return 2;
    }

    if (k.str() == k2.str())
      cout << "Keys match\n";
    else
    {
      cerr << "Keys differ!\n";
      return 2;
    }
  }
  else 
  {
    cerr << "Can't create valid key\n";
    return 2;
  }

  return 0;  
}





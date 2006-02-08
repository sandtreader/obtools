//==========================================================================
// ObTools::Crypto: test-rsa-key.cc
//
// Test harness for Crypto library RSA key functions
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
// Test public or private
void test(bool priv)
{
  Crypto::RSAKey k(priv);
  k.create();

  if (k.valid)
  {
    cout << "Created RSA key:\n" << k << endl;
    string ks = k.str();

    Crypto::RSAKey k2(ks, priv);
    if (k2.valid)
      cout << "Read back:\n" << k2 << endl;
    else
    {
      cerr << "Can't read back key\n";
      exit(2);
    }

    if (k.str() == k2.str())
      cout << "Keys match\n";
    else
    {
      cerr << "Keys differ!\n";
      exit(2);
    }
  }
  else 
  {
    cerr << "Can't create valid key\n";
    exit(2);
  }
}

//--------------------------------------------------------------------------
// Main
int main()
{
  cout << "\nPublic key:\n";
  test(false);

  cout << "\nPrivate key:\n";
  test(true);
  return 0;  
}





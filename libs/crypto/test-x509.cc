//==========================================================================
// ObTools::Crypto: test-x509.cc
//
// Test harness for Crypto library X509 certificate functions
//
// Copyright (c) 2008 xMill Consulting Limited.  All rights reserved
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
  Crypto::Library library;

  // Read a certificate from stdin
  Crypto::Certificate cert;

  cin >> cert;
  if (!cert)
  {
    cerr << "Certificate invalid\n";
    return 2;
  }

  cout << "CN is " << cert.get_cn() << endl;

  cout << "\nPEM:\n";
  cout << cert;

  return 0;  
}





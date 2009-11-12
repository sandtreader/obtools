//==========================================================================
// ObTools::Crypto: test-des.cc
//
// Test harness for Crypto library DES encryption/decryption functions
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
// Test a DES encryptor
void test(Crypto::DES& des, const string& what, bool capture_iv = false)
{
#define TEST_LEN 64
  unsigned char data[TEST_LEN];
  unsigned char copy[TEST_LEN];
  Misc::Random random;
  random.generate_binary(data, TEST_LEN);
  memcpy(copy, data, TEST_LEN);

  Misc::Dumper dumper(cout, 16, 4, false);
  cout << endl << what << " - original:\n";
  dumper.dump(data, TEST_LEN);

  // Capture IV before encrypt to replace before decrypt
  Crypto::DESKey iv;
  if (capture_iv) iv = des.get_iv();

  if (!des.encrypt(data, TEST_LEN))
  {
    cerr << what << " - can't encrypt!\n";
    exit(2);
  }

  cout << what << " - encrypted:\n";
  dumper.dump(data, TEST_LEN);

  // Replace IV before decrypt
  if (capture_iv)
  {
    cout << "Restoring IV to " << iv << " (became " << des.get_iv() << ")\n";
    des.set_iv(iv);
  }

  if (!des.decrypt(data, TEST_LEN))
  {
    cerr << what << " - can't decrypt!\n";
    exit(2);
  }

  cout << what << " - decrypted:\n";
  dumper.dump(data, TEST_LEN);

  if (memcmp(data, copy, TEST_LEN))
  {
    cerr << what << " - MISMATCH!\n";
    exit(2);
  }
  cout << "Blocks match\n";
}

//--------------------------------------------------------------------------
// Main
int main()
{
  Crypto::DESKey k1;
  k1.create();
  cout << "Key1: " << k1 << endl;

  Crypto::DESKey k2;
  k2.create();
  cout << "Key2: " << k2 << endl;

  Crypto::DESKey k3;
  k3.create();
  cout << "Key3: " << k3 << endl;

  Crypto::DESKey iv(false);
  iv.create();
  cout << "IV:   " << iv << endl;

  // Try all the combinations
  // ECB versions
  Crypto::DES ecb1;
  ecb1.add_key(k1);
  test(ecb1, "ECB1");

  Crypto::DES ecb2;
  ecb2.add_key(k1);
  ecb2.add_key(k2);
  test(ecb2, "ECB2");

  Crypto::DES ecb3;
  ecb3.add_key(k1);
  ecb3.add_key(k2);
  ecb3.add_key(k3);
  test(ecb3, "ECB3");

  // CBC versions
  Crypto::DES cbc1;
  cbc1.add_key(k1);
  cbc1.set_iv(iv);
  test(cbc1, "CBC1", true);

  Crypto::DES cbc2;
  cbc2.add_key(k1);
  cbc2.add_key(k2);
  cbc2.set_iv(iv);
  test(cbc2, "CBC2", true);

  Crypto::DES cbc3;
  cbc3.add_key(k1);
  cbc3.add_key(k2);
  cbc3.add_key(k3);
  cbc3.set_iv(iv);
  test(cbc3, "CBC3", true);

  return 0;  
}





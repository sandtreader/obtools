//==========================================================================
// ObTools::Crypto: test-aes.cc
//
// Test harness for Crypto library AES encryption/decryption functions
//
// Copyright (c) 2010 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-crypto.h"
#include "ot-misc.h"
#include <iostream>

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Test a AES encryptor
void test(Crypto::AES& aes, const string& what, bool capture_iv = false,
          bool rtb = false)
{
  const int TEST_LEN(rtb ? 73 : 64);
  unsigned char data[TEST_LEN];
  unsigned char copy[TEST_LEN];
  Misc::Random random;
  random.generate_binary(data, TEST_LEN);
  memcpy(copy, data, TEST_LEN);

  Misc::Dumper dumper(cout, 16, 4, false);
  cout << endl << what << " - original:" << endl;
  dumper.dump(data, TEST_LEN);

  // Capture IV before encrypt to replace before decrypt
  Crypto::AESKey iv;
  if (capture_iv) iv = aes.get_iv();

  if (rtb)
  {
    if (!aes.encrypt_rtb(data, TEST_LEN))
    {
      cerr << what << " - can't encrypt!" << endl;
      exit(2);
    }
  }
  else
  {
    if (!aes.encrypt(data, TEST_LEN))
    {
      cerr << what << " - can't encrypt!" << endl;
      exit(2);
    }
  }

  cout << what << " - encrypted:" << endl;
  dumper.dump(data, TEST_LEN);

  // Replace IV before decrypt
  if (capture_iv)
  {
    cout << "Restoring IV to " << iv << " (became " << aes.get_iv() << ")\n";
    aes.set_iv(iv);
  }

  if (rtb)
  {
    if (!aes.decrypt_rtb(data, TEST_LEN))
    {
      cerr << what << " - can't decrypt!" << endl;
      exit(2);
    }
  }
  else
  {
    if (!aes.decrypt(data, TEST_LEN))
    {
      cerr << what << " - can't decrypt!" << endl;
      exit(2);
    }
  }

  cout << what << " - decrypted:" << endl;
  dumper.dump(data, TEST_LEN);

  if (memcmp(data, copy, TEST_LEN))
  {
    cerr << what << " - MISMATCH!" << endl;
    exit(2);
  }
  cout << "Blocks match" << endl;
}

//--------------------------------------------------------------------------
// Main
int main()
{
  Crypto::AESKey k128(Crypto::AESKey::BITS_128);
  k128.create();
  cout << "Key: " << k128 << endl;

  Crypto::AESKey k192(Crypto::AESKey::BITS_192);
  k192.create();
  cout << "Key: " << k192 << endl;

  Crypto::AESKey k256(Crypto::AESKey::BITS_256);
  k256.create();
  cout << "Key: " << k256 << endl;

  Crypto::AESKey iv(Crypto::AESKey::BITS_128, false);
  iv.create();
  cout << "IV:  " << iv << endl;

  // Try all the combinations
  // ECB versions
  Crypto::AES ecb128;
  ecb128.set_key(k128);
  test(ecb128, "ECB 128");

  Crypto::AES ecb192;
  ecb192.set_key(k192);
  test(ecb192, "ECB 192");

  Crypto::AES ecb256;
  ecb256.set_key(k256);
  test(ecb256, "ECB 256");

  // CBC versions
  Crypto::AES cbc128;
  cbc128.set_key(k128);
  cbc128.set_iv(iv);
  test(cbc128, "CBC 128", true);

  Crypto::AES cbc192;
  cbc192.set_key(k192);
  cbc192.set_iv(iv);
  test(cbc192, "CBC 192", true);

  Crypto::AES cbc256;
  cbc256.set_key(k256);
  cbc256.set_iv(iv);
  test(cbc256, "CBC 256", true);

  // CBC versions with residual block termination
  Crypto::AES cbc128_rtb;
  cbc128_rtb.set_key(k128);
  cbc128_rtb.set_iv(iv);
  test(cbc128_rtb, "CBC 128 RTB", true, true);

  Crypto::AES cbc192_rtb;
  cbc192_rtb.set_key(k192);
  cbc192_rtb.set_iv(iv);
  test(cbc192_rtb, "CBC 192 RTB", true, true);

  Crypto::AES cbc256_rtb;
  cbc256_rtb.set_key(k256);
  cbc256_rtb.set_iv(iv);
  test(cbc256_rtb, "CBC 256 RTB", true, true);

  // CTR version
  Crypto::AES ctr128;
  ctr128.set_key(k128);
  ctr128.set_iv(iv);
  ctr128.set_ctr(true);
  test(ctr128, "CTR 128", true);

  return 0;
}

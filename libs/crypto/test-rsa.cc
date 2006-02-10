//==========================================================================
// ObTools::Crypto: test-rsa.cc
//
// Test harness for Crypto library RSA encryption/decryption functions
//
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-crypto.h"
#include "ot-misc.h"
#include <iostream>

#define KEY_LEN 1024
#define MAX_LEN 256
#define PLAINTEXT_LEN 16
#define NUM_ENCRYPT_CYCLES 1
#define NUM_DECRYPT_CYCLES 500

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Test a RSA encryptor
void test(Crypto::RSA& encryptor, Crypto::RSA& decryptor, 
	  const string& what, int cypher_size, int plaintext_size)
{
  unsigned char data[MAX_LEN];
  unsigned char result[MAX_LEN];
  unsigned char cypher[MAX_LEN];
  Misc::Random random;
  random.generate_binary(data, plaintext_size);

  Misc::Dumper dumper(cout, 16, 4, false);
  cout << endl << what << " - original of " << plaintext_size << " bytes:\n";
  dumper.dump(data, plaintext_size);

  for(int i=0; i<NUM_ENCRYPT_CYCLES; i++)
  {
    if (!encryptor.encrypt(data, plaintext_size, cypher))
    {
      cerr << what << " - can't encrypt!\n";
      exit(2);
    }
  }

  cout << what << " - encrypted:\n";
  dumper.dump(cypher, cypher_size);

  int n;

  for(int i=0; i<NUM_DECRYPT_CYCLES; i++)
  {
    n = decryptor.decrypt(cypher, result);

    if (!n)
    {
      cerr << what << " - can't decrypt!\n";
      exit(2);
    }
  }

  cout << what << " - decrypted " << n << " bytes:\n";
  dumper.dump(result, n);

  if (memcmp(data, result, plaintext_size))
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
  Crypto::RSA rsa_pri(true);
  rsa_pri.key.create(KEY_LEN);
  cout << "Private key:\n" << rsa_pri.key << endl;

  Crypto::RSA rsa_pub(false);  
  rsa_pub.key.read(rsa_pri.key.str(), true); // Transfer over by text (ugh!)
  cout << "Public key:\n" << rsa_pub.key << endl;

  int cypher_size = rsa_pri.cypher_size();
  cout << "Cypher size: " << cypher_size << endl;

  int max_plaintext = rsa_pri.max_plaintext();
  cout << "Max plaintext: " << max_plaintext << endl;

  if (max_plaintext > PLAINTEXT_LEN) max_plaintext = PLAINTEXT_LEN;

  // Public encrypt, private decrypt
  test(rsa_pub, rsa_pri, "Public-to-Private", cypher_size, max_plaintext);

  // Private encrypt, public decrypt
  test(rsa_pri, rsa_pub, "Private-to-Public", cypher_size, max_plaintext);

  return 0;  
}





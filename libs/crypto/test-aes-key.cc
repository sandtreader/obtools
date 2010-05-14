//==========================================================================
// ObTools::Crypto: test-aes-key.cc
//
// Test harness for Crypto library AES key functions
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
// Main

int main()
{
  for (unsigned i = 0; i < 3; ++i)
  {
    Crypto::AESKey::Size size;
    switch (i)
    {
      case 0:
        size = Crypto::AESKey::BITS_128;
        break;
      case 1:
        size = Crypto::AESKey::BITS_192;
        break;
      case 2:
        size = Crypto::AESKey::BITS_256;
        break;
    }

    Crypto::AESKey k(size);
    k.create();

    if (k.valid)
    {
      cout << "Created " << size << " bit AES key: " << k << endl;
      string ks = k.str();
      if (ks.size() != static_cast<unsigned>(k.size / 4))
      {
        cerr << "Key output is only " << ks.size() << " characters!" << endl;
        return 2;
      }

      Crypto::AESKey k2(ks, size);
      if (k2.valid)
        cout << "Read back: " << k2 << endl;
      else
      {
        cerr << "Can't read back key" << endl;
        return 2;
      }

      if (k.str() == k2.str())
        cout << "Keys match" << endl;
      else
      {
        cerr << "Keys differ!" << endl;
        return 2;
      }
    }
    else
    {
      cerr << "Can't create valid " << size << " bit key" << endl;
      return 2;
    }
  }

  return 0;
}

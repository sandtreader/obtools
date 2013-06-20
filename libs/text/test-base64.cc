//==========================================================================
// ObTools::Text: test-base64.cc
//
// Test harness for text library base64 encode/decode
//
// Copyright (c) 2006 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-text.h"
#include <iostream>
#include <stdlib.h>

using namespace std;
using namespace ObTools;

#define BLOCK_SIZE 200

// Try encoding/decoding various numbers
void test(Text::Base64& base64, uint64_t n)
{
  string b64 = base64.encode(n);
  cout << "Base 64 of " << hex << n << dec << " = [" << b64 << "]";

  uint64_t n2;
  if (base64.decode(b64, n2))
  {
    cout << " => " << hex << n2 << dec << endl;
    if (n!=n2)
    {
      cerr << "NUMBERS DIFFER\n";
      exit(2);
    }
  }
  else
  {
    cerr << "\nCAN'T DECODE NUMBER\n";
    exit(2);
  }
}

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  string s;
  if (argc > 1) 
    s = argv[1];
  else
  {
    while (!cin.eof())
    {
      int c = cin.get();
      if (c>=0) s+=c;
    }
  }

  Text::Base64 base64;

  // Encode text
  string es = base64.encode(s);
  cout << "Base 64 of [" << s << "] (" << s.size() << " bytes):\n" 
       << es << endl;
  
  // Decode text
  size_t len = base64.binary_length(es);
  cout << "Decode will take up to " << len << " bytes\n";
  unsigned char *buf = new unsigned char[len+1]; // So we can add 0
  len = base64.decode(es, buf, len);
  buf[len] = 0;
  string ds(reinterpret_cast<char *>(buf));
  cout << "Decode gave " << len << " bytes:\n[" << ds << "]\n";

  if (s == ds)
  {
    cout << "Decode matches\n";
  }
  else
  {
    cout << "DECODE DIFFERS!\n";
    return 2;
  }

  // Encode various numbers
  test(base64, 0);
  test(base64, 0xDEADBEEFUL);
  test(base64, 0xFFFFFFFFFFFFFFFFULL);
  test(base64, 0xF000000000000000ULL);

  return 0;
}





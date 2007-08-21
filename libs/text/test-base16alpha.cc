//==========================================================================
// ObTools::Text: test-base16alpha.cc
//
// Test harness for text library base16 alpha encode/decode
//
// Copyright (c) 2007 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-text.h"
#include <iostream>
#include <sstream>

using namespace std;
using namespace ObTools;

// Try encoding/decoding various numbers
void test(uint64_t n)
{
  Text::Base16Alpha base16;
  string b16 = base16.encode(n);
  cout << "Base 16 of " << n << " = [" << b16 << "]";

  uint64_t n2;
  if (base16.decode(b16, n2))
  {
    cout << " => " << n2 << endl;
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
  {
    for(int i=1; i<argc; i++)
    {
      s += argv[i];
      s += ' ';
    }
  }
  else
  {
    while (!cin.eof())
    {
      int c = cin.get();
      if (c>=0) s+=c;
    }
  }

  // Pull out numbers
  istringstream iss(s);
  while (iss)
  {
    uint64_t n;
    iss >> n;
    if (iss) test(n);
  }

  return 0;
}





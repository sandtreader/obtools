//==========================================================================
// ObTools::Text: test-text.cc
//
// Test harness for text library pattern matching function
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-text.h"
using namespace std;

string text = "Alpha.Beta.Gamma";

//--------------------------------------------------------------------------
// Pattern test
void test(string pattern, bool cased=true)
{
  cout << "Pattern: " << pattern << (cased?" (cased)":" (uncased)") << endl;
  if (ObTools::Text::pattern_match(pattern, text, cased))
    cout << "  MATCH\n";
  else
    cout << "  NO\n";
}

//--------------------------------------------------------------------------
// Main

int main()
{
  cout << "Text is: [" << text << "]\n\n";

  test("Alpha*", true);
  test("alpha*", true);
  test("alpha*", false);

  test("[A-Z]*");
  test("[!A-Z]*");
  test("*a");
  test("*b");

  test("*B*");
  test("Alpha.????.Gamma");
  test("Alpha.*.Gamma");
  test("Alpha.???.Gamma");

  test("*");
  test("*.*");
  test("*.*.*");

  return 0;  
}





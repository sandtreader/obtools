//==========================================================================
// ObTools::Text: test-text.cc
//
// Test harness for text library whitespace functions
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-text.h"
#include <iostream>

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main

int main()
{
  string text = "    \n    \n  Indented 2\n  \n\n    Indented 6\n\n   ";

  cout << "Original:\n===\n" << text << "===\n";

  string temp = Text::canonicalise_space(text);
  cout << "Canonicalised: [" << temp << "]\n";
  cout << "First word is '" << Text::remove_word(temp) << "' leaving ["
       << temp << "]\n\n";

  cout << "Split words:\n";
  vector<string> l = Text::split_words(text);
  for(vector<string>::iterator p = l.begin(); p!=l.end(); ++p)
    cout << "  " << *p << endl;
  cout << endl;

  string stext = Text::strip_blank_lines(text);
  cout << "Stripped blank lines:\n===\n" << stext << "===\n";

  stext = Text::condense_blank_lines(text);
  cout << "Condensed blank lines:\n===\n" << stext << "===\n";

  int ci = Text::get_common_indent(stext);
  cout << "Common indent is: " << ci << endl;
  stext = Text::remove_indent(stext, ci);

  cout << "Removed common indent:\n===\n" << stext << "===\n";

  return 0;  
}





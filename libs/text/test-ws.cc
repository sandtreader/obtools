//==========================================================================
// ObTools::Text: test-text.cc
//
// Test harness for text library whitespace functions
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-text.h"
using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main

int main()
{
  string text = "    \n    \n  Indented 2\n      Indented 6\n\n   ";

  string temp = Text::canonicalise_space(text);
  cout << "Canonicalised: [" << temp << "]\n";
  cout << "First word is '" << Text::remove_word(temp) << "' leaving ["
       << temp << "]\n\n";

  text = Text::strip_blank_lines(text);
  int ci = Text::get_common_indent(text);
  cout << "Common indent is: " << ci << endl;
  text = Text::remove_indent(text, ci);

  cout << "===\n" << text << "===\n";

  return 0;  
}





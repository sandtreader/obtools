//==========================================================================
// ObTools::Text: test-text.cc
//
// Test harness for text library whitespace functions
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-text.h"

//--------------------------------------------------------------------------
// Main

int main()
{
  string text = "    \n    \n  Indented 2\n      Indented 6\n\n   ";

  text = ObTools::Text::strip_blank_lines(text);
  int ci = ObTools::Text::get_common_indent(text);
  cout << "Common indent is: " << ci << endl;
  text = ObTools::Text::remove_indent(text, ci);

  cout << "===\n" << text << "===\n";
  return 0;  
}





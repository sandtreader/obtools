//==========================================================================
// ObTools::Text: ws.cc
//
// Useful white-space handling functions not provided by standard library
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-text.h"
#include <ctype.h>

namespace ObTools { namespace Text {

//--------------------------------------------------------------------------
// Strip single leading and trailing blank lines from a string
// (e.g. XML artefacts)
string strip_blank_lines(const string& text)
{
  // Find first non-space character
  string::size_type start = text.find_first_not_of(" \t\n");
  if (start == string::npos) return "";  // Completely empty

  // Find first newline
  string::size_type nl = text.find_first_of("\n");
  if (nl == string::npos) return text; // No newlines at all

  // If newline first, first line is blank - move forward
  if (nl<start)
    start = nl+1;  // Move to next line
  else
    start = 0;  // First line not blank

  // Find last non-space character
  string::size_type end = text.find_last_not_of(" \t\n");
  if (end == string::npos || end<start) return "";  // (shouldn't happen)

  // Find last newline
  nl = text.find_last_of("\n");

  // If newline last, last line is blank - chop it
  if (nl>end)
    return text.substr(start, nl-start+1);
  else
    return text.substr(start); // Not blank, leave it
}

//--------------------------------------------------------------------------
// Find minimum leading whitespace (common indent) of a string
// Tabs are treated as 8 spaces
// Won't return more than 80
int get_common_indent(const string& text)
{
  int min = 80;
  int indent = 0;
  bool seen_text = false;
  
  // Run a simple state machine counting leading whitespace on lines
  for(string::const_iterator p = text.begin(); p!=text.end(); p++)
  {
    switch (*p)
    {
      case ' ':
	if (!seen_text) indent++;
      break;

      case '\t':
	if (!seen_text) indent+=8;
      break;

      case '\n':
	seen_text = false;
	indent = 0;
      break;

      default:  // Any non-whitespace
	if (!seen_text)
	{
	  if (indent < min) min = indent;
	  seen_text = true;
	}
      break;
    }
  }

  return min;
}

//--------------------------------------------------------------------------
// Remove indent from text, up to 'indent' spaces
string remove_indent(const string& text, int indent)
{
  string result;
  int pos=0;

  for(string::const_iterator p = text.begin(); p!=text.end(); p++)
  {
    char c=*p;
    switch (c)
    {
      case ' ':
	// Only add spaces if used up indent
	if (pos++ >= indent) result+=c;
      break;

      case '\t':
	// Only add tab if used up indent
	if (pos >= indent) result+=c;
	pos += 8;
      break;

      case '\n':
	pos = 0;
	result+=c;
      break;

      default:  // Any non-whitespace
	pos++;
	result+=c;
      break;
    }
  }

  return result;
}

}} // namespaces

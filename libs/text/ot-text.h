//==========================================================================
// ObTools::Text: ot-text.h
//
// Public definitions for ObTools::Text 
// Generally useful text/matching extensions to standard C++ library:
// 
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#ifndef __OBTOOLS_TEXT_H
#define __OBTOOLS_TEXT_H

#include <string>
#include <deque>
#include <list>
#include <map>
#include <iostream>

namespace ObTools { namespace Text { 

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Whitespace handling functions (ws.cc)

//--------------------------------------------------------------------------
// Strip single leading and trailing blank lines from a string
// (e.g. XML artefacts)
string strip_blank_lines(const string& text);

//--------------------------------------------------------------------------
// Find minimum leading whitespace (common indent) of a string
// Tabs are treated as 8 spaces
// Won't return more than 80
int get_common_indent(const string& text);

//--------------------------------------------------------------------------
// Remove indent from text, up to 'indent' spaces
string remove_indent(const string& text, int indent);

//==========================================================================
// Pattern matching functions (pattern.cc)
// NB - Unix 'glob' style patterns, not full regexp

//--------------------------------------------------------------------------
// Matches a pattern against a string
// Returns whether it matches
// Pattern can contain:
//	*	Matches any number of characters, or none
//	?	Matches a single character
//	[abc]	Matches any character in set. Ranges x-y allowed
//	[!abc]  Matches	any character not in set.  Ranges allowed
//	\	Escapes following character special character
//
//	   cased gives whether case sensitive match (true)
bool pattern_match(const char *pattern, const char *text, 
		   bool cased=true);

// More C++ friendly version
bool pattern_match(const string& pattern, const string& text, 
		   bool cased=true);

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_TEXT_H




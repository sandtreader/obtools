//==========================================================================
// ObTools::Text: ot-text.h
//
// Public definitions for ObTools::Text 
// Generally useful text/matching extensions to standard C++ library:
// 
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#ifndef __OBTOOLS_TEXT_H
#define __OBTOOLS_TEXT_H

#include <string>
#include <vector>

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

//--------------------------------------------------------------------------
// Canonicalise a multiword string:
//  Remove leading and trailing whitespace
//  Translate intervening whitespace strings into single space
string canonicalise_space(const string& text);

//--------------------------------------------------------------------------
// Split a string into first word and remaining
// Text must be canonical (see above)
// Returns first word, removes it and space from text
string remove_word(string& text);

//--------------------------------------------------------------------------
// Get list of words from text
// Text is canonicalised before splitting
vector<string> split_words(const string& text);

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
// Substitution functions (subst.cc)
// Global String replace - equivalent to s/old/rep/g
// Case sensitive
// Handles case where new string includes old
string subst(string text, const string& old, const string& rep);
 
//==========================================================================
// Case functions (case.cc)

//--------------------------------------------------------------------------
// Lower-case a string
string tolower(const string& text);

//--------------------------------------------------------------------------
// Upper-case a string
string toupper(const string& text);

//==========================================================================
// MD5 hash function (md5.cc)

//--------------------------------------------------------------------------
// MD5 sum a string (returns hex string)
string md5(const string& text);


//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_TEXT_H

















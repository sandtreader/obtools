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
#include <stdint.h>

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
// Conversion functions (convert.cc)

//--------------------------------------------------------------------------
// Integer to string
string itos(int i);

//--------------------------------------------------------------------------
// String to integer
int stoi(const string& s);

//--------------------------------------------------------------------------
// Float to string (0 default)
string ftos(double f);

//--------------------------------------------------------------------------
// String to float (0.0 default)
double stof(const string& s);

//==========================================================================
// Base64 encoder/decoder
class Base64
{
  char pad;            // Character to use for padding ('='), or 0 for none
  char extra_62;       // Character to use for index 62 ('+')
  char extra_63;       // Character to use for index 63 ('/')

public:
  //--------------------------------------------------------------------------
  // Constructor
  Base64(char _pad='=', char _extra_62='+', char _extra_63='/'):
    pad(_pad), extra_62(_extra_62), extra_63(_extra_63) {}

  //--------------------------------------------------------------------------
  // Encode a binary block
  // Split gives length of line to split at - default (76) is to RFC
  // Set 0 to suppress split altogether
  string encode(const unsigned char *block, size_t length, int split=76);

  //--------------------------------------------------------------------------
  // Encode a 64-bit integer, top byte first (big-endian)
  // Will reduce size to 4 bytes if it fits
  string encode(uint64_t n);

  //--------------------------------------------------------------------------
  // Get length of binary block required for decode 
  // This is a maximum estimate - real length may be less than this, but
  // will never be more
  size_t binary_length(const string& base64);

  //--------------------------------------------------------------------------
  // Decode a base64 string into a binary block.  
  // Returns real length decoded if it fitted, max_length+1 if it didn't
  // - but it will never actually write more than max_length bytes
  size_t decode(const string& base64, unsigned char *block, size_t max_length);

  //--------------------------------------------------------------------------
  // Decode a 64-bit integer, top byte first (big-endian)
  // Returns whether successful - if so, sets 'n'
  bool decode(const string& base64, uint64_t& n);
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_TEXT_H

















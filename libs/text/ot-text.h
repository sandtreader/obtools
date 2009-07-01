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
// Compress blank lines from a string
// Removes _all_ blank lines at start and end of text (not just the first/last)
// and also condenses multiple blank lines in a single one.
// Horizontal whitespace is left alone
// This is particularly useful to tidy the output of an XML::Expander...
string condense_blank_lines(const string& text);

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
// Split a string into first line and remaining
// Returns first line (without newline), removes it and newline from text
string remove_line(string& text);

//--------------------------------------------------------------------------
// Get list of words from text
// Text is canonicalised before splitting
vector<string> split_words(const string& text);

//==========================================================================
// Record split functions (split.cc)

//--------------------------------------------------------------------------
// Split a string into fields using the given delimiter
// Canonicalises fields (removed leading and trailing whitespace, folds
// multiple internal whitespace into one) if canonicalise is set
// If max is set, stops after 'max-1' fields have been read, and drops the
// rest of the string into the last one
vector<string> split(const string& text, char delim=',', 
		     bool canonicalise = true, int max=0);

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
// String to integer (0 default)
int stoi(const string& s);

//--------------------------------------------------------------------------
// 64-bit integer to string
string i64tos(uint64_t i);

//--------------------------------------------------------------------------
// String to 64-bit integer (0 default)
uint64_t stoi64(const string& s);

//--------------------------------------------------------------------------
// Float to string, with zero padding
string ftos(double f, int width=0, int prec=0, bool zero_pad=false);

//--------------------------------------------------------------------------
// String to float (0.0 default)
double stof(const string& s);

//--------------------------------------------------------------------------
// Integer to hex
string itox(unsigned int i);

//--------------------------------------------------------------------------
// Hex to integer (0 default)
unsigned int xtoi(const string& s);

//--------------------------------------------------------------------------
// 64-bit integer to hex
string i64tox(uint64_t i);

//--------------------------------------------------------------------------
// Hex to 64-bit integer (0 default)
uint64_t xtoi64(const string& s);

//--------------------------------------------------------------------------
// Binary to hex (simple, use Misc::Dumper for long blocks)
string btox(const unsigned char *data, unsigned int length);

//--------------------------------------------------------------------------
// Hex to binary
// Reads up to max_length bytes into data, returns number actually read
unsigned int xtob(const string& hex, unsigned char *data, 
		  unsigned int max_length);

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
  // line_end is string to split with, and indent for next line
  string encode(const unsigned char *block, size_t length, 
		int split=76, const string& line_end = "\r\n");

  //--------------------------------------------------------------------------
  // Encode a 64-bit integer, top byte first (big-endian)
  // Will reduce size to 4 bytes if it fits
  string encode(uint64_t n);

  //--------------------------------------------------------------------------
  // Encode a binary string - options as encode above
  string encode(const string& binary, int split=76, 
		const string& line_end="\r\n");

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

  //--------------------------------------------------------------------------
  // Decode base64 text into the given (binary) string
  // Returns whether successful - if so, appends data to binary
  // Requires temporary buffer equal to the binary_length() of the string
  bool decode(const string& base64, string& binary);

};

//==========================================================================
// Base36 encoder/decoder
class Base36
{
public:
  //--------------------------------------------------------------------------
  // Default constructor 
  Base36() {}

  //--------------------------------------------------------------------------
  // Encode a 64-bit integer 
  // Uses as many characters as required
  static string encode(uint64_t n);

  //--------------------------------------------------------------------------
  // Decode a 64-bit integer
  // Returns whether successful - if so, sets 'n'
  static bool decode(const string& base36, uint64_t& n);
};

//==========================================================================
// Base16 'safe alphabet' encoder/decoder
// Alphabet is "bcdg hjkl mpqr svwz" - no vowels, and also missing "fntxy"
class Base16Alpha
{
public:
  //--------------------------------------------------------------------------
  // Default constructor 
  Base16Alpha() {}

  //--------------------------------------------------------------------------
  // Encode a 64-bit integer 
  // Uses as many characters as required
  static string encode(uint64_t n);

  //--------------------------------------------------------------------------
  // Decode a 64-bit integer
  // Returns whether successful - if so, sets 'n'
  static bool decode(const string& base16, uint64_t& n);
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_TEXT_H

















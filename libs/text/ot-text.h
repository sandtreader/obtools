//==========================================================================
// ObTools::Text: ot-text.h
//
// Public definitions for ObTools::Text
// Generally useful text/matching extensions to standard C++ library:
//
// Copyright (c) 2003-2015 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_TEXT_H
#define __OBTOOLS_TEXT_H

#include <string>
#include <vector>
#include <stdint.h>
#include "ot-gen.h"

namespace ObTools { namespace Text {

// Make our lives easier without polluting anyone else
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
// Remove all whitespace from a string
string remove_space(const string& text);

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

//--------------------------------------------------------------------------
// Split text into lines
// Newlines are removed
// Must be Unix (LF only) format
vector<string> split_lines(const string& text, bool remove_blanks = false);

//==========================================================================
// Record split functions (split.cc)

//--------------------------------------------------------------------------
// Split a string into fields using the given delimiter
// Canonicalises fields (removed leading and trailing whitespace, folds
// multiple internal whitespace into one) if canonicalise is set
// If max is set, stops after 'max-1' fields have been read, and drops the
// rest of the string into the last one
// If there are no delimiters, the whole string (even if empty) goes into
// the first result element
vector<string> split(const string& text, char delim=',',
                     bool canonicalise = true, int max=0);

//==========================================================================
// Pattern matching functions (pattern.cc)
// NB - Unix 'glob' style patterns, not full regexp

//--------------------------------------------------------------------------
// Matches a pattern against a string
// Returns whether it matches
// Pattern can contain:
//        *        Matches any number of characters, or none
//        ?        Matches a single character
//        [abc]        Matches any character in set. Ranges x-y allowed
//        [!abc]  Matches        any character not in set.  Ranges allowed
//        \        Escapes following character special character
//
//           cased gives whether case sensitive match (true)
// Adds any strings (including empty) matched by '*' into the given vector
bool pattern_match(const char *pattern, const char *text,
                   vector<string>& matches, bool cased=true);

// More C++ friendly version, with matches
bool pattern_match(const string& pattern, const string& text,
                   vector<string>& matches, bool cased=true);

// More C++ friendly version, without matches
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
// Integer representing a fixed point to string
string ifixtos(int i, int decimal_places = 0);

//--------------------------------------------------------------------------
// String to integer representing a fixed point (assumes correctness)
int stoifix(const string& s, int decimal_places = 0);

//--------------------------------------------------------------------------
// Float to string, with zero padding
string ftos(double f, int width=0, int prec=0, bool zero_pad=false);

//--------------------------------------------------------------------------
// String to float (0.0 default)
double stof(const string& s);

//--------------------------------------------------------------------------
// String to boolean (false default)
// Accepts [TtYy1]* as true
bool stob(const string& s);

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
string btox(const string& data);
string btox(const vector<byte>& data);

//--------------------------------------------------------------------------
// Hex to binary
// Reads up to max_length bytes into data, returns number actually read
unsigned int xtob(const string& hex, unsigned char *data,
                  unsigned int max_length);

//--------------------------------------------------------------------------
// Hex string to binary string
string xtob(const string& hex);

//==========================================================================
// Base64 encoder/decoder
class Base64
{
  char pad;            // Character to use for padding ('='), or 0 for none
  char extra_62;       // Character to use for index 62 ('+')
  char extra_63;       // Character to use for index 63 ('/')

public:
  //------------------------------------------------------------------------
  // Constructor
  Base64(char _pad='=', char _extra_62='+', char _extra_63='/'):
    pad(_pad), extra_62(_extra_62), extra_63(_extra_63) {}

  //------------------------------------------------------------------------
  // Encode a binary block
  // Split gives length of line to split at - default (76) is to RFC
  // Set 0 to suppress split altogether
  // line_end is string to split with, and indent for next line
  string encode(const unsigned char *block, size_t length,
                int split=76, const string& line_end = "\r\n");

  //------------------------------------------------------------------------
  // Encode a vector of bytes
  string encode(const vector<unsigned char>& data, int split = 76,
                const string& line_end = "\r\n")
  {
    return encode(&data[0], data.size(), split, line_end);
  }

  //------------------------------------------------------------------------
  // Encode a 64-bit integer, top byte first (big-endian)
  // Will reduce size to 4 bytes if it fits
  string encode(uint64_t n);

  //------------------------------------------------------------------------
  // Encode a binary string - options as encode above
  string encode(const string& binary, int split=76,
                const string& line_end="\r\n");

  //------------------------------------------------------------------------
  // Get length of binary block required for decode
  // This is a maximum estimate - real length may be less than this, but
  // will never be more
  size_t binary_length(const string& base64);

  //------------------------------------------------------------------------
  // Decode a base64 string into a binary block.
  // Returns real length decoded if it fitted, max_length+1 if it didn't
  // - but it will never actually write more than max_length bytes
  size_t decode(const string& base64, unsigned char *block, size_t max_length);

  //------------------------------------------------------------------------
  // Decode a 64-bit integer, top byte first (big-endian)
  // Returns whether successful - if so, sets 'n'
  bool decode(const string& base64, uint64_t& n);

  //------------------------------------------------------------------------
  // Decode base64 text into the given (binary) string
  // Returns whether successful - if so, appends data to binary
  // Requires temporary buffer equal to the binary_length() of the string
  bool decode(const string& base64, string& binary);

};

//==========================================================================
// Base64URL encoder/decoder - special version with URL safe characters
class Base64URL: public Base64
{
public:
  // Constructor
  Base64URL(): Base64(0, '-', '_') {}

  // Encode with no line splits
  string encode(const string& s) { return Base64::encode(s, 0); }
};

//==========================================================================
// Base36 encoder/decoder
class Base36
{
public:
  //------------------------------------------------------------------------
  // Default constructor
  Base36() {}

  //------------------------------------------------------------------------
  // Encode a 64-bit integer
  // Uses as many characters as required
  static string encode(uint64_t n);

  //------------------------------------------------------------------------
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
  //------------------------------------------------------------------------
  // Default constructor
  Base16Alpha() {}

  //------------------------------------------------------------------------
  // Encode a 64-bit integer
  // Uses as many characters as required
  static string encode(uint64_t n);

  //------------------------------------------------------------------------
  // Decode a 64-bit integer
  // Returns whether successful - if so, sets 'n'
  static bool decode(const string& base16, uint64_t& n);
};

//==========================================================================
// UTF8 encoder/decoder
class UTF8
{
public:
  //------------------------------------------------------------------------
  // Default constructor
  UTF8() {}

  //------------------------------------------------------------------------
  // Encode a single unicode character and append to a string
  static void append(string& utf8, wchar_t unicode);

  //------------------------------------------------------------------------
  // Encode a vector of unicode characters to a UTF8 string
  static string encode(const vector<wchar_t>& unicode);

  //------------------------------------------------------------------------
  // Encode a null terminated string of unicode characters to a UTF8 string
  static string encode(const wchar_t *unicode);

  //--------------------------------------------------------------------------
  // Encode an ISO-Latin1 8-bit string into a UTF8 string
  static string encode(const string& isolatin1);

  //------------------------------------------------------------------------
  // Decode a UTF8 string into a wide char vector
  static void decode(const string& utf8, vector<wchar_t>& unicode);

  // Note: Decode to ISO-Latin1 isn't safe!

  //------------------------------------------------------------------------
  // Squash diacritics (accents) from a UTF8 string
  // Only works in ISO-Latin1 range, replacing with approximate ASCII base
  // character.  Any other non-ASCII printable characters are replaced with
  // the fallback character given
  static string strip_diacritics(const string& utf8, char fallback='_');
};

//==========================================================================
// CSV reader
class CSV
{
  char sep;            // Separator character (',' by default)

public:
  //------------------------------------------------------------------------
  // Constructor
  CSV(char _sep=','): sep(_sep) {}

  //--------------------------------------------------------------------------
  // Read a line of CSV into the given vars
  // Won't fail, will try to fix up errors
  void read_line(const string& line, vector<string>& vars);

  //--------------------------------------------------------------------------
  // Read multiline CSV into a vector of vector of vars
  // Won't fail, will try to fix up errors
  void read(const string& text, vector<vector<string> >& data,
            bool skip_header = false);
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_TEXT_H

















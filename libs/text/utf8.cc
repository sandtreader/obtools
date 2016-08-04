//==========================================================================
// ObTools::Text: utf8.cc
//
// UTF8 encoding
//
// Copyright (c) 2011 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-text.h"

namespace ObTools { namespace Text {

void UTF8::append(string& utf8, wchar_t unicode)
{
  if (unicode < 0x00000080)
  {
    utf8 += (unicode & 0x7f);
  }
  else if (unicode < 0x00000800)
  {
    utf8 += (0xc0 | (0x1f & (unicode >> 6)));
    utf8 += (0x80 | (0x3f & unicode));
  }
  else if (unicode < 0x00010000)
  {
    utf8 += (0xe0 | (0x0f & (unicode >> 12)));
    utf8 += (0x80 | (0x3f & (unicode >> 6)));
    utf8 += (0x80 | (0x3f & unicode));
  }
  else if (unicode < 0x00200000)
  {
    utf8 += (0xf0 | (0x07 & (unicode >> 18)));
    utf8 += (0x80 | (0x3f & (unicode >> 12)));
    utf8 += (0x80 | (0x3f & (unicode >> 6)));
    utf8 += (0x80 | (0x3f & unicode));
  }
  else if (unicode < 0x04000000)
  {
    utf8 += (0xf8 | (0x03 & (unicode >> 24)));
    utf8 += (0x80 | (0x3f & (unicode >> 18)));
    utf8 += (0x80 | (0x3f & (unicode >> 12)));
    utf8 += (0x80 | (0x3f & (unicode >> 6)));
    utf8 += (0x80 | (0x3f & unicode));
  }
  else if (unicode <= 0x7fffffff)
  {
    utf8 += (0xfc | (0x01 & (unicode >> 30)));
    utf8 += (0x80 | (0x3f & (unicode >> 24)));
    utf8 += (0x80 | (0x3f & (unicode >> 18)));
    utf8 += (0x80 | (0x3f & (unicode >> 12)));
    utf8 += (0x80 | (0x3f & (unicode >> 6)));
    utf8 += (0x80 | (0x3f & unicode));
  }
}

//--------------------------------------------------------------------------
// Encode a wide char vector to a UTF8 string
string UTF8::encode(const vector<wchar_t>& unicode)
{
  string utf8;
  for (vector<wchar_t>::const_iterator it = unicode.begin();
       it != unicode.end(); ++it)
    append(utf8, *it);
  return utf8;
}

//--------------------------------------------------------------------------
// Decode a UTF8 string into a wide char vector
void UTF8::decode(const string& utf8, vector<wchar_t>& unicode)
{
  for(string::const_iterator p=utf8.begin(); p!=utf8.end(); ++p)
  {
    wchar_t c = static_cast<wchar_t>(*p);
    int extra = 0;
    wchar_t mask = 0xff;

    // Work out how many extra characters follow, and mask for first
    // Note extra is cumulative and mask keeps shifting the deeper we get
    if ((c & 0xC0) == 0xC0) { extra++; mask=0x1f; }
    if ((c & 0xE0) == 0xE0) { extra++; mask>>=1; }
    if ((c & 0xF0) == 0xF0) { extra++; mask>>=1; }
    if ((c & 0xF8) == 0xF8) { extra++; mask>>=1; }
    if ((c & 0xFC) == 0xFC) { extra++; mask>>=1; }

    // First character
    c &= mask;

    // Extras, 6 bits each
    for(int i=0; i<extra && p!=utf8.end(); ++i)
      c = (c<<6) | (*++p & 0x3f);

    unicode.push_back(c);
  }
}

//--------------------------------------------------------------------------
// Squash diacritics (accents) from a UTF8 string
// Only works in ISO-Latin1 range, replacing with approximate ASCII base
// character, or digraph
// Any other non-ASCII printable characters are replaced with
// the fallback character given
string UTF8::strip_diacritics(const string& utf8, char fallback)
{
  const char *latin1_mapping =
    "AAAAAAECEEEEIIIIDNOOOOOxOUUUUYPsaaaaaaeceeeeiiiidnooooo/ouuuuypy";
 // "ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ"

  vector<wchar_t> unicode;
  decode(utf8, unicode);

  vector<wchar_t> unicode_stripped;
  for (vector<wchar_t>::const_iterator it = unicode.begin();
       it != unicode.end(); ++it)
  {
    wchar_t c = *it;
    if (c >= 0xC0 && c<=0xFF) // Latin 1 accent range
    {
      switch (c)
      {
        // Special cases giving multiple characters
        case 0xC6: // AE
          unicode_stripped.push_back(L'A');
          c = L'E';
          break;

        case 0xD0: // ETH
        case 0xDE: // THORN
          unicode_stripped.push_back(L'T');
          c = L'H';
          break;

        case 0xDF: // Sharp S
          unicode_stripped.push_back(L's');
          c = L's';
          break;

        case 0xE6: // ae
          unicode_stripped.push_back(L'a');
          c = L'e';
          break;

        case 0xF0: // eth
        case 0xFE: // thorn
          unicode_stripped.push_back(L't');
          c = L'h';
          break;

        default:
          c = latin1_mapping[c-0xC0];
      }
    }
    else if (c > 127)
      c = (wchar_t)fallback;

    unicode_stripped.push_back(c);
  }

  return encode(unicode_stripped);
}


}} // namespaces

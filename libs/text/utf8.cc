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
    utf8 += (0xe0 | (0xf0 & (unicode >> 12)));
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

//------------------------------------------------------------------------
// Encode a wide char vector to a UTF8 string
string UTF8::encode(const vector<wchar_t>& unicode)
{
  string utf8;
  for (vector<wchar_t>::const_iterator it = unicode.begin();
       it != unicode.end(); ++it)
    append(utf8, *it);
  return utf8;
}

}} // namespaces

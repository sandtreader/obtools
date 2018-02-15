//==========================================================================
// ObTools::Text: test-utf8.cc
//
// GTest harness for text library UTF8 functions
//
// Copyright (c) 2016 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-text.h"
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

TEST(UTF8Test, TestUTF8Encode)
{
  vector<wchar_t> unicode;
  unicode.push_back(1);
  unicode.push_back(32);
  unicode.push_back(0x7f);
  unicode.push_back(0x80);
  unicode.push_back(0xff);
  unicode.push_back(0x100);
  unicode.push_back(0x7ff);
  unicode.push_back(0x800);
  unicode.push_back(0xffff);
  unicode.push_back(0x10000);
  unicode.push_back(0x3ffffff);
  unicode.push_back(0x4000000);
  unicode.push_back(0x7fffffff);

  string utf8 = Text::UTF8::encode(unicode);
  // Test encoding from http://www.ltg.ed.ac.uk/~richard/utf-8.cgi
  ASSERT_EQ("\x01 \x7F\xC2\x80\xC3\xBF\xC4\x80\xDF\xBF\xE0\xA0\x80\xEF\xBF\xBF\xF0\x90\x80\x80\xFB\xBF\xBF\xBF\xBF\xFC\x84\x80\x80\x80\x80\xFD\xBF\xBF\xBF\xBF\xBF", utf8);
}

TEST(UTF8Test, TestUTF8EncodeFromISOLatin1)
{
  string latin1("\x01 \x7f\x80\xff");
  string utf8 = Text::UTF8::encode(latin1);
  // Test encoding from http://www.ltg.ed.ac.uk/~richard/utf-8.cgi (latin1 only)
  ASSERT_EQ("\x01 \x7F\xC2\x80\xC3\xBF", utf8);
}

TEST(UTF8Test, TestUTF8Decode)
{
  string utf8 = "\x01 \x7F\xC2\x80\xC3\xBF\xC4\x80\xDF\xBF\xE0\xA0\x80\xEF\xBF\xBF\xF0\x90\x80\x80\xFB\xBF\xBF\xBF\xBF\xFC\x84\x80\x80\x80\x80\xFD\xBF\xBF\xBF\xBF\xBF";
  vector<wchar_t> unicode;
  Text::UTF8::decode(utf8, unicode);
  ASSERT_EQ(13, unicode.size());
  EXPECT_EQ(1,          unicode[0]);
  EXPECT_EQ(32,         unicode[1]);
  EXPECT_EQ(0x7f,       unicode[2]);
  EXPECT_EQ(0x80,       unicode[3]);
  EXPECT_EQ(0xff,       unicode[4]);
  EXPECT_EQ(0x100,      unicode[5]);
  EXPECT_EQ(0x7ff,      unicode[6]);
  EXPECT_EQ(0x800,      unicode[7]);
  EXPECT_EQ(0xffff,     unicode[8]);
  EXPECT_EQ(0x10000,    unicode[9]);
  EXPECT_EQ(0x3ffffff,  unicode[10]);
  EXPECT_EQ(0x4000000,  unicode[11]);
  EXPECT_EQ(0x7fffffff, unicode[12]);
}

TEST(UTF8Test, TestUTF8StripDiacritics)
{
  vector<wchar_t> unicode;

  // All Latin1 printables
  for(wchar_t i=32; i<=255; i++)
    if (i!=127) unicode.push_back(i);  // Skip DEL

  string utf8 = Text::UTF8::encode(unicode);
  string stripped = Text::UTF8::strip_diacritics(utf8);
  ASSERT_EQ(" !\"#$%&'()*+,-./0123456789:;<=>?"
            "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
            "`abcdefghijklmnopqrstuvwxyz{|}~"
            "________________________________"
            "________________________________"
            "AAAAAAAECEEEEIIIITHNOOOOOxOUUUUYTHss"
            "aaaaaaaeceeeeiiiithnooooo/ouuuuythy",
            stripped);
}

TEST(UTF8Test, TestUTF8StripDiacriticsFallback)
{
  vector<wchar_t> unicode;
  unicode.push_back(0x3456);  // God knows

  string utf8 = Text::UTF8::encode(unicode);
  string stripped = Text::UTF8::strip_diacritics(utf8, '?');
  ASSERT_EQ("?", stripped);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

//==========================================================================
// ObTools::Lang: test-lang.cc
//
// Test harness for ISO language code library
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-lang.h"
#include <sstream>
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// iso_639_to_lang: dispatch by code length

TEST(LangTest, TestISO639TwoCharCode)
{
  EXPECT_EQ(Lang::Language::english, Lang::iso_639_to_lang("en"));
  EXPECT_EQ(Lang::Language::french, Lang::iso_639_to_lang("fr"));
  EXPECT_EQ(Lang::Language::german, Lang::iso_639_to_lang("de"));
}

TEST(LangTest, TestISO639ThreeCharCode)
{
  EXPECT_EQ(Lang::Language::english, Lang::iso_639_to_lang("eng"));
  EXPECT_EQ(Lang::Language::french, Lang::iso_639_to_lang("fre"));
  EXPECT_EQ(Lang::Language::german, Lang::iso_639_to_lang("ger"));
}

TEST(LangTest, TestISO639InvalidLength)
{
  EXPECT_EQ(Lang::Language::unknown, Lang::iso_639_to_lang(""));
  EXPECT_EQ(Lang::Language::unknown, Lang::iso_639_to_lang("e"));
  EXPECT_EQ(Lang::Language::unknown, Lang::iso_639_to_lang("engl"));
  EXPECT_EQ(Lang::Language::unknown, Lang::iso_639_to_lang("english"));
}

//--------------------------------------------------------------------------
// iso_639_1_to_lang: 2-character codes

TEST(LangTest, TestISO6391CommonLanguages)
{
  EXPECT_EQ(Lang::Language::english, Lang::iso_639_1_to_lang("en"));
  EXPECT_EQ(Lang::Language::french, Lang::iso_639_1_to_lang("fr"));
  EXPECT_EQ(Lang::Language::spanish, Lang::iso_639_1_to_lang("es"));
  EXPECT_EQ(Lang::Language::chinese, Lang::iso_639_1_to_lang("zh"));
  EXPECT_EQ(Lang::Language::japanese, Lang::iso_639_1_to_lang("ja"));
  EXPECT_EQ(Lang::Language::russian, Lang::iso_639_1_to_lang("ru"));
  EXPECT_EQ(Lang::Language::arabic, Lang::iso_639_1_to_lang("ar"));
  EXPECT_EQ(Lang::Language::hindi, Lang::iso_639_1_to_lang("hi"));
  EXPECT_EQ(Lang::Language::portuguese, Lang::iso_639_1_to_lang("pt"));
  EXPECT_EQ(Lang::Language::korean, Lang::iso_639_1_to_lang("ko"));
}

TEST(LangTest, TestISO6391InvalidCode)
{
  EXPECT_EQ(Lang::Language::unknown, Lang::iso_639_1_to_lang("xx"));
  EXPECT_EQ(Lang::Language::unknown, Lang::iso_639_1_to_lang("00"));
}

//--------------------------------------------------------------------------
// lang_to_iso_639_1: reverse mapping

TEST(LangTest, TestLangToISO6391)
{
  EXPECT_EQ("en", Lang::lang_to_iso_639_1(Lang::Language::english));
  EXPECT_EQ("fr", Lang::lang_to_iso_639_1(Lang::Language::french));
  EXPECT_EQ("de", Lang::lang_to_iso_639_1(Lang::Language::german));
  EXPECT_EQ("es", Lang::lang_to_iso_639_1(Lang::Language::spanish));
  EXPECT_EQ("zh", Lang::lang_to_iso_639_1(Lang::Language::chinese));
  EXPECT_EQ("ja", Lang::lang_to_iso_639_1(Lang::Language::japanese));
}

TEST(LangTest, TestLangToISO6391NoMapping)
{
  // Language::unknown has no 639-1 code
  EXPECT_EQ("", Lang::lang_to_iso_639_1(Lang::Language::unknown));
}

//--------------------------------------------------------------------------
// iso_639_2_to_lang: 3-character codes

TEST(LangTest, TestISO6392CommonLanguages)
{
  EXPECT_EQ(Lang::Language::english, Lang::iso_639_2_to_lang("eng"));
  EXPECT_EQ(Lang::Language::french, Lang::iso_639_2_to_lang("fre"));
  EXPECT_EQ(Lang::Language::french, Lang::iso_639_2_to_lang("fra"));
  EXPECT_EQ(Lang::Language::german, Lang::iso_639_2_to_lang("ger"));
  EXPECT_EQ(Lang::Language::german, Lang::iso_639_2_to_lang("deu"));
  EXPECT_EQ(Lang::Language::chinese, Lang::iso_639_2_to_lang("chi"));
  EXPECT_EQ(Lang::Language::chinese, Lang::iso_639_2_to_lang("zho"));
  EXPECT_EQ(Lang::Language::japanese, Lang::iso_639_2_to_lang("jpn"));
}

TEST(LangTest, TestISO6392BTFormAliases)
{
  // Both B and T forms should map to same language
  EXPECT_EQ(Lang::Language::albanian, Lang::iso_639_2_to_lang("alb"));
  EXPECT_EQ(Lang::Language::albanian, Lang::iso_639_2_to_lang("sqi"));
  EXPECT_EQ(Lang::Language::dutch, Lang::iso_639_2_to_lang("dut"));
  EXPECT_EQ(Lang::Language::dutch, Lang::iso_639_2_to_lang("nld"));
  EXPECT_EQ(Lang::Language::basque, Lang::iso_639_2_to_lang("baq"));
  EXPECT_EQ(Lang::Language::basque, Lang::iso_639_2_to_lang("eus"));
}

TEST(LangTest, TestISO6392InvalidCode)
{
  EXPECT_EQ(Lang::Language::unknown, Lang::iso_639_2_to_lang("xxx"));
  EXPECT_EQ(Lang::Language::unknown, Lang::iso_639_2_to_lang("aaa"));
}

//--------------------------------------------------------------------------
// lang_to_iso_639_2: reverse mapping

TEST(LangTest, TestLangToISO6392)
{
  EXPECT_EQ("eng", Lang::lang_to_iso_639_2(Lang::Language::english));
  EXPECT_EQ("jpn", Lang::lang_to_iso_639_2(Lang::Language::japanese));
  EXPECT_EQ("kor", Lang::lang_to_iso_639_2(Lang::Language::korean));
  EXPECT_EQ("ara", Lang::lang_to_iso_639_2(Lang::Language::arabic));
}

TEST(LangTest, TestLangToISO6392NoMapping)
{
  EXPECT_EQ("", Lang::lang_to_iso_639_2(Lang::Language::unknown));
}

//--------------------------------------------------------------------------
// Roundtrip conversions

TEST(LangTest, TestRoundtripISO6391)
{
  // en -> english -> en
  auto lang = Lang::iso_639_1_to_lang("en");
  EXPECT_EQ("en", Lang::lang_to_iso_639_1(lang));

  lang = Lang::iso_639_1_to_lang("fr");
  EXPECT_EQ("fr", Lang::lang_to_iso_639_1(lang));
}

TEST(LangTest, TestRoundtripISO6392)
{
  // eng -> english -> eng
  auto lang = Lang::iso_639_2_to_lang("eng");
  EXPECT_EQ("eng", Lang::lang_to_iso_639_2(lang));

  lang = Lang::iso_639_2_to_lang("jpn");
  EXPECT_EQ("jpn", Lang::lang_to_iso_639_2(lang));
}

//--------------------------------------------------------------------------
// lang_to_string

TEST(LangTest, TestLangToString)
{
  EXPECT_EQ("English", Lang::lang_to_string(Lang::Language::english));
  EXPECT_EQ("French", Lang::lang_to_string(Lang::Language::french));
  EXPECT_EQ("German", Lang::lang_to_string(Lang::Language::german));
  EXPECT_EQ("Spanish; Castilian",
            Lang::lang_to_string(Lang::Language::spanish));
  EXPECT_EQ("Chinese", Lang::lang_to_string(Lang::Language::chinese));
  EXPECT_EQ("Japanese", Lang::lang_to_string(Lang::Language::japanese));
}

TEST(LangTest, TestLangToStringSpecialCodes)
{
  EXPECT_EQ("Multiple languages",
            Lang::lang_to_string(Lang::Language::multiple_languages));
  EXPECT_EQ("No linguistic content; Not applicable",
            Lang::lang_to_string(Lang::Language::no_linguistic_content));
  EXPECT_EQ("Undetermined",
            Lang::lang_to_string(Lang::Language::undetermined));
}

TEST(LangTest, TestLangToStringUnknown)
{
  EXPECT_EQ("", Lang::lang_to_string(Lang::Language::unknown));
}

TEST(LangTest, TestLangToStringLanguageGroups)
{
  EXPECT_NE("", Lang::lang_to_string(Lang::Language::afro_asiatic_languages));
  EXPECT_NE("", Lang::lang_to_string(Lang::Language::bantu_languages));
  EXPECT_NE("", Lang::lang_to_string(Lang::Language::germanic_languages));
}

TEST(LangTest, TestLangToStringConstructedLanguages)
{
  EXPECT_EQ("Esperanto", Lang::lang_to_string(Lang::Language::esperanto));
  EXPECT_EQ("Klingon; tlhIngan-Hol",
            Lang::lang_to_string(Lang::Language::klingon));
}

//--------------------------------------------------------------------------
// operator<<

TEST(LangTest, TestStreamOperator)
{
  ostringstream oss;
  oss << Lang::Language::english;
  EXPECT_EQ("English", oss.str());
}

TEST(LangTest, TestStreamOperatorChaining)
{
  ostringstream oss;
  oss << Lang::Language::french << " and " << Lang::Language::german;
  EXPECT_EQ("French and German", oss.str());
}

//--------------------------------------------------------------------------
// Various language families for broad coverage

TEST(LangTest, TestISO6392VariousLanguages)
{
  EXPECT_EQ(Lang::Language::esperanto, Lang::iso_639_2_to_lang("epo"));
  EXPECT_EQ(Lang::Language::latin, Lang::iso_639_2_to_lang("lat"));
  EXPECT_EQ(Lang::Language::sanskrit, Lang::iso_639_2_to_lang("san"));
  EXPECT_EQ(Lang::Language::swahili, Lang::iso_639_2_to_lang("swa"));
  EXPECT_EQ(Lang::Language::welsh, Lang::iso_639_2_to_lang("wel"));
  EXPECT_EQ(Lang::Language::zulu, Lang::iso_639_2_to_lang("zul"));
  EXPECT_EQ(Lang::Language::irish, Lang::iso_639_2_to_lang("gle"));
  EXPECT_EQ(Lang::Language::hawaiian, Lang::iso_639_2_to_lang("haw"));
  EXPECT_EQ(Lang::Language::navajo, Lang::iso_639_2_to_lang("nav"));
  EXPECT_EQ(Lang::Language::tibetan, Lang::iso_639_2_to_lang("tib"));
}

TEST(LangTest, TestISO6391VariousLanguages)
{
  EXPECT_EQ(Lang::Language::italian, Lang::iso_639_1_to_lang("it"));
  EXPECT_EQ(Lang::Language::swedish, Lang::iso_639_1_to_lang("sv"));
  EXPECT_EQ(Lang::Language::polish, Lang::iso_639_1_to_lang("pl"));
  EXPECT_EQ(Lang::Language::turkish, Lang::iso_639_1_to_lang("tr"));
  EXPECT_EQ(Lang::Language::thai, Lang::iso_639_1_to_lang("th"));
  EXPECT_EQ(Lang::Language::vietnamese, Lang::iso_639_1_to_lang("vi"));
  EXPECT_EQ(Lang::Language::finnish, Lang::iso_639_1_to_lang("fi"));
  EXPECT_EQ(Lang::Language::czech, Lang::iso_639_1_to_lang("cs"));
  EXPECT_EQ(Lang::Language::danish, Lang::iso_639_1_to_lang("da"));
  EXPECT_EQ(Lang::Language::norwegian, Lang::iso_639_1_to_lang("no"));
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

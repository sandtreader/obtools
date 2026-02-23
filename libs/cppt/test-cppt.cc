//==========================================================================
// ObTools::CPPT: test-cppt.cc
//
// Test harness for C++ template processor library
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-cppt.h"
#include <sstream>
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

//==========================================================================
// TokenRecogniser tests

TEST(TokenRecogniserTest, TestAddTokenAndMatchSingle)
{
  CPPT::TokenRecogniser tr;
  tr.add_token("ab");
  CPPT::TokenState state;

  // First char: partial match
  EXPECT_TRUE(tr.process_char('a', state));
  EXPECT_EQ(CPPT::TOKEN_READING, state);

  // Second char: completes token (only one possibility, so immediately valid)
  EXPECT_TRUE(tr.process_char('b', state));
  EXPECT_EQ(CPPT::TOKEN_VALID, state);
  EXPECT_EQ("ab", tr.get_token());
}

TEST(TokenRecogniserTest, TestNoMatch)
{
  CPPT::TokenRecogniser tr;
  tr.add_token("ab");
  CPPT::TokenState state;

  // Non-matching char
  EXPECT_FALSE(tr.process_char('x', state));
  EXPECT_EQ(CPPT::TOKEN_READING, state);
}

TEST(TokenRecogniserTest, TestEmptyTokenIgnored)
{
  CPPT::TokenRecogniser tr;
  tr.add_token("");
  tr.add_token("a");
  CPPT::TokenState state;

  // Should match 'a' - only one possibility, so immediately valid
  EXPECT_TRUE(tr.process_char('a', state));
  EXPECT_EQ(CPPT::TOKEN_VALID, state);
  EXPECT_EQ("a", tr.get_token());
}

TEST(TokenRecogniserTest, TestOverlappingTokens)
{
  CPPT::TokenRecogniser tr;
  tr.add_token("<?");
  tr.add_token("<?=");
  CPPT::TokenState state;

  // Process '<'
  EXPECT_TRUE(tr.process_char('<', state));
  EXPECT_EQ(CPPT::TOKEN_READING, state);

  // Process '?'
  EXPECT_TRUE(tr.process_char('?', state));
  EXPECT_EQ(CPPT::TOKEN_READING, state);

  // Process '=' - matches "<?=" (only one possibility left, so immediately valid)
  EXPECT_TRUE(tr.process_char('=', state));
  EXPECT_EQ(CPPT::TOKEN_VALID, state);
  EXPECT_EQ("<?=", tr.get_token());
}

TEST(TokenRecogniserTest, TestOverlappingTokensShorterMatch)
{
  CPPT::TokenRecogniser tr;
  tr.add_token("<?");
  tr.add_token("<?=");
  CPPT::TokenState state;

  // Process '<'
  EXPECT_TRUE(tr.process_char('<', state));
  // Process '?'
  EXPECT_TRUE(tr.process_char('?', state));
  // Process 'a' - not '=' so "<?=" doesn't match, but "<?" should be valid
  EXPECT_FALSE(tr.process_char('a', state));
  EXPECT_EQ(CPPT::TOKEN_VALID, state);
  EXPECT_EQ("<?", tr.get_token());
}

TEST(TokenRecogniserTest, TestInvalidToken)
{
  CPPT::TokenRecogniser tr;
  tr.add_token("abc");
  CPPT::TokenState state;

  // Match first char
  EXPECT_TRUE(tr.process_char('a', state));
  // Match second char
  EXPECT_TRUE(tr.process_char('b', state));
  // Mismatch - not 'c', and "ab" is not a valid token
  EXPECT_FALSE(tr.process_char('x', state));
  EXPECT_EQ(CPPT::TOKEN_INVALID, state);
  EXPECT_EQ("ab", tr.get_token());
}

//==========================================================================
// Processor tests - full template processing

static CPPT::Tags standard_tags()
{
  return CPPT::Tags{"<?", "?>", "<?=", "?>", "<?#", "?>"};
}

TEST(ProcessorTest, TestPlainText)
{
  istringstream in("Hello World");
  ostringstream out;
  CPPT::Processor proc(in, out, standard_tags());
  proc.process();
  string result = out.str();
  EXPECT_NE(string::npos, result.find("cout << \"Hello World\""));
}

TEST(ProcessorTest, TestPlainTextWithNewline)
{
  istringstream in("Hello\nWorld\n");
  ostringstream out;
  CPPT::Processor proc(in, out, standard_tags());
  proc.process();
  string result = out.str();
  EXPECT_NE(string::npos, result.find("Hello\\n\""));
  EXPECT_NE(string::npos, result.find("World\\n\""));
}

TEST(ProcessorTest, TestCodeBlock)
{
  istringstream in("<? int x = 42; ?>\n");
  ostringstream out;
  CPPT::Processor proc(in, out, standard_tags());
  proc.process();
  string result = out.str();
  EXPECT_NE(string::npos, result.find("int x = 42;"));
  // Should not contain cout for the code block
  EXPECT_EQ(string::npos, result.find("cout << \"int x"));
}

TEST(ProcessorTest, TestExpressionBlock)
{
  istringstream in("Value: <?= x ?>");
  ostringstream out;
  CPPT::Processor proc(in, out, standard_tags());
  proc.process();
  string result = out.str();
  EXPECT_NE(string::npos, result.find("cout << \"Value: \" << ( x )"));
}

TEST(ProcessorTest, TestCommentBlock)
{
  istringstream in("Before<?# this is a comment ?>\nAfter\n");
  ostringstream out;
  CPPT::Processor proc(in, out, standard_tags());
  proc.process();
  string result = out.str();
  // Comment content should not appear
  EXPECT_EQ(string::npos, result.find("this is a comment"));
  EXPECT_NE(string::npos, result.find("Before"));
  EXPECT_NE(string::npos, result.find("After"));
}

TEST(ProcessorTest, TestQuoteEscaping)
{
  istringstream in("Say \"Hi\"");
  ostringstream out;
  CPPT::Processor proc(in, out, standard_tags());
  proc.process();
  string result = out.str();
  EXPECT_NE(string::npos, result.find("\\\"Hi\\\""));
}

TEST(ProcessorTest, TestBackslashEscaping)
{
  istringstream in("C:\\path");
  ostringstream out;
  CPPT::Processor proc(in, out, standard_tags());
  proc.process();
  string result = out.str();
  EXPECT_NE(string::npos, result.find("C:\\\\path"));
}

TEST(ProcessorTest, TestCustomStreamName)
{
  istringstream in("Hello");
  ostringstream out;
  CPPT::Processor proc(in, out, standard_tags(), "mystream");
  proc.process();
  string result = out.str();
  EXPECT_NE(string::npos, result.find("mystream << \"Hello\""));
  EXPECT_EQ(string::npos, result.find("cout << \"Hello\""));
}

TEST(ProcessorTest, TestMixedCodeAndText)
{
  istringstream in("<? if (true) { ?>\nYes\n<? } ?>\n");
  ostringstream out;
  CPPT::Processor proc(in, out, standard_tags());
  proc.process();
  string result = out.str();
  EXPECT_NE(string::npos, result.find("if (true) {"));
  EXPECT_NE(string::npos, result.find("Yes"));
}

TEST(ProcessorTest, TestExpressionInMiddleOfText)
{
  istringstream in("x=<?= val ?>!");
  ostringstream out;
  CPPT::Processor proc(in, out, standard_tags());
  proc.process();
  string result = out.str();
  EXPECT_NE(string::npos, result.find("x="));
  EXPECT_NE(string::npos, result.find("val"));
}

TEST(ProcessorTest, TestEmptyInput)
{
  istringstream in("");
  ostringstream out;
  CPPT::Processor proc(in, out, standard_tags());
  proc.process();
  EXPECT_EQ("", out.str());
}

TEST(ProcessorTest, TestNewlineOnlyText)
{
  istringstream in("\n");
  ostringstream out;
  CPPT::Processor proc(in, out, standard_tags());
  proc.process();
  string result = out.str();
  EXPECT_NE(string::npos, result.find("endl"));
}

TEST(ProcessorTest, TestMultipleExpressions)
{
  istringstream in("<?= a ?>+<?= b ?>");
  ostringstream out;
  CPPT::Processor proc(in, out, standard_tags());
  proc.process();
  string result = out.str();
  EXPECT_NE(string::npos, result.find("( a )"));
  EXPECT_NE(string::npos, result.find("( b )"));
}

TEST(ProcessorTest, TestCustomTags)
{
  CPPT::Tags custom_tags{"[%", "%]", "[%=", "%]", "[%#", "%]"};
  istringstream in("Hello [%= name %]!");
  ostringstream out;
  CPPT::Processor proc(in, out, custom_tags);
  proc.process();
  string result = out.str();
  EXPECT_NE(string::npos, result.find("name"));
  EXPECT_NE(string::npos, result.find("Hello"));
}

TEST(ProcessorTest, TestCodeBlockWithMultipleLines)
{
  istringstream in("<? int x = 1;\nint y = 2; ?>\n");
  ostringstream out;
  CPPT::Processor proc(in, out, standard_tags());
  proc.process();
  string result = out.str();
  EXPECT_NE(string::npos, result.find("int x = 1;"));
  EXPECT_NE(string::npos, result.find("int y = 2;"));
}

//==========================================================================
// TOKEN_INVALID path tests
// These trigger partial tag matches that fail, producing TOKEN_INVALID

TEST(ProcessorTest, TestNormalInvalidToken)
{
  // "<" starts matching "<?" but "x" doesn't match "?" -> TOKEN_INVALID
  // The "<" should be output as text
  istringstream in("<x");
  ostringstream out;
  CPPT::Processor proc(in, out, standard_tags());
  proc.process();
  string result = out.str();
  EXPECT_NE(string::npos, result.find("<x"));
}

TEST(ProcessorTest, TestCodeInvalidToken)
{
  // Inside code block, "?" starts matching "?>" but "x" fails -> TOKEN_INVALID
  // The "?" should pass through as code
  istringstream in("<? x ?x ?>\n");
  ostringstream out;
  CPPT::Processor proc(in, out, standard_tags());
  proc.process();
  string result = out.str();
  EXPECT_NE(string::npos, result.find("x ?x"));
}

TEST(ProcessorTest, TestExprInvalidToken)
{
  // Inside expr block, "?" starts matching "?>" but "x" fails -> TOKEN_INVALID
  istringstream in("<?= a?b ?>");
  ostringstream out;
  CPPT::Processor proc(in, out, standard_tags());
  proc.process();
  string result = out.str();
  EXPECT_NE(string::npos, result.find("a?b"));
}

TEST(ProcessorTest, TestCommentInvalidToken)
{
  // Inside comment block, "?" starts matching "?>" but "x" fails
  // The invalid token is swallowed (comment content is discarded)
  istringstream in("<?# a ?x stuff ?>\nAfter\n");
  ostringstream out;
  CPPT::Processor proc(in, out, standard_tags());
  proc.process();
  string result = out.str();
  EXPECT_EQ(string::npos, result.find("stuff"));
  EXPECT_NE(string::npos, result.find("After"));
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

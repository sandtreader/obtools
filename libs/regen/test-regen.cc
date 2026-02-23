//==========================================================================
// ObTools::ReGen: test-regen.cc
//
// Test harness for code regeneration library
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-regen.h"
#include <sstream>
#include <fstream>
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

//==========================================================================
// MarkedFile tests

TEST(MarkedFileTest, TestReadLine)
{
  istringstream in("line1\nline2\nline3\n");
  ReGen::MarkedFile mf(in);
  EXPECT_TRUE(mf.read_line());
  EXPECT_EQ("line1", mf.line_text());
  EXPECT_TRUE(mf.read_line());
  EXPECT_EQ("line2", mf.line_text());
  EXPECT_TRUE(mf.read_line());
  EXPECT_EQ("line3", mf.line_text());
  EXPECT_FALSE(mf.read_line());
}

TEST(MarkedFileTest, TestReadLineEmpty)
{
  istringstream in("");
  ReGen::MarkedFile mf(in);
  EXPECT_FALSE(mf.read_line());
}

TEST(MarkedFileTest, TestLineTypeNormal)
{
  istringstream in("normal line\n");
  ReGen::MarkedFile mf(in);
  mf.read_line();
  EXPECT_EQ(ReGen::LINE_NORMAL, mf.line_type());
}

TEST(MarkedFileTest, TestLineTypeOpen)
{
  istringstream in("//~[tagname\n");
  ReGen::MarkedFile mf(in);
  mf.read_line();
  EXPECT_EQ(ReGen::LINE_OPEN, mf.line_type());
}

TEST(MarkedFileTest, TestLineTypeClose)
{
  istringstream in("//~]\n");
  ReGen::MarkedFile mf(in);
  mf.read_line();
  EXPECT_EQ(ReGen::LINE_CLOSE, mf.line_type());
}

TEST(MarkedFileTest, TestLineTypeUserStart)
{
  istringstream in("//~^ user section\n");
  ReGen::MarkedFile mf(in);
  mf.read_line();
  EXPECT_EQ(ReGen::LINE_USER_START, mf.line_type());
}

TEST(MarkedFileTest, TestLineTypeUserEnd)
{
  istringstream in("//~v end user\n");
  ReGen::MarkedFile mf(in);
  mf.read_line();
  EXPECT_EQ(ReGen::LINE_USER_END, mf.line_type());
}

TEST(MarkedFileTest, TestLineTypeMarkerOnly)
{
  // Marker with no following character
  istringstream in("//~\n");
  ReGen::MarkedFile mf(in);
  mf.read_line();
  EXPECT_EQ(ReGen::LINE_NORMAL, mf.line_type());
}

TEST(MarkedFileTest, TestLineTypeUnrecognisedMarker)
{
  istringstream in("//~x something\n");
  ReGen::MarkedFile mf(in);
  mf.read_line();
  EXPECT_EQ(ReGen::LINE_NORMAL, mf.line_type());
}

TEST(MarkedFileTest, TestLineTag)
{
  istringstream in("//~[ myblock \n");
  ReGen::MarkedFile mf(in);
  mf.read_line();
  EXPECT_EQ("myblock", mf.line_tag());
}

TEST(MarkedFileTest, TestLineTagNoTag)
{
  istringstream in("normal line\n");
  ReGen::MarkedFile mf(in);
  mf.read_line();
  EXPECT_EQ("", mf.line_tag());
}

TEST(MarkedFileTest, TestLineTagNoText)
{
  // Marker + open char but nothing after
  istringstream in("//~[\n");
  ReGen::MarkedFile mf(in);
  mf.read_line();
  EXPECT_EQ("", mf.line_tag());
}

TEST(MarkedFileTest, TestLineTagWhitespaceOnly)
{
  istringstream in("//~[   \n");
  ReGen::MarkedFile mf(in);
  mf.read_line();
  EXPECT_EQ("", mf.line_tag());
}

TEST(MarkedFileTest, TestCustomMarker)
{
  // Marker must be 3 chars (line_type hardcodes pos += 3)
  istringstream in("##~[tag\n");
  ReGen::MarkedFile mf(in, "##~");
  mf.read_line();
  EXPECT_EQ(ReGen::LINE_OPEN, mf.line_type());
}

//==========================================================================
// Block tests

TEST(BlockTest, TestAddLineAndDump)
{
  ReGen::Block block;
  block.add_line(ReGen::LINE_OPEN, "//~[test");
  block.add_line(ReGen::LINE_NORMAL, "generated code");
  block.add_line(ReGen::LINE_CLOSE, "//~]");

  ostringstream oss;
  block.dump(oss);
  string result = oss.str();
  EXPECT_NE(string::npos, result.find("//~[test"));
  EXPECT_NE(string::npos, result.find("generated code"));
  EXPECT_NE(string::npos, result.find("//~]"));
}

TEST(BlockTest, TestDumpWithUserSection)
{
  ReGen::Block block;
  block.add_line(ReGen::LINE_OPEN, "//~[test");
  block.add_line(ReGen::LINE_NORMAL, "gen");
  block.add_line(ReGen::LINE_USER_START, "//~^");
  block.add_line(ReGen::LINE_NORMAL, "user content");
  block.add_line(ReGen::LINE_USER_END, "//~v");
  block.add_line(ReGen::LINE_CLOSE, "//~]");

  ostringstream oss;
  block.dump(oss);
  string result = oss.str();
  // User section should have "-" prefix
  EXPECT_NE(string::npos, result.find("-user content"));
}

//==========================================================================
// MasterFile tests

TEST(MasterFileTest, TestParseMaster)
{
  istringstream master(
    "//~[block1\n"
    "generated line 1\n"
    "//~]\n"
    "//~[block2\n"
    "generated line 2\n"
    "//~]\n"
  );
  ReGen::MasterFile mf(master);

  ostringstream oss;
  mf.dump(oss);
  string result = oss.str();
  EXPECT_NE(string::npos, result.find("block1"));
  EXPECT_NE(string::npos, result.find("generated line 1"));
  EXPECT_NE(string::npos, result.find("block2"));
  EXPECT_NE(string::npos, result.find("generated line 2"));
}

TEST(MasterFileTest, TestMergeSimple)
{
  // Master has updated content
  istringstream master(
    "//~[block1\n"
    "new generated line\n"
    "//~]\n"
  );
  ReGen::MasterFile mf(master);

  // User file with same block but different generated content
  istringstream user(
    "//~[block1\n"
    "old generated line\n"
    "//~]\n"
  );
  ReGen::MarkedFile ufile(user);

  ostringstream out;
  mf.merge(ufile, out);
  string result = out.str();

  // Master's content should win
  EXPECT_NE(string::npos, result.find("new generated line"));
  EXPECT_EQ(string::npos, result.find("old generated line"));
}

TEST(MasterFileTest, TestMergePreservesUserSection)
{
  istringstream master(
    "//~[block1\n"
    "generated\n"
    "//~^\n"
    "default user text\n"
    "//~v\n"
    "more generated\n"
    "//~]\n"
  );
  ReGen::MasterFile mf(master);

  istringstream user(
    "//~[block1\n"
    "old generated\n"
    "//~^\n"
    "MY CUSTOM CODE\n"
    "//~v\n"
    "old more generated\n"
    "//~]\n"
  );
  ReGen::MarkedFile ufile(user);

  ostringstream out;
  mf.merge(ufile, out);
  string result = out.str();

  // User's custom code should be preserved
  EXPECT_NE(string::npos, result.find("MY CUSTOM CODE"));
  // Master's generated parts should be used
  EXPECT_NE(string::npos, result.find("generated"));
  EXPECT_NE(string::npos, result.find("more generated"));
}

TEST(MasterFileTest, TestMergeOrphanBlockKept)
{
  // Master has no blocks
  istringstream master("");
  ReGen::MasterFile mf(master);

  // User file has an orphan block
  istringstream user(
    "//~[orphan\n"
    "orphan content\n"
    "//~]\n"
  );
  ReGen::MarkedFile ufile(user);

  ostringstream out;
  mf.merge(ufile, out);
  string result = out.str();

  // Orphan should be kept by default
  EXPECT_NE(string::npos, result.find("orphan content"));
}

TEST(MasterFileTest, TestMergeDeleteOrphans)
{
  istringstream master("");
  ReGen::MasterFile mf(master);

  istringstream user(
    "//~[orphan\n"
    "orphan content\n"
    "//~]\n"
  );
  ReGen::MarkedFile ufile(user);

  ostringstream out;
  mf.merge(ufile, out, ReGen::MERGE_DELETE_ORPHANS);
  string result = out.str();

  // Orphan should be deleted
  EXPECT_EQ(string::npos, result.find("orphan content"));
}

TEST(MasterFileTest, TestMergeNewBlockAppended)
{
  // Master has a new block
  istringstream master(
    "//~[newblock\n"
    "new content\n"
    "//~]\n"
  );
  ReGen::MasterFile mf(master);

  // User file has no blocks
  istringstream user("preamble\n");
  ReGen::MarkedFile ufile(user);

  ostringstream out;
  mf.merge(ufile, out);
  string result = out.str();

  // New block should be appended
  EXPECT_NE(string::npos, result.find("preamble"));
  EXPECT_NE(string::npos, result.find("new content"));
}

TEST(MasterFileTest, TestMergeSuppressNew)
{
  istringstream master(
    "//~[newblock\n"
    "new content\n"
    "//~]\n"
  );
  ReGen::MasterFile mf(master);

  istringstream user("preamble\n");
  ReGen::MarkedFile ufile(user);

  ostringstream out;
  mf.merge(ufile, out, ReGen::MERGE_SUPPRESS_NEW);
  string result = out.str();

  // New block should be suppressed
  EXPECT_NE(string::npos, result.find("preamble"));
  EXPECT_EQ(string::npos, result.find("new content"));
}

TEST(MasterFileTest, TestMergeNormalLinesOutsideBlocks)
{
  istringstream master(
    "//~[block1\n"
    "gen\n"
    "//~]\n"
  );
  ReGen::MasterFile mf(master);

  istringstream user(
    "before block\n"
    "//~[block1\n"
    "old gen\n"
    "//~]\n"
    "after block\n"
  );
  ReGen::MarkedFile ufile(user);

  ostringstream out;
  mf.merge(ufile, out);
  string result = out.str();

  EXPECT_NE(string::npos, result.find("before block"));
  EXPECT_NE(string::npos, result.find("after block"));
  EXPECT_NE(string::npos, result.find("gen"));
}

TEST(MasterFileTest, TestMergeMultipleUserSections)
{
  istringstream master(
    "//~[block1\n"
    "gen1\n"
    "//~^\n"
    "default1\n"
    "//~v\n"
    "gen2\n"
    "//~^\n"
    "default2\n"
    "//~v\n"
    "gen3\n"
    "//~]\n"
  );
  ReGen::MasterFile mf(master);

  istringstream user(
    "//~[block1\n"
    "old gen1\n"
    "//~^\n"
    "USER SECTION 1\n"
    "//~v\n"
    "old gen2\n"
    "//~^\n"
    "USER SECTION 2\n"
    "//~v\n"
    "old gen3\n"
    "//~]\n"
  );
  ReGen::MarkedFile ufile(user);

  ostringstream out;
  mf.merge(ufile, out);
  string result = out.str();

  // Both user sections preserved
  EXPECT_NE(string::npos, result.find("USER SECTION 1"));
  EXPECT_NE(string::npos, result.find("USER SECTION 2"));
  // Master generated sections used
  EXPECT_NE(string::npos, result.find("gen1"));
  EXPECT_NE(string::npos, result.find("gen2"));
  EXPECT_NE(string::npos, result.find("gen3"));
}

//==========================================================================
// regenbuf / rofstream tests (filesystem-based)

static const string test_dir = "/tmp/obtools-test-regen";

class RegenStreamTest: public ::testing::Test
{
protected:
  void SetUp() override
  {
    system(("rm -rf " + test_dir).c_str());
    system(("mkdir -p " + test_dir).c_str());
  }
  void TearDown() override
  {
    system(("rm -rf " + test_dir).c_str());
  }
};

TEST_F(RegenStreamTest, TestRofstreamNewFile)
{
  string fn = test_dir + "/output.cc";
  {
    ReGen::rofstream out(fn);
    out << "//~[block1\n"
        << "generated code\n"
        << "//~]\n";
  }

  ifstream result(fn);
  ASSERT_TRUE(result.good());
  string content((istreambuf_iterator<char>(result)),
                  istreambuf_iterator<char>());
  EXPECT_NE(string::npos, content.find("generated code"));
}

TEST_F(RegenStreamTest, TestRofstreamMerge)
{
  string fn = test_dir + "/output.cc";

  // Create initial file with user modifications
  {
    ofstream initial(fn);
    initial << "//~[block1\n"
            << "old generated\n"
            << "//~^\n"
            << "MY USER CODE\n"
            << "//~v\n"
            << "//~]\n";
  }

  // Regenerate - master has updated generated code
  {
    ReGen::rofstream out(fn);
    out << "//~[block1\n"
        << "NEW generated\n"
        << "//~^\n"
        << "default user code\n"
        << "//~v\n"
        << "//~]\n";
  }

  ifstream result(fn);
  ASSERT_TRUE(result.good());
  string content((istreambuf_iterator<char>(result)),
                  istreambuf_iterator<char>());

  // User code should be preserved
  EXPECT_NE(string::npos, content.find("MY USER CODE"));
  // New generated code should be used
  EXPECT_NE(string::npos, content.find("NEW generated"));
  // Old generated code should be replaced
  EXPECT_EQ(string::npos, content.find("old generated"));
}

TEST_F(RegenStreamTest, TestRofstreamDoubleClose)
{
  string fn = test_dir + "/output.cc";
  ReGen::rofstream out(fn);
  out << "test\n";
  out.close();
  out.close(); // should be idempotent
}

//==========================================================================
// Test merge when user has deleted a user section from a block
// This exercises the LINE_CLOSE "remaining master lines" path

TEST(MasterFileTest, TestMergeUserDeletedCutout)
{
  // Master has a user cutout
  istringstream master(
    "//~[block1\n"
    "gen1\n"
    "//~^\n"
    "default user text\n"
    "//~v\n"
    "gen2\n"
    "//~]\n"
  );
  ReGen::MasterFile mf(master);

  // User file has the block but removed the user section markers
  // Just has OPEN and CLOSE with some content in between
  istringstream user(
    "//~[block1\n"
    "user replaced everything\n"
    "//~]\n"
  );
  ReGen::MarkedFile ufile(user);

  ostringstream out;
  mf.merge(ufile, out);
  string result = out.str();

  // Master generated content should appear
  EXPECT_NE(string::npos, result.find("gen1"));
  // The remaining lines after the suppressed section should be output
  EXPECT_NE(string::npos, result.find("gen2"));
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

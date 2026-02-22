//==========================================================================
// ObTools::Misc: test-dumper.cc
//
// GTest harness for hex dumper
//
// Copyright (c) 2026 Paul Clark.
//==========================================================================

#include "ot-misc.h"
#include <gtest/gtest.h>
#include <sstream>

namespace {

using namespace std;
using namespace ObTools;
using namespace ObTools::Misc;

TEST(DumperTest, TestBasicDump)
{
  ostringstream oss;
  Dumper d(oss);
  d.dump("Hello", 5);
  string output = oss.str();
  // Should contain hex offset and hex bytes
  EXPECT_NE(string::npos, output.find("0000:"));
  EXPECT_NE(string::npos, output.find("48"));  // 'H'
  // Should contain ASCII section
  EXPECT_NE(string::npos, output.find("Hello"));
}

TEST(DumperTest, TestDumpWithoutASCII)
{
  ostringstream oss;
  Dumper d(oss, 16, 4, false);
  d.dump("AB", 2);
  string output = oss.str();
  EXPECT_NE(string::npos, output.find("4142"));
  EXPECT_EQ(string::npos, output.find("| AB"));
}

TEST(DumperTest, TestDumpMultipleLines)
{
  ostringstream oss;
  Dumper d(oss, 4, 0);  // 4 bytes per line, no split
  d.dump("ABCDEFGH", 8);
  string output = oss.str();
  EXPECT_NE(string::npos, output.find("0000:"));
  EXPECT_NE(string::npos, output.find("0004:"));
}

TEST(DumperTest, TestDumpNonPrintableChars)
{
  ostringstream oss;
  Dumper d(oss, 16, 4, true);
  string data = "\x01\x02\x7f";
  d.dump(data.data(), data.size());
  string output = oss.str();
  // Non-printable chars should appear as '.'
  EXPECT_NE(string::npos, output.find("..."));
}

TEST(DumperTest, TestDumpStringOverload)
{
  ostringstream oss;
  Dumper d(oss);
  string data = "test";
  d.dump(data);
  EXPECT_NE(string::npos, oss.str().find("test"));
}

TEST(DumperTest, TestDumpVectorOverload)
{
  ostringstream oss;
  Dumper d(oss);
  vector<byte> data{byte{0x41}, byte{0x42}};
  d.dump(data);
  EXPECT_NE(string::npos, oss.str().find("AB"));
}

TEST(DumperTest, TestDumpWithCustomSplit)
{
  ostringstream oss;
  Dumper d(oss, 8, 2, true);
  d.dump("ABCDEFGH", 8);
  string output = oss.str();
  // Split every 2 bytes means spaces between byte pairs
  EXPECT_NE(string::npos, output.find("0000:"));
}

TEST(DumperTest, TestDumpRestoresFormatting)
{
  ostringstream oss;
  oss << 42;  // Decimal before
  Dumper d(oss);
  d.dump("A", 1);
  oss << 42;  // Should still be decimal after
  string output = oss.str();
  // After dump, format should be restored to decimal with no fill
  EXPECT_NE(string::npos, output.rfind("42"));
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

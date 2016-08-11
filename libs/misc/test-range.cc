//==========================================================================
// ObTools::Text: test-range.cc
//
// Test harness for Misc library RangeSet
//
// Copyright (c) 2006 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-misc.h"
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

TEST(RangeSetTest, TestInsert)
{
  Misc::UInt64RangeSet rs(5);
  ASSERT_TRUE(rs.begin() == rs.end());
  rs.insert(10, 2);
  ASSERT_EQ(1, rs.count());
  rs.insert(14, 4);
  ASSERT_EQ(2, rs.count());
  rs.insert(16, 5);
  ASSERT_EQ(2, rs.count());
  rs.insert(9, 2);
  ASSERT_EQ(2, rs.count());

  set<Misc::UInt64RangeSet::Range> expected;
  expected.insert(Misc::UInt64RangeSet::Range(9, 3));
  expected.insert(Misc::UInt64RangeSet::Range(14, 7));

  ASSERT_EQ(expected, rs.ranges);
}

TEST(RangeSetTest, TestInsertSquashesMultipleOverlaps)
{
  Misc::UInt64RangeSet rs(5);
  ASSERT_TRUE(rs.begin() == rs.end());
  rs.insert(10, 2);
  ASSERT_EQ(1, rs.count()) << rs;
  rs.insert(16, 5);
  ASSERT_EQ(2, rs.count()) << rs;
  rs.insert(0, 30);
  ASSERT_EQ(1, rs.count()) << rs;

  set<Misc::UInt64RangeSet::Range> expected;
  expected.insert(Misc::UInt64RangeSet::Range(0, 30));

  ASSERT_EQ(expected, rs.ranges);
}

TEST(RangeSetTest, TestInsertCoalescesAdjacent)
{
  Misc::UInt64RangeSet rs(5);
  ASSERT_TRUE(rs.begin() == rs.end());
  rs.insert(10, 2);
  ASSERT_EQ(1, rs.count()) << rs;
  rs.insert(16, 5);
  ASSERT_EQ(2, rs.count()) << rs;
  rs.insert(12, 4);
  ASSERT_EQ(1, rs.count()) << rs;

  set<Misc::UInt64RangeSet::Range> expected;
  expected.insert(Misc::UInt64RangeSet::Range(10, 11));

  ASSERT_EQ(expected, rs.ranges);
}

TEST(RangeSetTest, TestRemove)
{
  Misc::UInt64RangeSet rs(5);
  ASSERT_TRUE(rs.begin() == rs.end());
  rs.insert(10, 2);
  ASSERT_EQ(1, rs.count());
  rs.insert(16, 5);
  ASSERT_EQ(2, rs.count());
  rs.remove(9, 10);
  ASSERT_EQ(1, rs.count());
  rs.insert(9, 2);
  rs.remove(10, 10);

  set<Misc::UInt64RangeSet::Range> expected;
  expected.insert(Misc::UInt64RangeSet::Range(9, 1));
  expected.insert(Misc::UInt64RangeSet::Range(20, 1));

  ASSERT_EQ(expected, rs.ranges);
}

TEST(RangeSetTest, TestInverse)
{
  Misc::UInt64RangeSet rs(5);
  ASSERT_TRUE(rs.begin() == rs.end());
  rs.insert(10, 2);
  ASSERT_EQ(1, rs.count());
  rs.insert(16, 5);
  ASSERT_EQ(2, rs.count());
  rs = rs.inverse();

  set<Misc::UInt64RangeSet::Range> expected;
  expected.insert(Misc::UInt64RangeSet::Range(0, 10));
  expected.insert(Misc::UInt64RangeSet::Range(12, 4));

  ASSERT_EQ(expected, rs.ranges);
}

TEST(RangeSetTest, TestIntersection)
{
  Misc::UInt64RangeSet rs(5);
  ASSERT_TRUE(rs.begin() == rs.end());
  rs.insert(10, 2);
  ASSERT_EQ(1, rs.count());
  rs.insert(16, 5);
  ASSERT_EQ(2, rs.count());

  Misc::UInt64RangeSet rs2(5);
  ASSERT_TRUE(rs2.begin() == rs2.end());
  rs2.insert(9, 2);
  ASSERT_EQ(1, rs2.count());
  rs2.insert(13, 10);
  ASSERT_EQ(2, rs2.count());

  rs = rs.intersection(rs2);

  set<Misc::UInt64RangeSet::Range> expected;
  expected.insert(Misc::UInt64RangeSet::Range(10, 1));
  expected.insert(Misc::UInt64RangeSet::Range(16, 5));

  ASSERT_EQ(expected, rs.ranges);
}

TEST(RangeSetTest, TestMultiRangeSetIntersection)
{
  Misc::RangeSet<double, double> rs1(5.0);
  ASSERT_TRUE(rs1.begin() == rs1.end());
  rs1.insert(10.0, 2.0);
  ASSERT_EQ(1, rs1.count());
  rs1.insert(16.0, 5.0);
  ASSERT_EQ(2, rs1.count());

  Misc::RangeSet<double, double> rs2(5.0);
  ASSERT_TRUE(rs2.begin() == rs2.end());
  rs2.insert(9.0, 2.0);
  ASSERT_EQ(1, rs2.count());
  rs2.insert(13.0, 10.0);
  ASSERT_EQ(2, rs2.count());

  Misc::RangeSet<double, double> rs3(5.0);
  ASSERT_TRUE(rs3.begin() == rs3.end());
  rs3.insert(5.0, 12.0);
  ASSERT_EQ(1, rs3.count());

  list<Misc::RangeSet<double, double> > rs_list;
  rs_list.push_back(rs1);
  rs_list.push_back(rs2);
  rs_list.push_back(rs3);

  Misc::RangeSet<double, double>
         actual = Misc::RangeSet<double, double>::intersection(rs_list);

  set<Misc::RangeSet<double, double>::Range> expected;
  expected.insert(Misc::RangeSet<double, double>::Range(10.0, 1.0));
  expected.insert(Misc::RangeSet<double, double>::Range(16.0, 1.0));

  ASSERT_EQ(expected, actual.ranges);
}

TEST(RangeSetTest, TestContains)
{
  Misc::UInt64RangeSet rs(5);
  ASSERT_TRUE(rs.begin() == rs.end());
  rs.insert(10, 2);
  ASSERT_EQ(1, rs.count());
  rs.insert(16, 5);
  ASSERT_EQ(2, rs.count());

  ASSERT_TRUE(rs.contains(18, 2));
  ASSERT_FALSE(rs.contains(3, 4));
  ASSERT_FALSE(rs.contains(10, 3));
}

TEST(RangeSetTest, TestCoverage)
{
  Misc::UInt64RangeSet rs(5);
  ASSERT_TRUE(rs.begin() == rs.end());
  rs.insert(10, 2);
  ASSERT_EQ(1, rs.count());
  rs.insert(16, 5);
  ASSERT_EQ(2, rs.count());

  ASSERT_EQ(2 + 5, rs.coverage());
}

TEST(RangeSetTest, TestPercentageComplete)
{
  Misc::UInt64RangeSet rs(5);
  ASSERT_TRUE(rs.begin() == rs.end());
  rs.insert(10, 2);
  ASSERT_EQ(1, rs.count());
  rs.insert(16, 5);
  ASSERT_EQ(2, rs.count());

  ASSERT_EQ((100 * (2 + 5)) / 21, rs.percentage_complete());
}

TEST(RangeSetTest, TestGauge)
{
  Misc::UInt64RangeSet rs(5);
  ASSERT_TRUE(rs.begin() == rs.end());
  rs.insert(10, 2);
  ASSERT_EQ(1, rs.count());
  rs.insert(16, 5);
  ASSERT_EQ(2, rs.count());

  ASSERT_EQ("          ==    =====", rs.gauge());
}

TEST(RangeSetTest, TestRead)
{
  Misc::UInt64RangeSet rs(5);
  rs.read("10-11,16-20");

  set<Misc::UInt64RangeSet::Range> expected;
  expected.insert(Misc::UInt64RangeSet::Range(10, 2));
  expected.insert(Misc::UInt64RangeSet::Range(16, 5));

  ASSERT_EQ(expected, rs.ranges);
}

TEST(RangeSetTest, TestToString)
{
  Misc::UInt64RangeSet rs(5);
  ASSERT_TRUE(rs.begin() == rs.end());
  rs.insert(10, 2);
  ASSERT_EQ(1, rs.count());
  rs.insert(16, 5);
  ASSERT_EQ(2, rs.count());

  ASSERT_EQ("10-11,16-20", rs.str());
}

TEST(RangeSetTest, TestReadFromXML)
{
  XML::Element xml("rangeset");
  xml.set_attr_int("total_length", 21);
  XML::Element& x_rs = xml.add("range");
  x_rs.set_attr_int("start", 10);
  x_rs.set_attr_int("length", 2);
  XML::Element& x_rs2 = xml.add("range");
  x_rs2.set_attr_int("start", 16);
  x_rs2.set_attr_int("length", 5);

  Misc::UInt64RangeSet rs(5);
  rs.read_from_xml(xml);

  set<Misc::UInt64RangeSet::Range> expected;
  expected.insert(Misc::UInt64RangeSet::Range(10, 2));
  expected.insert(Misc::UInt64RangeSet::Range(16, 5));

  ASSERT_EQ(expected, rs.ranges);
}

TEST(RangeSetTest, TestAddToXML)
{
  Misc::UInt64RangeSet rs(5);
  ASSERT_TRUE(rs.begin() == rs.end());
  rs.insert(10, 2);
  ASSERT_EQ(1, rs.count());
  rs.insert(16, 5);
  ASSERT_EQ(2, rs.count());

  XML::Element xml("rangeset");
  rs.add_to_xml(xml);

  set<Misc::UInt64RangeSet::Range> expected;
  expected.insert(Misc::UInt64RangeSet::Range(10, 2));
  expected.insert(Misc::UInt64RangeSet::Range(16, 5));

  list<XML::Element *> elements = xml.get_children("range");

  ASSERT_EQ(expected.size(), elements.size());
  list<XML::Element *>::const_iterator it;
  Misc::UInt64RangeSet::const_iterator ex;
  for (it = elements.begin(), ex = expected.begin();
       it != elements.end(); ++it, ++ex)
  {
    ASSERT_EQ(ex->start, (*it)->get_attr_int("start"));
    ASSERT_EQ(ex->length, (*it)->get_attr_int("length"));
  }
}

TEST(RangeSetTest, TestReadBinary)
{
  unsigned char buff[] = "\0\0\0\0\0\0\0\25" "\0\0\0\2"
                         "\0\0\0\0\0\0\0\12" "\0\0\0\0\0\0\0\2"
                         "\0\0\0\0\0\0\0\20" "\0\0\0\0\0\0\0\5";
  Channel::BlockReader br(buff, sizeof(buff));
  Misc::UInt64RangeSet rs(5);
  rs.read(br);

  set<Misc::UInt64RangeSet::Range> expected;
  expected.insert(Misc::UInt64RangeSet::Range(10, 2));
  expected.insert(Misc::UInt64RangeSet::Range(16, 5));

  ASSERT_EQ(expected, rs.ranges);
}

TEST(RangeSetTest, TestWriteBinary)
{
  unsigned char buff[44];
  Channel::BlockWriter bw(buff, sizeof(buff));

  Misc::UInt64RangeSet rs(5);
  ASSERT_TRUE(rs.begin() == rs.end());
  rs.insert(10, 2);
  ASSERT_EQ(1, rs.count());
  rs.insert(16, 5);
  ASSERT_EQ(2, rs.count());
  rs.write(bw);

  unsigned char expected[] = "\0\0\0\0\0\0\0\25" "\0\0\0\2"
                             "\0\0\0\0\0\0\0\12" "\0\0\0\0\0\0\0\2"
                             "\0\0\0\0\0\0\0\20" "\0\0\0\0\0\0\0\5";

  ASSERT_FALSE(memcmp(expected, buff, sizeof(buff)));
}

TEST(RangeSetTest, TestDump)
{
  Misc::UInt64RangeSet rs(5);
  ASSERT_TRUE(rs.begin() == rs.end());
  rs.insert(10, 2);
  ASSERT_EQ(1, rs.count());
  rs.insert(16, 5);
  ASSERT_EQ(2, rs.count());

  string expected("10, 2\n16, 5\n");
  ostringstream actual;
  rs.dump(actual);

  ASSERT_EQ(expected, actual.str());
}

} // anonymous namespace

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
      return RUN_ALL_TESTS();
}

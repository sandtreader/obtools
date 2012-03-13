//==========================================================================
// ObTools::Gen: test-shared-pointer.cc
//
// Test harness for shared pointer
//
// Copyright (c) 2012 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-gen.h"
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

TEST(SharedPointerTest, TestPointsTo)
{
  string *s = new string("Hello, world!");
  Gen::SharedPointer<string> sp(s);
  ASSERT_EQ(*s, *sp);
}

TEST(SharedPointerTest, TestCopyPointsTo)
{
  string *s = new string("Hello, world!");
  Gen::SharedPointer<string> sp(s);
  Gen::SharedPointer<string> sp2(sp);
  ASSERT_EQ(*s, *sp2);
}

TEST(SharedPointerTest, TestAssignmentPointsTo)
{
  string *s = new string("Hello, world!");
  Gen::SharedPointer<string> sp(s);
  Gen::SharedPointer<string> sp2(0);
  sp2 = sp;
  ASSERT_EQ(*s, *sp2);
}

TEST(SharedPointerDeathTest, TestReleaseOnDestruct)
{
  string *s = new string("Hello, world!");
  {
    Gen::SharedPointer<string> sp(s);
  }
  ASSERT_DEATH(s->substr(0, 5) == "Hello", "");
}

TEST(SharedPointerDeathTest, TestLastCopyReleasesOnDestruct)
{
  string *s = new string("Hello, world!");
  {
    Gen::SharedPointer<string> *sp3(0);
    {
      Gen::SharedPointer<string> sp(s);
      Gen::SharedPointer<string> sp2(sp);
      sp3 = new Gen::SharedPointer<string>(sp2);
    }
    delete sp3;
  }
  ASSERT_DEATH(s->substr(0, 5) == "Hello", "");
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

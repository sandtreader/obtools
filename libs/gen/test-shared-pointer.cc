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

TEST(SharedPointerTest, TestEmptyPointerIsInvalid)
{
  Gen::SharedPointer<int> sp(0);
  ASSERT_TRUE(!sp);
}

//--------------------------------------------------------------------------
// Helper class that alters a string on destruction
class OnDestructByeBye
{
private:
  string *str;

public:
  OnDestructByeBye(string *_str):
    str(_str)
  {}

  ~OnDestructByeBye()
  {
    *str = "Bye bye, cruel world!";
  }
};

TEST(SharedPointerTest, TestReleaseOnDestruct)
{
  string s("Hello, world!");
  {
    Gen::SharedPointer<OnDestructByeBye> sp(new OnDestructByeBye(&s));
  }
  ASSERT_EQ("Bye bye", s.substr(0, 7));
}

TEST(SharedPointerTest, TestLastCopyReleasesOnDestruct)
{
  string s("Hello, world!");
  {
    Gen::SharedPointer<OnDestructByeBye> *sp3(0);
    {
      Gen::SharedPointer<OnDestructByeBye> sp(new OnDestructByeBye(&s));
      Gen::SharedPointer<OnDestructByeBye> sp2(sp);
      sp3 = new Gen::SharedPointer<OnDestructByeBye>(sp2);
    }
    ASSERT_EQ("Hello", s.substr(0, 5));
    delete sp3;
  }
  ASSERT_EQ("Bye bye", s.substr(0, 7));
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

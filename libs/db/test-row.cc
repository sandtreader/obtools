//==========================================================================
// ObTools::DB: test-row.cc
//
// Test harness for row creation
//
// Copyright (c) 2023 Paul Clark.  All rights reserved
//==========================================================================

#include <gtest/gtest.h>
#include "ot-log.h"
#include "ot-db.h"

using namespace std;
using namespace ObTools;
using namespace ObTools::DB;

TEST(DatabaseRows, TestOutputAsWhere)
{
  DB::Row where;
  where.add("foo", "bar");
  where.add_null("splat");

  auto where_clause = where.get_where_clause();
  EXPECT_EQ("foo = 'bar' AND splat IS NULL", where_clause);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

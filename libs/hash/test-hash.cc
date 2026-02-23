//==========================================================================
// ObTools::Hash: test-hash.cc
//
// Test harness for fast ID hash table library
//
// Copyright (c) 2006 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#define _SINGLE
#include "ot-hash.h"
#include <sstream>
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

//==========================================================================
// Table construction

TEST(HashTableTest, TestConstruction)
{
  Hash::Table<> table(2, 16);
  EXPECT_TRUE(table.ok());
  EXPECT_FALSE(!table);
}

TEST(HashTableTest, TestMaxIDBits)
{
  Hash::Table<> table(2, 16);
  // 2 bits top + 16 bits hash_id = 18 bits
  EXPECT_EQ(18, table.max_id_bits());
}

TEST(HashTableTest, TestCapacity)
{
  Hash::Table<> table(2, 16);
  // 2^2 * 16 = 64
  EXPECT_EQ(64u, table.capacity());
}

TEST(HashTableTest, TestMemory)
{
  Hash::Table<> table(2, 16);
  EXPECT_GT(table.memory(), 0u);
}

//==========================================================================
// Add and lookup

TEST(HashTableTest, TestAddAndLookup)
{
  Hash::Table<> table(2, 16);
  ASSERT_TRUE(table.add(100, 1));
  EXPECT_EQ(1, table.lookup(100));
}

TEST(HashTableTest, TestLookupNonexistent)
{
  Hash::Table<> table(2, 16);
  EXPECT_EQ(Hash::INVALID_INDEX, table.lookup(999));
}

TEST(HashTableTest, TestAddMultiple)
{
  Hash::Table<> table(2, 32);
  for (int i = 0; i < 10; i++)
    ASSERT_TRUE(table.add(i * 100, i));

  for (int i = 0; i < 10; i++)
    EXPECT_EQ(i, table.lookup(i * 100));
}

TEST(HashTableTest, TestAddCollisions)
{
  // Small block to force collisions
  Hash::Table<uint32_t, uint16_t, int16_t, int32_t> table(1, 4);
  // IDs that will hash to same position
  ASSERT_TRUE(table.add(0, 10));
  ASSERT_TRUE(table.add(2, 20));   // same block, different hash
  EXPECT_EQ(10, table.lookup(0));
  EXPECT_EQ(20, table.lookup(2));
}

//==========================================================================
// Remove

TEST(HashTableTest, TestRemoveExisting)
{
  Hash::Table<> table(2, 16);
  table.add(100, 42);
  EXPECT_TRUE(table.remove(100));
  EXPECT_EQ(Hash::INVALID_INDEX, table.lookup(100));
}

TEST(HashTableTest, TestRemoveNonexistent)
{
  Hash::Table<> table(2, 16);
  EXPECT_FALSE(table.remove(999));
}

TEST(HashTableTest, TestLookupAndRemove)
{
  Hash::Table<> table(2, 16);
  table.add(100, 42);
  EXPECT_EQ(42, table.lookup_and_remove(100));
  EXPECT_EQ(Hash::INVALID_INDEX, table.lookup(100));
}

TEST(HashTableTest, TestLookupAndRemoveNonexistent)
{
  Hash::Table<> table(2, 16);
  EXPECT_EQ(Hash::INVALID_INDEX, table.lookup_and_remove(999));
}

TEST(HashTableTest, TestRemoveAndReAdd)
{
  Hash::Table<> table(2, 16);
  table.add(100, 1);
  table.remove(100);
  ASSERT_TRUE(table.add(100, 2));
  EXPECT_EQ(2, table.lookup(100));
}

TEST(HashTableTest, TestRemoveFromChain)
{
  // Use small blocks to force chaining
  Hash::Table<uint32_t, uint16_t, int16_t, int32_t> table(1, 8);

  // Add items that will chain
  table.add(2, 10);
  table.add(4, 20);  // different block, no collision
  table.add(6, 30);

  // Remove middle item
  EXPECT_TRUE(table.remove(4));
  EXPECT_EQ(Hash::INVALID_INDEX, table.lookup(4));

  // Others still there
  EXPECT_EQ(10, table.lookup(2));
  EXPECT_EQ(30, table.lookup(6));
}

//==========================================================================
// Check and stats

TEST(HashTableTest, TestCheckEmptyTable)
{
  Hash::Table<> table(2, 16);
  ostringstream oss;
  EXPECT_TRUE(table.check(oss));
}

TEST(HashTableTest, TestCheckAfterAdditions)
{
  Hash::Table<> table(2, 16);
  for (int i = 0; i < 10; i++)
    table.add(i * 7, i);

  ostringstream oss;
  EXPECT_TRUE(table.check(oss));
}

TEST(HashTableTest, TestCheckAfterRemovals)
{
  Hash::Table<> table(2, 16);
  for (int i = 0; i < 10; i++)
    table.add(i * 7, i);

  // Remove some
  table.remove(0);
  table.remove(21);
  table.remove(49);

  ostringstream oss;
  EXPECT_TRUE(table.check(oss));
}

TEST(HashTableTest, TestGetStatsEmpty)
{
  Hash::Table<> table(2, 16);
  Hash::Stats stats;
  table.get_stats(stats);
  EXPECT_EQ(0u, stats.entries);
  EXPECT_EQ(0, stats.max_chain);
  EXPECT_EQ(0, stats.max_fullness);
}

TEST(HashTableTest, TestGetStatsWithEntries)
{
  Hash::Table<> table(2, 16);
  for (int i = 0; i < 10; i++)
    table.add(i * 100, i);

  Hash::Stats stats;
  table.get_stats(stats);
  EXPECT_EQ(10u, stats.entries);
  EXPECT_GE(stats.max_chain, 1);
}

//==========================================================================
// Dump

TEST(HashTableTest, TestDump)
{
  Hash::Table<> table(2, 16);
  table.add(100, 42);

  ostringstream oss;
  table.dump(oss);
  string result = oss.str();
  // Should contain block headers
  EXPECT_NE(string::npos, result.find("Block"));
  // Should contain at least one entry or EMPTY
  EXPECT_FALSE(result.empty());
}

//==========================================================================
// Fill table to capacity

TEST(HashTableTest, TestFillToCapacity)
{
  Hash::Table<uint32_t, uint16_t, int16_t, int32_t> table(1, 8);
  // Total capacity is 2^1 * 8 = 16
  for (int i = 0; i < 16; i++)
  {
    if (!table.add(i, i))
      break;
  }

  // Verify all that were added can be looked up
  ostringstream oss;
  EXPECT_TRUE(table.check(oss)) << oss.str();
}

TEST(HashTableTest, TestOverfillReturnsFailure)
{
  Hash::Table<uint32_t, uint16_t, int16_t, int32_t> table(0, 4);
  // Only 1 block of 4 entries
  for (int i = 0; i < 4; i++)
    ASSERT_TRUE(table.add(i, i));

  // Next add should fail
  EXPECT_FALSE(table.add(99, 99));
}

//==========================================================================
// Entry template

TEST(HashEntryTest, TestUsedCheck)
{
  Hash::Entry<uint16_t, int16_t, int32_t> entry;
  entry.index = Hash::INVALID_INDEX;
  EXPECT_FALSE(entry.used());

  entry.index = 42;
  EXPECT_TRUE(entry.used());
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

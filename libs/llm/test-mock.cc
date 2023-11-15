//==========================================================================
// ObTools::LLM: test-mock.cc
//
// Test harness for mock LLM interface
//
// Copyright (c) 2023 Paul Clark
//==========================================================================

#include <gtest/gtest.h>
#include "ot-llm.h"
#include "ot-log.h"

using namespace std;
using namespace ObTools;
using namespace ObTools::LLM;

TEST(MockInterface, Test_reflection_of_context)
{
  MockInterface mock;
  mock.be_verbose();

  Context context;
  context.add({ Context::Element::Role::prompt, "FOO" });
  auto completion = mock.complete(context).as_str();
  EXPECT_EQ("I got 1 elements. The last one was 'FOO'", completion);
}

TEST(MockInterface, Test_embedding)
{
  MockInterface mock;
  auto embedding = mock.get_embedding("Wombats are go!");
  ASSERT_EQ(16, embedding.size());

  // MD5 hash (from online tool) = 6b14e801 d913ebd2 6cc48a06 a2a51a68
  Embedding expected{
    0x6b,0x14,0xe8,0x01,
    0xd9,0x13,0xeb,0xd2,
    0x6c,0xc4,0x8a,0x06,
    0xa2,0xa5,0x1a,0x68
  };
  EXPECT_EQ(expected, embedding);
}

int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_out = new Log::StreamChannel{&cout};
    auto level_out = new Log::LevelFilter{chan_out, Log::Level::detail};
    Log::logger.connect(level_out);
  }

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

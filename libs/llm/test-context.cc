//==========================================================================
// ObTools::LLM: test-context.cc
//
// Test harness for contexts
//
// Copyright (c) 2023 Paul Clark
//==========================================================================

#include <gtest/gtest.h>
#include "ot-llm.h"
#include "ot-log.h"

using namespace std;
using namespace ObTools;
using namespace ObTools::LLM;

TEST(Context, Test_adding_elements)
{
  Context context;
  context.add({ Context::Element::Role::prompt, "Make paperclips" });

  ASSERT_EQ(1, context.elements.size());
  auto& element = context.elements[0];
  EXPECT_EQ(Context::Element::Role::prompt, element.role);
  EXPECT_EQ("Make paperclips", element.message);
}

TEST(Context, Test_generating_json)
{
  Context context;
  context.add({ Context::Element::Role::instruction, "Do nothing else" });
  context.add({ Context::Element::Role::prompt, "Make paperclips" });
  context.add({ Context::Element::Role::response, "Done" });

  auto json = context.to_json();
  ASSERT_EQ(3, json.a.size());
  EXPECT_EQ("instruction", json.a[0]["role"].as_str());
  EXPECT_EQ("Do nothing else", json.a[0]["message"].as_str());
  EXPECT_EQ("prompt", json.a[1]["role"].as_str());
  EXPECT_EQ("Make paperclips", json.a[1]["message"].as_str());
  EXPECT_EQ("response", json.a[2]["role"].as_str());
  EXPECT_EQ("Done", json.a[2]["message"].as_str());
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

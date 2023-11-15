//==========================================================================
// ObTools::LLM: test-openai.cc
//
// Test harness for OpenAI interface
//
// Copyright (c) 2023 Paul Clark
//==========================================================================

#include <gtest/gtest.h>
#include "ot-llm.h"
#include "ot-log.h"
#include "ot-file.h"

using namespace std;
using namespace ObTools;
using namespace ObTools::LLM;

namespace {
  // Put your API key in here
  const auto key_path = "/var/lib/obtools/openai.api.key";
};

class OpenAITest : public ::testing::Test
{
protected:
  OpenAIInterface *openai{0};
  Context context;

  void SetUp() override
  {
    // Get key from environment
    Misc::PropertyList env;
    env.fill_from_environment();

    auto api_key = env["OPENAI_API_KEY"];
    if (api_key.empty())
    {
      // Try reading it from file
      File::Path key_file(key_path);
      if (!key_file.read_all(api_key))
      {
        cerr << "Not running live tests because no OPENAI_API_KEY in env, "
             << "nor in " << key_path << endl;
        return;
      }
    }

    api_key = Text::remove_space(api_key);
    openai = new OpenAIInterface(api_key);
    openai->set_property("temperature", 0);  // Try and make it consistent!
  }
};

TEST_F(OpenAITest, Test_simple_one_shot_context)
{
  if (!openai) return;
  context.add({ Context::Element::Role::instruction,
                "Answer with only a single word" });
  context.add({ Context::Element::Role::prompt,
                "Say hello" });
  auto completion = openai->complete(context).as_str();
  ASSERT_EQ("Hello", completion);
}

TEST_F(OpenAITest, Test_context_preserves_state)
{
  if (!openai) return;
  context.add({ Context::Element::Role::instruction,
                "Answer with only a single word" });
  context.add({ Context::Element::Role::prompt,
                "Say hello" });
  auto completion = openai->complete(context).as_str();
  context.add({ Context::Element::Role::response, completion });
  context.add({ Context::Element::Role::prompt,
                "What did you say?" });
  completion = openai->complete(context).as_str();
  ASSERT_EQ("Hello", completion);
}

TEST_F(OpenAITest, Test_embedding)
{
  if (!openai) return;
  try
  {
    auto embedding = openai->get_embedding("Wombats are go!");
    ASSERT_EQ(1536, embedding.size());

    // Just test the first few
    EXPECT_NEAR(-0.0150698, embedding[0], 0.001);
    EXPECT_NEAR(-0.0286509, embedding[1], 0.001);
    EXPECT_NEAR(-0.0187523, embedding[2], 0.001);
    EXPECT_NEAR(0.00158011, embedding[3], 0.001);
  }
  catch (Exception e)
  {
    FAIL() << e.error;
  }
}

int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_out = new Log::StreamChannel{&cout};
    auto level_out = new Log::LevelFilter{chan_out, Log::Level::dump};
    Log::logger.connect(level_out);
  }

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

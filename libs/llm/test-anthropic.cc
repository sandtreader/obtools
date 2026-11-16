//==========================================================================
// ObTools::LLM: test-anthropic.cc
//
// Test harness for Anthropic (Claude) interface
//
// Copyright (c) 2026 Paul Clark
//==========================================================================

#include <gtest/gtest.h>
#include "ot-llm.h"
#include "ot-log.h"

using namespace std;
using namespace ObTools;
using namespace ObTools::LLM;

namespace {
  using Role = Context::Element::Role;

  // A schema as callers give it, without additionalProperties
  JSON::Value action_schema()
  {
    JSON::Value schema(JSON::Value::OBJECT);
    auto& props = schema
      .set("type", "object")
      .put("properties", JSON::Value::OBJECT);
    props.put("say", JSON::Value::OBJECT)
      .set("type", "string")
      .set("description", "What to say");
    props.put("end", JSON::Value::OBJECT)
      .set("type", "boolean")
      .set("description", "Whether the conversation has ended");
    return schema;
  }

  // Parse a JSON string
  JSON::Value parse(const string& s)
  {
    istringstream iss(s);
    JSON::Parser parser(iss);
    return parser.read_value();
  }
};

//--------------------------------------------------------------------------
// Offline tests - the request we build and the responses we read

TEST(AnthropicRequest, Test_instructions_go_to_system_turns_to_messages)
{
  AnthropicInterface claude("test-key");
  Context context;
  context.add({ Role::instruction, "Be brief" });
  context.add({ Role::instruction, "Be kind" });
  context.add({ Role::prompt, "Hello" });
  context.add({ Role::response, "Hi" });
  context.add({ Role::prompt, "How are you?" });

  auto req = claude.build_request(context);
  EXPECT_EQ("claude-opus-5-5", req["model"].as_str());
  EXPECT_LT(0, req["max_tokens"].as_int());
  EXPECT_EQ("default", req["fallbacks"].as_str());

  const auto& system = req["system"];
  ASSERT_EQ(2u, system.a.size());
  EXPECT_EQ("text", system[0]["type"].as_str());
  EXPECT_EQ("Be brief", system[0]["text"].as_str());
  EXPECT_EQ("Be kind", system[1]["text"].as_str());

  const auto& messages = req["messages"];
  ASSERT_EQ(3u, messages.a.size());
  EXPECT_EQ("user", messages[0]["role"].as_str());
  EXPECT_EQ("Hello", messages[0]["content"][0]["text"].as_str());
  EXPECT_EQ("assistant", messages[1]["role"].as_str());
  EXPECT_EQ("user", messages[2]["role"].as_str());
}

TEST(AnthropicRequest, Test_same_role_turns_merged_and_empty_ones_dropped)
{
  AnthropicInterface claude("test-key");
  Context context;
  context.add({ Role::prompt, "One" });
  context.add({ Role::prompt, "" });
  context.add({ Role::prompt, " \n " });
  context.add({ Role::prompt, "Two" });
  context.add({ Role::response, "Three" });
  context.add({ Role::response, "Four" });
  context.add({ Role::prompt, "Five" });

  auto req = claude.build_request(context);
  const auto& messages = req["messages"];
  ASSERT_EQ(3u, messages.a.size());
  ASSERT_EQ(2u, messages[0]["content"].a.size());
  EXPECT_EQ("Two", messages[0]["content"][1]["text"].as_str());
  ASSERT_EQ(2u, messages[1]["content"].a.size());
  EXPECT_EQ("Five", messages[2]["content"][0]["text"].as_str());
}

TEST(AnthropicRequest, Test_conversation_starts_and_ends_with_the_user)
{
  AnthropicInterface claude("test-key");

  // Nothing but instructions: a user turn to answer
  Context empty;
  empty.add({ Role::instruction, "Begin the scene" });
  auto req0 = claude.build_request(empty);
  const auto& m0 = req0["messages"];
  ASSERT_EQ(1u, m0.a.size());
  EXPECT_EQ("user", m0[0]["role"].as_str());
  EXPECT_FALSE(m0[0]["content"][0]["text"].as_str().empty());

  // Starting and ending with responses
  Context context;
  context.add({ Role::response, "I remember this" });
  context.add({ Role::prompt, "Go on" });
  context.add({ Role::response, "So I said" });
  auto req1 = claude.build_request(context);
  const auto& m1 = req1["messages"];
  // (An opening user turn, the three given, and a closing user turn.)
  ASSERT_EQ(5u, m1.a.size());
  EXPECT_EQ("user", m1[0]["role"].as_str());
  EXPECT_EQ("assistant", m1[1]["role"].as_str());
  EXPECT_EQ("assistant", m1[3]["role"].as_str());
  EXPECT_EQ("user", m1[4]["role"].as_str());
  EXPECT_FALSE(m1[4]["content"][0]["text"].as_str().empty());
}

TEST(AnthropicRequest, Test_properties_map_and_sampling_ones_are_dropped)
{
  AnthropicInterface claude("test-key");
  claude.set_property("model", "claude-sonnet-5-5");
  claude.set_property("effort", "low");
  claude.set_property("max_tokens", 2000);
  claude.set_property("temperature", 0);
  claude.set_property("top_p", 0.5);
  Context context;
  context.add({ Role::prompt, "Hi" });

  auto req = claude.build_request(context);
  EXPECT_EQ("claude-sonnet-5-5", req["model"].as_str());
  EXPECT_EQ("low", req["output_config"]["effort"].as_str());
  EXPECT_EQ(2000, req["max_tokens"].as_int());
  EXPECT_FALSE(!!req["temperature"]);
  EXPECT_FALSE(!!req["top_p"]);
  EXPECT_FALSE(!!req["effort"]);
}

TEST(AnthropicRequest, Test_mandatory_function_asks_for_structured_output)
{
  AnthropicInterface claude("test-key");
  claude.register_function("action", "Use this for all responses", nullptr,
                           action_schema());
  Context context;
  context.add({ Role::prompt, "Hi" });

  auto req = claude.build_request(context);
  const auto& format = req["output_config"]["format"];
  EXPECT_EQ("json_schema", format["type"].as_str());
  EXPECT_EQ("object", format["schema"]["type"].as_str());
  EXPECT_FALSE(format["schema"]["additionalProperties"].is_true());
  EXPECT_EQ(JSON::Value::FALSE_,
            format["schema"]["additionalProperties"].type);
  EXPECT_EQ("string",
            format["schema"]["properties"]["say"]["type"].as_str());
  // (Every property required, all of them: see closed() in anthropic.cc.)
  const auto& required = format["schema"]["required"];
  ASSERT_EQ(2u, required.a.size());
  EXPECT_EQ("end", required[0].as_str());
  EXPECT_EQ("say", required[1].as_str());
  EXPECT_FALSE(!!req["tools"]);
}

TEST(AnthropicRequest, Test_functions_with_callbacks_are_tools)
{
  AnthropicInterface claude("test-key");
  claude.register_function("wave", "Wave a hand",
                           [](const JSON::Value&) {}, action_schema());
  Context context;
  context.add({ Role::prompt, "Hi" });

  auto req = claude.build_request(context);
  const auto& tools = req["tools"];
  ASSERT_EQ(1u, tools.a.size());
  EXPECT_EQ("wave", tools[0]["name"].as_str());
  EXPECT_EQ("Wave a hand", tools[0]["description"].as_str());
  EXPECT_EQ("object", tools[0]["input_schema"]["type"].as_str());
  EXPECT_FALSE(!!req["output_config"]["format"]);
}

TEST(AnthropicResponse, Test_text_is_returned_and_thinking_ignored)
{
  AnthropicInterface claude("test-key");
  auto resp = parse(R"({"type":"message","role":"assistant",
    "stop_reason":"end_turn",
    "content":[{"type":"thinking","thinking":"","signature":"x"},
               {"type":"text","text":"Hello"}]})");
  EXPECT_EQ("Hello", claude.read_response(resp).as_str());
}

TEST(AnthropicResponse, Test_mandatory_function_returns_parsed_json)
{
  AnthropicInterface claude("test-key");
  claude.register_function("action", "Use this for all responses", nullptr,
                           action_schema());
  auto resp = parse(R"({"type":"message","role":"assistant",
    "stop_reason":"end_turn",
    "content":[{"type":"text","text":"{\"say\":\"Hi there\",\"end\":false}"}]})");
  auto result = claude.read_response(resp);
  ASSERT_EQ(JSON::Value::OBJECT, result.type);
  EXPECT_EQ("Hi there", result["say"].as_str());
}

TEST(AnthropicResponse, Test_tool_use_calls_its_callback)
{
  AnthropicInterface claude("test-key");
  string waved;
  claude.register_function("wave", "Wave a hand",
                           [&waved](const JSON::Value& args)
                           { waved = args["say"].as_str(); },
                           action_schema());
  auto resp = parse(R"({"type":"message","role":"assistant",
    "stop_reason":"tool_use",
    "content":[{"type":"text","text":"Waving"},
               {"type":"tool_use","id":"toolu_1","name":"wave",
                "input":{"say":"bye"}}]})");
  EXPECT_EQ("Waving", claude.read_response(resp).as_str());
  EXPECT_EQ("bye", waved);
}

TEST(AnthropicResponse, Test_refusal_and_truncation_throw)
{
  AnthropicInterface claude("test-key");
  auto refused = parse(R"({"type":"message","role":"assistant",
    "stop_reason":"refusal",
    "stop_details":{"type":"refusal","category":"cyber"},
    "content":[]})");
  EXPECT_THROW(claude.read_response(refused), Exception);

  claude.register_function("action", "Use this for all responses", nullptr,
                           action_schema());
  auto cut = parse(R"({"type":"message","role":"assistant",
    "stop_reason":"max_tokens",
    "content":[{"type":"text","text":"{\"say\":\"Hi"}]})");
  EXPECT_THROW(claude.read_response(cut), Exception);
}

TEST(AnthropicResponse, Test_no_embeddings)
{
  AnthropicInterface claude("test-key");
  EXPECT_THROW(claude.get_embedding("Hello"), Exception);
}

//--------------------------------------------------------------------------
// Live tests - only with ANTHROPIC_API_KEY set (Tup doesn't pass it through,
// so a build never calls the API: run this by hand to try them)

class AnthropicTest : public ::testing::Test
{
protected:
  AnthropicInterface *claude{0};
  Context context;

  void SetUp() override
  {
    // Get key from environment
    Misc::PropertyList env;
    env.fill_from_environment();

    auto api_key = env["ANTHROPIC_API_KEY"];
    if (api_key.empty())
    {
      cerr << "Not running live tests because no ANTHROPIC_API_KEY in env\n";
      return;
    }

    api_key = Text::remove_space(api_key);
    claude = new AnthropicInterface(api_key);
    claude->set_property("effort", "low");  // Quick, for a test
  }

  void TearDown() override
  {
    delete claude;
  }
};

TEST_F(AnthropicTest, Test_simple_one_shot_context)
{
  if (!claude) return;
  context.add({ Role::instruction,
                "Answer with only a single word, with no punctuation" });
  context.add({ Role::prompt, "Say hello" });
  auto completion = Text::tolower(claude->complete(context).as_str());
  EXPECT_NE(string::npos, completion.find("hello"));
}

TEST_F(AnthropicTest, Test_mandatory_function)
{
  if (!claude) return;
  claude->register_function("action", "Use this for all responses", nullptr,
                            action_schema());
  context.add({ Role::instruction,
                "You are a friendly shopkeeper. Greet the customer." });
  context.add({ Role::prompt, "Customer: Good morning!" });
  auto action = claude->complete(context);
  ASSERT_EQ(JSON::Value::OBJECT, action.type);
  EXPECT_FALSE(action["say"].as_str().empty());
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

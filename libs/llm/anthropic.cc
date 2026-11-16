//==========================================================================
// ObTools::LLM: anthropic.cc
//
// Implementation of an LLM interface for Anthropic / Claude, through the
// Messages API
//
// Copyright (c) 2026 Paul Clark
//==========================================================================

#include "ot-llm.h"
#include "ot-log.h"
#include <set>

namespace ObTools { namespace LLM {

namespace {
  const string anthropic_api_url{"https://api.anthropic.com/v1"};
  const auto anthropic_messages_url = anthropic_api_url+"/messages";
  const auto anthropic_version = "2026-10-21";
  // On a refusal by the model's safety classifiers, the API runs the request
  // again on a model it chooses for the refusal's category
  const auto anthropic_beta = "server-side-fallback-2026-07-01";
  const auto default_model = "claude-opus-5-5";
  const int64_t default_max_tokens = 16000;
  const auto user_agent = "ObTools AI Agent";
  const auto connection_timeout = 15;
  const auto operation_timeout = 120;  // (it thinks before it answers)

  // The user's turns the API needs where the context has none: a
  // conversation starts with the user, and ends with them (an answer can't
  // be begun for the model)
  const auto opening_prompt = "(Begin.)";
  const auto continuing_prompt = "(Continue.)";

  // Sampling properties the API refuses, and those of the request's own
  // which go elsewhere in it
  const set<string> refused_properties{"temperature", "top_p", "top_k"};

  // A text content block
  JSON::Value text_block(const string& text)
  {
    JSON::Value block(JSON::Value::OBJECT);
    block.set("type", "text").set("text", text);
    return block;
  }

  // A schema as structured output takes it: every object closed to
  // properties it doesn't list, and requiring all those it does.  (The
  // answer's properties come in the schema's order, which our JSON writes
  // alphabetically; left optional, the model skipped them for one that
  // came later, then wrote them inside that one's string, sometimes
  // looping there till it ran out of tokens.)
  JSON::Value closed(const JSON::Value& schema)
  {
    JSON::Value s = schema;
    if (s.type != JSON::Value::OBJECT) return s;
    if (s["type"].as_str() == "object" && !s["additionalProperties"])
      s.put("additionalProperties", JSON::Value(JSON::Value::FALSE_));
    if (s["properties"].type == JSON::Value::OBJECT)
    {
      auto& required = s.put("required", JSON::Value::ARRAY);
      for(const auto& it: s["properties"].o)
        required.add(it.first);
    }
    for(auto& it: s["properties"].o)
      it.second = closed(it.second);
    if (s["items"].type == JSON::Value::OBJECT)
      s.put("items", closed(s["items"]));
    for(const auto& key: { "anyOf", "allOf" })
      for(auto& sub: s[key].a)
        sub = closed(sub);
    for(const auto& key: { "$defs", "definitions" })
      for(auto& it: s[key].o)
        it.second = closed(it.second);
    return s;
  }
}

/// Constructor
AnthropicInterface::AnthropicInterface(const string& _api_key):
  api_key(_api_key),
  http_client(Web::URL(anthropic_api_url), &ssl_context, user_agent,
              connection_timeout, operation_timeout)
{
  http_client.enable_persistence();
}

/// The request for a completion with a given context
JSON::Value AnthropicInterface::build_request(const Context& context) const
{
  JSON::Value req_json(JSON::Value::OBJECT);
  req_json.put("model", default_model);
  req_json.put("max_tokens", default_max_tokens);
  JSON::Value output_config(JSON::Value::OBJECT);

  for(const auto& p: string_props)
  {
    if (p.first == "effort")
      output_config.set("effort", p.second);
    else if (!refused_properties.count(p.first))
      req_json.put(p.first, p.second);
  }

  for(const auto& p: number_props)
  {
    if (p.first == "max_tokens")
      req_json.put("max_tokens", static_cast<int64_t>(p.second));
    else if (!refused_properties.count(p.first))
      req_json.put(p.first, p.second);
  }

  // Instructions are the system prompt, in order; the rest the
  // conversation, a turn's consecutive elements merged into one
  auto& system = req_json.put("system", JSON::Value::ARRAY);
  auto& messages = req_json.put("messages", JSON::Value::ARRAY);
  for(const auto& element: context.elements)
  {
    // (Blank text is refused)
    if (element.message.find_first_not_of(" \t\r\n") == string::npos)
      continue;

    string role;
    switch (element.role)
    {
      case Context::Element::Role::instruction:
        system.add(text_block(element.message));
        continue;

      case Context::Element::Role::prompt:
        role = "user";
        break;

      case Context::Element::Role::response:
        role = "assistant";
        break;
    }

    if (messages.a.empty() && role == "assistant")
    {
      auto& opening = messages.add(JSON::Value::OBJECT);
      opening.set("role", "user");
      opening.put("content", JSON::Value::ARRAY).add(text_block(opening_prompt));
    }

    if (messages.a.empty() || messages.a.back()["role"].as_str() != role)
    {
      auto& msg = messages.add(JSON::Value::OBJECT);
      msg.set("role", role);
      msg.put("content", JSON::Value::ARRAY);
    }

    messages.a.back()["content"].add(text_block(element.message));
  }

  if (messages.a.empty() || messages.a.back()["role"].as_str() != "user")
  {
    auto& msg = messages.add(JSON::Value::OBJECT);
    msg.set("role", "user");
    msg.put("content", JSON::Value::ARRAY)
      .add(text_block(messages.a.size() > 1 ? continuing_prompt
                                            : opening_prompt));
  }

  if (system.a.empty()) req_json.o.erase("system");

  // A function without a callback is the response's shape, as structured
  // output (the model can't be made to call a tool); those with callbacks
  // are tools it may use
  for(const auto& it: functions)
  {
    const auto& name = it.first;
    const auto& fn = it.second;
    if (!fn.callback)
    {
      auto& format = output_config.put("format", JSON::Value::OBJECT);
      format.set("type", "json_schema").set("schema", closed(fn.params_schema));
    }
    else
    {
      if (!req_json["tools"]) req_json.put("tools", JSON::Value::ARRAY);
      auto& tool = req_json["tools"].add(JSON::Value::OBJECT);
      tool
        .set("name", name)
        .set("description", fn.description)
        .set("input_schema", fn.params_schema);
    }
  }

  if (!output_config.o.empty()) req_json.put("output_config", output_config);

  req_json.put("fallbacks", "default");
  return req_json;
}

/// Read a completion from the API's response
JSON::Value AnthropicInterface::read_response(const JSON::Value& resp_json) const
{
  const auto& stop_reason = resp_json["stop_reason"].as_str();
  if (stop_reason == "refusal")
  {
    const auto& category = resp_json["stop_details"]["category"].as_str();
    throw Exception("Claude declined to answer"
                    + (category.empty() ? string() : " ("+category+")"));
  }

  // The text, its blocks together; tools used have their callbacks called
  // (thinking and the rest are passed over)
  string text;
  auto used_tool = false;
  for(const auto& block: resp_json["content"].a)
  {
    const auto& type = block["type"].as_str();
    if (type == "text")
      text += block["text"].as_str();
    else if (type == "tool_use")
    {
      const auto& name = block["name"].as_str();
      const auto it = functions.find(name);
      if (it == functions.end() || !it->second.callback)
        throw Exception("Unknown function called: "+name);
      it->second.callback(block["input"]);
      used_tool = true;
    }
  }

  // Asked for structured output: return it parsed
  for(const auto& it: functions)
  {
    if (it.second.callback) continue;
    if (stop_reason == "max_tokens")
      throw Exception("Claude's structured answer was cut short at max_tokens");

    try
    {
      istringstream iss(text);
      JSON::Parser parser(iss);
      return parser.read_value();
    }
    catch (const JSON::Exception& e)
    {
      throw Exception("Bad structured answer from Claude: "+e.error);
    }
  }

  // Content is only allowed to be empty if it used a tool
  if (text.empty() && !used_tool)
    throw Exception("Empty content returned from Claude");

  return JSON::Value(text);
}

/// Get a completion with a given context
JSON::Value AnthropicInterface::complete(const Context& context)
{
  return read_response(call_api(anthropic_messages_url,
                                build_request(context)));
}

/// Embeddings aren't offered by Anthropic
Embedding AnthropicInterface::get_embedding(const string&)
{
  throw Exception("Anthropic offers no embeddings: use another interface "
                  "for them");
}

/// Call an Anthropic API endpoint, returning JSON
JSON::Value AnthropicInterface::call_api(const string& url,
                                         const JSON::Value& req_json)
{
  Log::Streams log;
  Web::HTTPMessage request("POST", Web::URL(url));
  request.headers.put("x-api-key", api_key);
  request.headers.put("anthropic-version", anthropic_version);
  request.headers.put("anthropic-beta", anthropic_beta);
  request.headers.put("Content-Type", "application/json");
  request.body = req_json.str();

  OBTOOLS_LOG_IF_DEBUG(log.debug << "\n\n>>> " << req_json.str(true) << endl;)

  Web::HTTPMessage response;
  if (!http_client.fetch(request, response))
  {
    log.error << "Anthropic HTTP fetch failed (internal)\n";
    throw Exception("Anthropic HTTP fetch failed");
  }

  // The response, or the error's own message
  JSON::Value resp_json;
  try
  {
    istringstream iss(response.body);
    JSON::Parser parser(iss);
    resp_json = parser.read_value();
  }
  catch (const JSON::Exception& e)
  {
    log.error << "Anthropic gave bad JSON response (" << response.code
              << "): " << e.error << endl;
    throw Exception("Bad JSON response from Anthropic");
  }

  if (response.code == 200)
  {
    OBTOOLS_LOG_IF_DEBUG(log.debug << "\n\n<<< " << response.body << endl;)
    return resp_json;
  }

  const auto& message = resp_json["error"]["message"].as_str();
  log.error << "POST to Anthropic failed: " << response.code << " "
            << (message.empty() ? response.body : message) << endl;
  throw Exception("Anthropic POST failed: " + to_string(response.code)
                  + (message.empty() ? string() : " "+message));
}

}} // namespaces

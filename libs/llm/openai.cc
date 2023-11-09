//==========================================================================
// ObTools::LLM: openai.cc
//
// Implementation of an LLM interface for OpenAI / GPT
//
// Copyright (c) 2023 Paul Clark
//==========================================================================

#include "ot-llm.h"
#include "ot-log.h"

namespace ObTools { namespace LLM {

namespace {
  const string openai_api_url{"https://api.openai.com/v1"};
  const auto openai_completion_url = openai_api_url+"/chat/completions";
  const auto openai_completion_model = "gpt-4";
  const auto user_agent = "ObTools AI Agent";
  const auto connection_timeout = 15;
  const auto operation_timeout = 60;
}

/// Constructor
OpenAIInterface::OpenAIInterface(const string& _api_key):
  api_key(_api_key),
  http_client(Web::URL(openai_api_url), &ssl_context, user_agent,
              connection_timeout, operation_timeout)
{
  http_client.enable_persistence();
}

/// Get a completion with a given context
JSON::Value OpenAIInterface::complete(const Context& context)
{
  Log::Streams log;
  JSON::Value req_json(JSON::Value::OBJECT);
  req_json.put("model", openai_completion_model);
  for(const auto& p: string_props) req_json.put(p.first, p.second);
  for(const auto& p: number_props) req_json.put(p.first, p.second);

  auto& messages = req_json.put("messages", JSON::Value::ARRAY);
  for(const auto& element: context.elements)
  {
    auto& msg = messages.add(JSON::Value::OBJECT);
    switch (element.role)
    {
      case Context::Element::Role::instruction:
        msg.set("role", "system");
        break;

      case Context::Element::Role::prompt:
        msg.set("role", "user");
        break;

      case Context::Element::Role::response:
        msg.set("role", "assistant");
        break;
    }

    msg.set("content", element.message);
  }

  // Add functions
  if (!functions.empty())
  {
    auto& jf = req_json.put("functions", JSON::Value::ARRAY);
    for(const auto& it: functions)
    {
      const auto& name = it.first;
      const auto& fn = it.second;
      auto& jfo = jf.add(JSON::Value::OBJECT);
      jfo
        .set("name", name)
        .set("description", fn.description)
        .set("parameters", fn.params_schema);

      // If it doesn't have a callback, force the LLM to use it
      if (!fn.callback)
      {
        JSON::Value fc_json(JSON::Value::OBJECT);
        fc_json.put("name", name);
        req_json.put("function_call", fc_json);
      }
    }
  }

  Web::HTTPMessage request("POST", Web::URL(openai_completion_url));
  request.headers.put("Authorization", "Bearer "+api_key);
  request.headers.put("Content-Type", "application/json");
  request.body = req_json.str();

  OBTOOLS_LOG_IF_DEBUG(log.debug << "\n\n>>> " << req_json.str(true) << endl;)

  Web::HTTPMessage response;
  if (!http_client.fetch(request, response))
  {
    log.error << "OpenAI HTTP fetch failed (internal)\n";
    throw Exception("OpenAI HTTP fetch failed");
  }

  if (response.code == 200)
  {
    OBTOOLS_LOG_IF_DEBUG(log.debug << "\n\n<<< " << response.body << endl;)

    try
    {
      istringstream iss(response.body);
      JSON::Parser parser(iss);
      auto resp_json = parser.read_value();

      const auto& choices = resp_json["choices"];
      if (choices.a.empty())
        throw Exception("No choices returned from OpenAI");

      const auto& message = choices[0]["message"];
      if (!message) throw new Exception("No message returned from OpenAI");
      if (message["role"].as_str() != "assistant")
        throw Exception("Response role was not assistant");

      const auto& content = message["content"].as_str();

      // Check for function call
      const auto& fc = message["function_call"];
      if (fc.type == JSON::Value::OBJECT)
      {
        // Find the function
        const auto& fname = fc["name"].as_str();
        if (fname.empty()) throw Exception("Function call has no name");
        const auto it = functions.find(fname);
        if (it == functions.end())
          throw Exception("Unknown function called: "+fname);
        const auto fn = it->second;

        const auto& args_s = fc["arguments"].as_str();
        if (args_s.empty()) throw Exception("Function call has no arguments");

        istringstream iss_args(args_s);
        JSON::Parser parser_args(iss_args); // Using the outer try/catch
        const auto args = parser_args.read_value();

        if (fn.callback)
        {
          // Call the function
          fn.callback(args);

          // !!! Allow a response from the function which we feed back?
        }
        else
        {
          // Mandatory function - return the args directly
          return args;
        }
      }
      else
      {
        // Content is only allowed to be empty if it was a function call
        if (content.empty())
          throw Exception("Empty content returned from OpenAI");
      }

      return JSON::Value(content);
    }
    catch (JSON::Exception e)
    {
      log.error << "OpenAI gave bad JSON response: " << e.error << endl;
      throw Exception("Bad JSON response from OpenAI");
    }
  }
  else
  {
    log.error << "POST to OpenAI failed: " << response.code
              << " " << response.body << endl;
    throw Exception("OpenAI POST failed");
  }
}

}} // namespaces


//==========================================================================
// ObTools::LLM: ot-llm.h
//
// Public definitions for ObTools::LLM library
//
// Copyright (c) 2023 Paul Clark.
//==========================================================================

#ifndef __OBTOOLS_LLM_H
#define __OBTOOLS_LLM_H

#include <string>
#include <ot-json.h>
#include <ot-web.h>
#include <ot-ssl-openssl.h>

namespace ObTools { namespace LLM {

// Make our lives easier without polluting anyone else
using namespace std;
using namespace ObTools;

//==========================================================================
/// Context for an AI completion
struct Context
{
  // Element of the context - messages being passed back and forth
  struct Element
  {
    enum class Role
    {
      instruction,
      prompt,
      response
    };

    Role role;
    string message;

    Element(Role _role, const string& _message):
      role(_role), message(_message) {}
  };

  vector<Element> elements;     // List of ordered elements

  // Add an element
  void add(const Element& e) { elements.push_back(e); }

  // Get a JSON structure for the context
  JSON::Value to_json() const;
};

//==========================================================================
// Embedding
using Embedding = vector<double>;

//==========================================================================
// Interface exception
struct Exception
{
  string error;
  Exception(const string& _error): error(_error) {}
};

//==========================================================================
/// Generic LLM interface
class Interface
{
public:
  /// Definition of a function callback taking JSON object
  using callback_t = function<void(const JSON::Value&)>;

protected:
  struct Function
  {
    string description;
    callback_t callback;  // If null, response is mandatory raw function call
    JSON::Value params_schema;
  };

  map<string, string> string_props;
  map<string, double> number_props;
  map<string, Function> functions;  // By name

 public:
  /// Set a configuration property, string or number
  void set_property(const string& name, const string& value)
  { string_props[name] = value; }
  void set_property(const string& name, double value)
  { number_props[name] = value; }

  /// Register a function
  void register_function(const string& name, const string& description,
                         callback_t callback, const JSON::Value& params_schema)
  {
    functions[name] = Function({ description, callback, params_schema });
  }

  /// Get a completion with a given context
  // In normal use, the result will just be a JSON string.  With a mandatory
  // function it will be the function call
  virtual JSON::Value complete(const Context& context) = 0;

  /// Get an embedding for the given text
  virtual Embedding get_embedding(const string& text) = 0;

  /// Virtual destructor
  virtual ~Interface() {}
};

//==========================================================================
/// Mock LLM interface
class MockInterface: public Interface
{
  bool verbose{false};

 public:
  /// Constructor
  MockInterface() {}

  /// Be verbose
  void be_verbose() { verbose = true; }

  /// Get a completion with a given context
  JSON::Value complete(const Context& context) override;

  /// Get an embedding for the given text
  Embedding get_embedding(const string& text) override;
};

//==========================================================================
/// OpenAI interface
class OpenAIInterface: public Interface
{
  string api_key;

  ObTools::SSL_OpenSSL::Context ssl_context;
  Web::HTTPClient http_client;

  JSON::Value call_api(const string& url, const JSON::Value& req_json);

 public:
  /// Constructor
  OpenAIInterface(const string& _api_key);

  /// Get a completion with a given context
  JSON::Value complete(const Context& context) override;

  /// Get an embedding for the given text
  Embedding get_embedding(const string& text) override;
};

//==========================================================================
/// Anthropic interface (Claude, through the Messages API)
// Properties: "model" (claude-opus-5-5 by default), "effort" (low, medium,
// high, xhigh or max: how hard it thinks; the model's default if unset) and
// "max_tokens"; temperature and the other sampling properties are dropped
// (the API refuses them).  A function with no callback is asked for as
// structured output, its schema the response's, and returned parsed; those
// with callbacks are offered as tools, and called as the model uses them.
class AnthropicInterface: public Interface
{
  string api_key;

  ObTools::SSL_OpenSSL::Context ssl_context;
  Web::HTTPClient http_client;

  JSON::Value call_api(const string& url, const JSON::Value& req_json);

 public:
  /// Constructor
  AnthropicInterface(const string& _api_key);

  /// The request for a completion with a given context
  JSON::Value build_request(const Context& context) const;

  /// Read a completion from the API's response: the text, or with a
  /// mandatory function, its arguments; tools used have their callbacks
  /// called.  Throws Exception on a refusal or a truncated answer
  JSON::Value read_response(const JSON::Value& resp_json) const;

  /// Get a completion with a given context
  JSON::Value complete(const Context& context) override;

  /// Embeddings aren't offered by Anthropic: throws Exception
  Embedding get_embedding(const string& text) override;
};

//==========================================================================
}} // namespaces

#endif // !__OBTOOLS_LLM_H

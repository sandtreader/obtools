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
}} // namespaces

#endif // !__OBTOOLS_LLM_H

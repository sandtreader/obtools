# CLAUDE.md - ObTools::LLM Library

## Overview

`ObTools::LLM` provides an abstraction layer for Large Language Model APIs, with Anthropic (Claude), OpenAI and mock implementations. Lives under `namespace ObTools::LLM`.

**Header:** `ot-llm.h`
**Dependencies:** `ot-log`, `ot-time`, `ot-json`, `ot-misc`, `ot-ssl-openssl`, `ot-web`

## Key Classes

| Class | Purpose |
|-------|---------|
| `Interface` | Abstract LLM interface |
| `AnthropicInterface` | Anthropic/Claude implementation (Messages API) |
| `OpenAIInterface` | OpenAI/GPT implementation |
| `MockInterface` | Mock for testing (reflects context, MD5 embeddings) |
| `Context` | Conversation context (instructions, prompts, responses) |
| `Exception` | Interface error |

## Types

```cpp
using Embedding = vector<double>;
```

## Context

```cpp
struct Context {
  struct Element {
    enum class Role { instruction, prompt, response };
    Role role;
    string message;
  };
  vector<Element> elements;
  void add(const Element& e);
  JSON::Value to_json() const;
};
```

## Interface

```cpp
void set_property(const string& name, const string& value);
void set_property(const string& name, double value);
void register_function(const string& name, const string& description,
                      callback_t callback, const JSON::Value& params_schema);
virtual JSON::Value complete(const Context& context) = 0;
virtual Embedding get_embedding(const string& text) = 0;
```

## AnthropicInterface

```cpp
AnthropicInterface(const string& api_key);
JSON::Value build_request(const Context& context) const;   // the request JSON
JSON::Value read_response(const JSON::Value& resp) const;  // text, or mandatory fn's args
JSON::Value complete(const Context& context) override;     // claude-opus-5-5 by default
Embedding get_embedding(const string& text) override;      // throws: no embeddings
```

Properties: `model`, `effort` (into `output_config.effort`), `max_tokens`
(16000 by default); `temperature`, `top_p`, `top_k` dropped (refused by the
API). A function without a callback becomes `output_config.format` (JSON
schema, every object given `additionalProperties: false`), since the current
models refuse a forced tool call; functions with callbacks become `tools`.
Sends `fallbacks: "default"` with the `server-side-fallback-2026-07-01` beta;
throws on `stop_reason: "refusal"`, and on structured output cut short at
`max_tokens`.

## OpenAIInterface

```cpp
OpenAIInterface(const string& api_key);
JSON::Value complete(const Context& context) override;   // uses gpt-4
Embedding get_embedding(const string& text) override;     // uses text-embedding-ada-002
```

## MockInterface

```cpp
MockInterface();
void be_verbose();
JSON::Value complete(const Context& context) override;   // reflects context
Embedding get_embedding(const string& text) override;     // MD5-based
```

# CLAUDE.md - ObTools::LLM Library

## Overview

`ObTools::LLM` provides an abstraction layer for Large Language Model APIs, with OpenAI and mock implementations. Lives under `namespace ObTools::LLM`.

**Header:** `ot-llm.h`
**Dependencies:** `ot-log`, `ot-time`, `ot-json`, `ot-misc`, `ot-ssl-openssl`, `ot-web`

## Key Classes

| Class | Purpose |
|-------|---------|
| `Interface` | Abstract LLM interface |
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

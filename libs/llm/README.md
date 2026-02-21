# ObTools::LLM

Abstraction layer for Large Language Model APIs, with OpenAI and mock implementations. Supports completions, embeddings, and function calling.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-llm.h"
using namespace ObTools;

// Create interface
LLM::OpenAIInterface llm("sk-your-api-key");
llm.set_property("temperature", 0.7);

// Build conversation context
LLM::Context ctx;
ctx.add({LLM::Context::Element::Role::instruction,
         "You are a helpful assistant."});
ctx.add({LLM::Context::Element::Role::prompt,
         "What is the capital of France?"});

// Get completion
JSON::Value response = llm.complete(ctx);
```

### Embeddings

```cpp
LLM::Embedding embedding = llm.get_embedding("Hello world");
// Returns vector<double> from text-embedding-ada-002
```

### Mock Interface for Testing

```cpp
LLM::MockInterface mock;
mock.be_verbose();

LLM::Context ctx;
ctx.add({LLM::Context::Element::Role::prompt, "test"});
JSON::Value response = mock.complete(ctx);  // reflects context back
```

## Build

```
NAME    = ot-llm
TYPE    = lib
DEPENDS = ot-log ot-time ot-json ot-misc ot-ssl-openssl ot-web
```

## License

Copyright (c) 2023 Paul Clark. MIT License.

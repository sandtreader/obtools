# ObTools::LLM

Abstraction layer for Large Language Model APIs, with Anthropic (Claude), OpenAI and mock implementations. Supports completions, embeddings (OpenAI and mock only), and function calling.

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

### Anthropic (Claude)

```cpp
LLM::AnthropicInterface llm("sk-ant-your-api-key");
llm.set_property("model", "claude-opus-5-5");  // the default
llm.set_property("effort", "low");             // low ... max: lower answers sooner
JSON::Value response = llm.complete(ctx);
```

Calls the Messages API (`https://api.anthropic.com/v1/messages`, API version
`2026-10-21`). Instructions become the system prompt; consecutive prompts or
responses are merged into one turn, blank ones dropped, and a short user turn
is added where the API needs one (it starts and ends with the user).
`temperature`, `top_p` and `top_k` are dropped: the API refuses them. A
function registered without a callback is asked for as structured output (its
schema the response's) and its arguments returned, as OpenAI's forced function
call; those with callbacks are offered as tools and called as the model uses
them. On a refusal by Claude's safety classifiers the API runs the request
again on a fallback model it chooses (`fallbacks: "default"`); a refusal it
can't recover from throws `Exception`. There are no embeddings:
`get_embedding()` throws.

The live tests in `test-anthropic.cc` run only with `ANTHROPIC_API_KEY` set
(Tup doesn't pass it through, so a build never calls the API).

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

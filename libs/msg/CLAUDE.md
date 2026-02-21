# CLAUDE.md - ObTools::Message Library

## Overview

`ObTools::Message` provides a generic message/RPC broker supporting multiple transports and XML-configured handlers. Header-only. Lives under `namespace ObTools::Message`.

**Header:** `ot-msg.h`
**Dependencies:** `ot-xml`, `ot-init`, `ot-ssl`
**Type:** headers

## Key Classes

| Class | Purpose |
|-------|---------|
| `Handler<CONTEXT>` | Abstract message handler |
| `Transport<CONTEXT>` | Abstract transport layer |
| `Broker<CONTEXT>` | Message broker managing handlers and transports |
| `Exception` | Message handling error |

## Handler<CONTEXT>

```cpp
Handler(const XML::Element& cfg, const string& doc_name = "",
        const string& ns_prefix = "", const string& ns_url = "",
        bool complex_result = false);
virtual void handle_message(CONTEXT& context, const XML::Element& request,
                           const SSL::ClientDetails& client,
                           XML::Element& response) = 0;
```

Members: `name`, `document_name`, `complex_result`, `ns_prefix`, `ns_url`

## Transport<CONTEXT>

```cpp
Transport(const string& name);
virtual void register_handler(Handler<CONTEXT>& handler,
                              const XML::Element& config) = 0;
```

## Broker<CONTEXT>

```cpp
Broker(Init::Registry<Handler<CONTEXT>>& handler_registry);
void add_transport(Transport<CONTEXT> *trans);
void configure(const XML::Element& config);  // from <message> XML
void shutdown();
```

## Design

- Handlers are created via `Init::Registry` from XML config
- Multiple transports can serve the same handler
- Handlers throw `Exception` on errors
- CONTEXT template allows any handler context type

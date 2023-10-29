//==========================================================================
// ObTools::Core: context.cc
//
// Implementation of contexts
//
// Copyright (c) 2023 Paul Clark
//==========================================================================

#include "ot-llm.h"

namespace ObTools { namespace LLM {

// Get a JSON structure for the context
JSON::Value Context::to_json() const
{
  JSON::Value json(JSON::Value::ARRAY);
  for(const auto& element: elements)
  {
    auto& ejo = json.add(JSON::Value::OBJECT);
    ejo.set("message", element.message);
    switch (element.role)
    {
      case Context::Element::Role::instruction:
        ejo.set("role", "instruction");
        break;

      case Context::Element::Role::prompt:
        ejo.set("role", "prompt");
        break;

      case Context::Element::Role::response:
        ejo.set("role", "response");
        break;
    }
  }

  return json;
}

}} // namespaces


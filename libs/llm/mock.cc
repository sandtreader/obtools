//==========================================================================
// ObTools::LLM: mock.cc
//
// Implementation of a mock LLM interface
//
// Copyright (c) 2023 Paul Clark
//==========================================================================

#include "ot-llm.h"
#include "ot-log.h"

namespace ObTools { namespace LLM {

/// Get a completion with a given context and prompt
JSON::Value MockInterface::complete(const Context& context)
{
  if (verbose)
  {
    Log::Detail log;
    log << "Mock context:\n" << context.to_json().str(true);
  }

  ostringstream oss;
  oss << "I got " << context.elements.size() << " elements.";
  if (!context.elements.empty())
    oss << " The last one was '" << context.elements.back().message << "'";
  return JSON::Value(oss.str());
}

}} // namespaces


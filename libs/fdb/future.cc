//==========================================================================
// ObTools::FDB: future.cc
//
// FoundationDB API implementation: Future object access
//
// Copyright (c) 2021 Paul Clark.  All rights reserved
//==========================================================================

#include "ot-fdb.h"
#include "ot-log.h"

namespace ObTools { namespace FDB {

// Get a string value
string Future::get_string(const string& def)
{
  if (!future) return def;
  fdb_bool_t present;
  const uint8_t *value;
  int length;
  auto err = fdb_future_get_value(future, &present, &value, &length);
  if (err)
  {
    Log::Error log;
    log << "Failed to get string value: " << err << endl;
    return def;
  }

  if (!present) return def;
  return string(reinterpret_cast<const char *>(value), length);
}

}} // namespaces

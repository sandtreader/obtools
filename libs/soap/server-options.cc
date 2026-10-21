//==========================================================================
// ObTools::SOAP: server-options.cc
//
// HTTP(S) server options for SOAP Message Interfaces
//
// Copyright (c) 2026 Paul Clark.
//==========================================================================

#include "ot-soap.h"

namespace ObTools { namespace SOAP {

//--------------------------------------------------------------------------
// Read from a <server> or <ssl-server> element
void ServerOptions::read(const XML::Element& server_e)
{
  const auto positive = [&server_e](const char *attr, int& value)
  {
    const auto v = server_e.get_attr_int(attr, 0);
    if (v > 0) value = v;
  };

  positive("backlog", backlog);
  positive("max-threads", max_threads);
  positive("timeout", timeout);

  // min-spare may be 0
  const auto spare = server_e.get_attr_int("min-spare", -1);
  if (spare >= 0) min_spare = spare;
}

}} // namespaces

//==========================================================================
// ObTools::Net: host.cc
//
// Hostname access
//
// Copyright (c) 2023 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-net.h"
#include <unistd.h>

namespace ObTools { namespace Net {

//--------------------------------------------------------------------------
// Get hostname
string Host::get_hostname()
{
  const size_t HOST_NAME_MAX_LEN = 256;
  char hostname[HOST_NAME_MAX_LEN];

  if (gethostname(hostname, HOST_NAME_MAX_LEN) == 0)
    return hostname;

  throw runtime_error(string("Failed to get hostname: ")+ strerror(errno));
}

}} // namespaces


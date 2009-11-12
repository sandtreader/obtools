//==========================================================================
// ObTools::SSL: context.cc
//
// C++ wrapper for SSL application context
//
// Copyright (c) 2008 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-ssl.h"
#include "ot-log.h"
#include "ot-text.h"

namespace ObTools { namespace SSL {

//--------------------------------------------------------------------------
// Static:  Log SSL errors
void Context::log_errors(const string& text)
{
  Log::Streams log;
  log.error << "SSL: " << text << endl;
}

}} // namespaces




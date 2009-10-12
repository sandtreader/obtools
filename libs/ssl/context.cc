//==========================================================================
// ObTools::SSL: context.cc
//
// C++ wrapper for SSL application context
//
// Copyright (c) 2008 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
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




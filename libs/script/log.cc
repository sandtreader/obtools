//==========================================================================
// ObTools::Script: log.cc
//
// Log action
// <log level="N"/>text</log>
//
// Copyright (c) 2006 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-script.h"
#include "ot-log.h"

namespace ObTools { namespace Script {

//------------------------------------------------------------------------
// Run action
void LogAction::run()
{
  int level = xml.get_attr_int("level", Log::LEVEL_SUMMARY);
  Log::Message msg((Log::Level)level, xml.get_content());
  Log::logger.log(msg);
}


}} // namespaces

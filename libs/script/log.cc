//==========================================================================
// ObTools::Script: log.cc
//
// Log action
// <log level="N"/>text</log>
//
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
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

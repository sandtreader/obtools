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
bool LogAction::run(Context& con)
{
  int level = xml.get_attr_int("level", Log::LEVEL_SUMMARY);
  string text = xml.get_content();

  // Interpolate with context vars
  text = con.vars.interpolate(text);

  Log::Message msg((Log::Level)level, text);
  Log::logger.log(msg);

  return true;
}


}} // namespaces

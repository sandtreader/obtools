//==========================================================================
// ObTools::Script: set.cc
//
// Set variable action
// <set var="foo"/>content</log>
//
// Copyright (c) 2011 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-script.h"

namespace ObTools { namespace Script {

//------------------------------------------------------------------------
// Run action
void SetAction::run(Context& con)
{
  string var = xml["var"];
  if (!var.empty())
  {
    string text = xml.get_content();

    // Interpolate with context vars
    text = con.vars.interpolate(text);

    con.vars.add(var, text);
  }
}


}} // namespaces

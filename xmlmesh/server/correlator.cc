//==========================================================================
// ObTools::XMLMesh:Server correlator.cc
//
// Implementation of XMLMesh request-response correlator
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "server.h"
#include "ot-log.h"
#include "ot-text.h"
#include <sstream>

namespace ObTools { namespace XMLMesh { 

//------------------------------------------------------------------------
// Handle a request message - remembers it in correlation map, alters
// ID to own so we can find responses
void Correlator::handle_request(IncomingMessage& msg)
{
  CorrelatedRequest cr(msg.transport, msg.client, msg.message.get_id());

  // Create a new ID
  ostringstream ids;
  ids << "CREQ-" << ++id_serial;
  string new_id = ids.str();

  // !!! HTF are we going to subsititute this in the message??
}

//------------------------------------------------------------------------
// Handle a response message - looks up correlation and sends it back to
// the given client, modifying ID back to the original
void Correlator::handle_response(IncomingMessage& msg)
{

}

}} // namespaces





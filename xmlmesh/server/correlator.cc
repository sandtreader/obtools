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
// Note transport & client are where it is being sent on to - source
// is in msg.transport/client
void Correlator::handle_request(IncomingMessage& msg, Transport *transport,
				Net::EndPoint& client)
{
  // Grab old ID
  string old_id = msg.message.get_id();

  // Create a new ID
  ostringstream ids;
  ids << "CREQ-" << ++id_serial;
  string new_id = ids.str();

  // Create correlated request and enter in the cache
  Correlation *cr = new Correlation(msg.transport, msg.client, old_id,
				    transport, client, new_id);
  request_cache.add(new_id, cr);

  // Substitute our id in the message
  msg.message.set_id(new_id);

  // Log it
  Log::Detail << "Correlator: Opened correlation:\n  " << *cr << endl;
}

//------------------------------------------------------------------------
// Handle a response message - looks up correlation and sends it back to
// the given client, modifying ref back to the original
void Correlator::handle_response(IncomingMessage& msg)
{
  string our_ref = msg.message.get_ref();

  // Look it up
  Correlation *cr = request_cache.lookup(our_ref);
  if (cr)
  {
    Log::Detail << "Correlator: Found correlation:\n   " << *cr << endl;

    // Protect from spoofing by other clients
    if (cr->dest_transport == msg.transport && cr->dest_client == msg.client)
    {
      // Substitute original ID for ref
      msg.message.set_ref(cr->source_id);

      // Send fixed message to original client
      if (!server.send(msg.message, cr->source_transport, cr->source_client))
      {
	Log::Error << "Can't forward response to " 
		   << cr->source_transport->name 
		   << ":" << cr->source_client << endl;
      }

      // Remove it from cache
      request_cache.remove(our_ref);
    }
    else
    {
      Log::Error << "Correlator: Spoofed response to ID " << our_ref 
		 << " received from " << msg.transport->name
		 << ":" << msg.client << endl;
    }
  }
  else
  {
    Log::Error << "Can't find correlation for response ref:" 
	       << our_ref << endl;
  }
}

//------------------------------------------------------------------------
// Tick function - times out correlations
void Correlator::tick()
{
  request_cache.tidy();
}

//------------------------------------------------------------------------
// Correlation stream operator
ostream& operator<<(ostream&s, const Correlation& c)
{
  s << "[" << c.source_transport->name << ":" << c.source_client 
    << "(" << c.source_id << ") -> "
    << c.dest_transport->name << ":" << c.dest_client 
    << "(" << c.dest_id << ") ]";
  return s;
}

}} // namespaces





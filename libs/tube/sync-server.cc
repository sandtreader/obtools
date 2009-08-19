//==========================================================================
// ObTools::Tube: sync-server.cc
//
// Implementation of synchronous request/response server
// Really just a sugaring of the more general Server
//
// Copyright (c) 2007 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-tube.h"
#include "ot-log.h"

#include <sstream>

namespace ObTools { namespace Tube {

//------------------------------------------------------------------------
// Function to handle an incoming client message, called from parent
bool SyncServer::handle_message(ClientMessage& msg)
{
  switch (msg.action)
  {
    case ClientMessage::STARTED:
    case ClientMessage::FINISHED:
      // Pass to async handler
      return handle_async_message(msg);
      
    case ClientMessage::MESSAGE_DATA:
    {
      // Check flags
      if (msg.msg.flags & FLAG_RESPONSE_REQUIRED)
      {
	// Handle it as a synchronous request
	ClientMessage response(msg.client, 0);
	if (handle_request(msg, response.msg))
	{
	  // Fix up the response flags
	  response.msg.flags &=~ MASK_SYNC_FLAGS;
	  response.msg.flags |= FLAG_RESPONSE_PROVIDED;
	  response.msg.flags |= msg.msg.flags & MASK_REQUEST_ID;

	  // Send it back
	  send(response);
	}

	return true;
      }
      else
      {
	// Handle it as async
	return handle_async_message(msg);
      }
    }
  }

  return false;
}

//------------------------------------------------------------------------
// Function to handle asynchronous messages (not requiring a response)
// Implemented here just to log an error
bool SyncServer::handle_async_message(ClientMessage& msg)
{
  if (msg.action == ClientMessage::MESSAGE_DATA)
  {
    Log::Streams log;
    log.error << "Unwanted asynchronous message " << msg.msg.stag() 
	      << " received from " << msg.client << endl;
  }

  // Ignore STARTED, FINISHED

  return true;
}


}} // namespaces





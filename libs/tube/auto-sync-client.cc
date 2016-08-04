//==========================================================================
// ObTools::Tube: auto-sync-client.cc
//
// Implementation of synchronous request tube client with internal message
// dispatch thread - can only be used for synchronous requests
//
// Copyright (c) 2007 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-tube.h"
#include "ot-log.h"

namespace ObTools { namespace Tube {

//==========================================================================
// Background dispatch thread

//--------------------------------------------------------------------------
// Dispatch thread class
// Just repeatedly calls back into wait()
class DispatchThread: public MT::Thread
{
  AutoSyncClient& client;
  Log::Streams log;  // Thread local

  void run()
  {
    while (client.is_alive())
    {
      Message msg;
      client.wait(msg);

      // Pass it to async handler, if valid
      if (msg.is_valid()) client.handle_async_message(msg);
    }

    OBTOOLS_LOG_IF_DEBUG(log.debug << client.name
			 << " (dispatch): Thread shut down\n";)
  }

public:
  DispatchThread(AutoSyncClient &_client): client(_client) { start(); }
};

//------------------------------------------------------------------------
// Overrideable function to handle an asynchronous message - by default
// just errors and (if response required), sends back WHAT
void AutoSyncClient::handle_async_message(Message& msg)
{
  Log::Streams log;
  log.error << name << ": Non-response message "
	    << msg.stag() << " ignored\n";

  if (msg.flags & FLAG_RESPONSE_REQUIRED)
  {
    log.detail << "But response requested, so we'll oblige\n";
    Message response(0x4641494C,
		     "Unknown request",
		     FLAG_RESPONSE_PROVIDED | (msg.flags & MASK_REQUEST_ID));
    send(response);
  }
}

//==========================================================================
// Foreground stuff

//------------------------------------------------------------------------
// Constructor - takes server endpoint (address+port), request timeout
// (in seconds) and optional name
AutoSyncClient::AutoSyncClient(Net::EndPoint _server, int _timeout,
			       const string& _name):
  SyncClient(_server, _timeout, _name)
{
  dispatch_thread = new DispatchThread(*this);
}

//------------------------------------------------------------------------
// Constructor with SSL
AutoSyncClient::AutoSyncClient(Net::EndPoint _server, SSL::Context *_ctx,
			       int _timeout, const string& _name):
  SyncClient(_server, _ctx, _timeout, _name)
{
  dispatch_thread = new DispatchThread(*this);
}

//------------------------------------------------------------------------
// Shut down client cleanly
void AutoSyncClient::shutdown()
{
  if (alive)
  {
    SyncClient::shutdown();

    // Send an empty message to unblock dispatch thread
    Message msg;
    receive_q.send(msg);

    // Wait for thread to exit cleanly
    for(int i=0; i<5; i++)
    {
      if (!*dispatch_thread) break;
      this_thread::sleep_for(chrono::milliseconds{10});
    }

    // If still not dead, cancel it
    if (!!*dispatch_thread) dispatch_thread->cancel();
  }
}

//------------------------------------------------------------------------
// Destructor
AutoSyncClient::~AutoSyncClient()
{
  shutdown();

  // Now it's safe to delete the thread
  delete dispatch_thread;
}

}} // namespaces





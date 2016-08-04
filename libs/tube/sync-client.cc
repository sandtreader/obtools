//==========================================================================
// ObTools::Tube: sync-client.cc
//
// Implementation of synchronous request tube client
//
// Copyright (c) 2007-2010 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-tube.h"
#include "ot-log.h"

namespace ObTools { namespace Tube {

//==========================================================================
// Background timeout thread

//--------------------------------------------------------------------------
// Timeout thread class
// Just repeatedly calls back into do_timeouts()
class TimeoutThread: public MT::Thread
{
  SyncClient& client;
  Log::Streams log;  // Thread local

  void run()
  {
    while (client.is_alive())
    {
      client.do_timeouts(log);
      this_thread::sleep_for(chrono::milliseconds{10});
    }

    OBTOOLS_LOG_IF_DEBUG(log.debug << client.name
                         << " (timeout): Thread shut down\n";)
  }

public:
  TimeoutThread(SyncClient &_client): client(_client) { start(); }
};

//--------------------------------------------------------------------------
// Handle timeouts
void SyncClient::do_timeouts(Log::Streams& log)
{
  requests.do_timeouts(log, timeout, name);
}


//==========================================================================
// Foreground stuff

//--------------------------------------------------------------------------
// Constructor - takes server endpoint (address+port), request timeout
// (in seconds) and optional name
SyncClient::SyncClient(Net::EndPoint _server, int _timeout,
                       const string& _name):
  Client(_server, _name), timeout(_timeout)
{
  timeout_thread = new TimeoutThread(*this);
}

//--------------------------------------------------------------------------
// Constructor with SSL
SyncClient::SyncClient(Net::EndPoint _server, SSL::Context *_ctx,
                       int _timeout, const string& _name):
  Client(_server, _ctx, _name), timeout(_timeout)
{
  timeout_thread = new TimeoutThread(*this);
}

//--------------------------------------------------------------------------
// Request/response - blocks waiting for a response, or timeout/failure
// Returns whether a response was received, fills in response if so
bool SyncClient::request(Message& request, Message& response)
{
  // Set up request in cache
  requests.start_request(request, server, name);

  // Send it
  send(request);

  // Wait for response
  return requests.wait_response(request, response);
}

//--------------------------------------------------------------------------
// Override of wait() which filters out responses, while leaving async
// message to be returned normally
// NB!  poll() will still return true for responses, so wait() may block
bool SyncClient::wait(Message& msg)
{
  Log::Streams log;

  for(;;) // While processing responses
  {
    // Wait for any message, normally
    if (!Client::wait(msg)) return false;

    // Pass it to the request cache to see if it wants it - if it doesn't,
    // return it to the user
    if (!requests.handle_response(msg, name)) return true;

    // Request cache used it - loop for another one
  }
}

//--------------------------------------------------------------------------
// Shut down client cleanly
void SyncClient::shutdown()
{
  if (is_alive())
  {
    Client::shutdown();
    requests.shutdown();

    // Wait for timeout thread to exit cleanly
    for(int i=0; i<5; i++)
    {
      if (!*timeout_thread) break;
      this_thread::sleep_for(chrono::milliseconds{10});
    }

    // If still not dead, cancel it
    if (!!*timeout_thread) timeout_thread->cancel();
  }
}

//--------------------------------------------------------------------------
// Destructor
SyncClient::~SyncClient()
{
  shutdown();

  // Now it's safe to delete the thread
  delete timeout_thread;
}

}} // namespaces





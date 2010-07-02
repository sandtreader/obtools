//==========================================================================
// ObTools::Tube: bi-sync-server.cc
//
// Implementation of bidirectional synchronous request/response server
//
// Copyright (c) 2010 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-tube.h"
#include "ot-log.h"

#include <sstream>

// Time to wait (us) if send queue full
#define SEND_BUSY_WAIT_TIME 10000

namespace ObTools { namespace Tube {

//==========================================================================
// Background timeout thread

//--------------------------------------------------------------------------
// Timeout thread class
// Just repeatedly calls back into do_timeouts()
class TimeoutThread: public MT::Thread
{
  BiSyncServer& server;
  Log::Streams log;  // Thread local

  void run() 
  { 
    while (server.is_alive())
    {
      server.do_timeouts(log);
      MT::Thread::usleep(10000);
    }

    OBTOOLS_LOG_IF_DEBUG(log.debug << server.name
			 << " (timeout): Thread shut down\n";)
  }

public:
  TimeoutThread(BiSyncServer &_server): server(_server) { start(); }
};

//------------------------------------------------------------------------
// Handle timeouts
void BiSyncServer::do_timeouts(Log::Streams& log)
{
  requests.do_timeouts(log, request_timeout, name);
}

//==========================================================================
// Foreground stuff

//------------------------------------------------------------------------
// Constructors - as Server but with timeout
BiSyncServer::BiSyncServer(int port, int _request_timeout, 
			   const string& _name, 
			   int backlog, int min_spare_threads, 
			   int max_threads, int _client_timeout):
  SyncServer(port, _name, backlog, min_spare_threads, max_threads, 
	     _client_timeout),
  request_timeout(_request_timeout)
{
  timeout_thread = new TimeoutThread(*this);
}

BiSyncServer::BiSyncServer(Net::EndPoint local, int _request_timeout,
			   const string& _name, int backlog, 
			   int min_spare_threads, int max_threads,
			   int _client_timeout):
  SyncServer(local, _name, backlog, min_spare_threads, max_threads,
	     _client_timeout),
  request_timeout(_request_timeout)
{
  timeout_thread = new TimeoutThread(*this);
}

BiSyncServer::BiSyncServer(SSL::Context *_ctx, int port, 
			   int _request_timeout, 
			   const string& _name, int backlog, 
			   int min_spare_threads, int max_threads,
			   int _client_timeout):
  SyncServer(_ctx, port, _name, backlog, min_spare_threads, max_threads,
	     _client_timeout),
  request_timeout(_request_timeout)
{
  timeout_thread = new TimeoutThread(*this);
}

BiSyncServer::BiSyncServer(SSL::Context *_ctx, Net::EndPoint local, 
			   int _request_timeout, const string& _name, 
			   int backlog, int min_spare_threads, 
			   int max_threads, int _client_timeout):
  SyncServer(_ctx, local, _name, backlog, min_spare_threads, max_threads,
	     _client_timeout),
  request_timeout(_request_timeout)
{
  timeout_thread = new TimeoutThread(*this);
}

//------------------------------------------------------------------------
// Request/response - blocks waiting for a response, or timeout/failure
// Returns whether a response was received, fills in response if so
bool BiSyncServer::request(ClientMessage& request, Message& response)
{
  // Start request in our request cache, to establish ID and set state
  // for response
  requests.start_request(request.msg, request.client.address, name);

  // Lookup session by client address
  ClientSession *cs = 0;
  {
    MT::RWReadLock lock(client_sessions.mutex);
    map<Net::EndPoint, ClientSession *>::iterator p 
      = client_sessions.sessions.find(request.client.address);
    if (p != client_sessions.sessions.end())
    {
      cs = p->second;

      // Send it (this duplicates code in Server::send but we've got the
      // session already so this saves looking it up again)
      while (cs->send_q.waiting() > max_send_queue) // Zero must work
	MT::Thread::usleep(SEND_BUSY_WAIT_TIME);
      
      cs->send_q.send(request.msg);
    }
  }

  // Wait for response
  return requests.wait_response(request.msg, response);
}

//------------------------------------------------------------------------
// Handle asynchronous messages, which includes responses
bool BiSyncServer::handle_async_message(ClientMessage& msg)
{
  // Check it's a response before going to the trouble of
  // looking up the session
  if (msg.action == ClientMessage::MESSAGE_DATA
      && (msg.msg.flags & FLAG_RESPONSE_PROVIDED))
  {
    requests.handle_response(msg.msg, name);  // Must handle it
    return true;  // Snaffle it
  }
  else if (msg.action == ClientMessage::FINISHED)
  {
    // Shut down and fail all waiting requests for this client
    requests.shutdown(msg.client.address);
  }

  // Not a response - pass down another level
  return handle_client_async_message(msg);
}

//------------------------------------------------------------------------
// Handle asynchronous messages which aren't responses
bool BiSyncServer::handle_client_async_message(ClientMessage& msg)
{
  // Call SyncServer's original handle_async_message which just logs it
  return SyncServer::handle_async_message(msg);
}

//------------------------------------------------------------------------
// Shut down server cleanly
void BiSyncServer::shutdown()
{
  if (is_alive())
  {
    Server::shutdown();

    // Shut down requests in all sessions
    requests.shutdown();

    // Wait for timeout thread to exit cleanly
    for(int i=0; i<5; i++)
    {
      if (!*timeout_thread) break;
      MT::Thread::usleep(10000);
    }

    // If still not dead, cancel it
    if (!!*timeout_thread) timeout_thread->cancel();
  }
}

//------------------------------------------------------------------------
// Destructor
BiSyncServer::~BiSyncServer()
{
  shutdown();

  // Now it's safe to delete the thread
  delete timeout_thread;
}

}} // namespaces





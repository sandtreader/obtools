//==========================================================================
// ObTools::Tube: sync-client.cc
//
// Implementation of synchronous request tube client
//
// Copyright (c) 2007 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-tube.h"
#include "ot-log.h"

#include <unistd.h>
#include <sstream>

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
      MT::Thread::usleep(10000);
    }

    OBTOOLS_LOG_IF_DEBUG(log.debug << client.name 
			 << " (timeout): Thread shut down\n";)
  }

public:
  TimeoutThread(SyncClient &_client): client(_client) { start(); }
};

//------------------------------------------------------------------------
// Handle timeouts
void SyncClient::do_timeouts(Log::Streams& log)
{
  MT::Lock lock(request_mutex); 
  Time::Stamp now = Time::Stamp::now();
  for(map<id_t, Request>::iterator p = requests.begin(); 
      p!=requests.end(); ++p)
  {
    Request& req = p->second;

    if ((now-req.started).seconds() >= timeout)
    {
      log.summary << name << ": Request " << (int)p->first << " timed out\n";
      req.ready.signal();  // Response is still invalid
    }
  }
}

//==========================================================================
// Foreground stuff

//------------------------------------------------------------------------
// Constructor - takes server endpoint (address+port), request timeout
// (in seconds) and optional name
SyncClient::SyncClient(Net::EndPoint _server, int _timeout, 
		       const string& _name):
  Client(_server, _name), timeout(_timeout), request_id(0)
{
  timeout_thread = new TimeoutThread(*this);
}

//------------------------------------------------------------------------
// Request/response - blocks waiting for a response, or timeout/failure
// Returns whether a response was received, fills in response if so
bool SyncClient::request(Message& request, Message& response)
{
  Log::Streams log;

  // Fix the flags on the request
  request.flags |= FLAG_RESPONSE_REQUIRED;
  request.flags |= request_id << SHIFT_REQUEST_ID;

  // Send it
  if (!send(request)) return false;

  // Lock mutex from here in, but cv.wait() unlocks it during wait
  MT::Lock lock(request_mutex); 

  // Add a request record
  id_t id = request_id++;

  OBTOOLS_LOG_IF_DEBUG(log.debug << name << ": Sending request ID " << (int)id 
		       << " - " << request.stag() << endl;)

  Request& req = requests[id] = Request();

  // Wait for signal, unlocking and then relocking mutex
  req.ready.wait(request_mutex);

  // Copy response 
  response = req.response;

  // Remove request record
  requests.erase(id);

  // Return true if response is valid (can be invalid on thread shutdown)
  return response.is_valid();
}

//------------------------------------------------------------------------
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

    // Check if it's a response
    if (msg.flags & FLAG_RESPONSE_PROVIDED)
    {
      // Get ID
      id_t id = (id_t)((msg.flags & MASK_REQUEST_ID)>>SHIFT_REQUEST_ID);

      OBTOOLS_LOG_IF_DEBUG(log.debug << name 
			   << ": Got response message for ID " 
			   << (int)id << " - " << msg.stag() << endl;)

      // Lookup ID in mutex
      MT::Lock lock(request_mutex); 
      map<id_t, Request>::iterator p = requests.find(id);
      if (p!=requests.end())
      {
	Request& req = p->second;

	// Copy message into response and signal readiness
	req.response = msg;
	req.ready.signal();
      }
      else
      {
	log.error << name << ": Response for unknown ID " << id 
		  << " - " << msg.stag() << endl;
      }

      // Either way, drop this message, loop and get another one
    }
    else
    {
      // It's an asynch message - just return it to caller
      return true; 
    }
  }
}

//------------------------------------------------------------------------
// Shut down client cleanly
void SyncClient::shutdown()
{
  if (alive)
  {
    Client::shutdown();

    // Signal all request conditions to free up requesting threads
    MT::Lock lock(request_mutex); 
    for(map<id_t, Request>::iterator p = requests.begin(); 
	p!=requests.end(); ++p)
    {
      Request& req = p->second;
      req.ready.signal();  // Leaving empty response
    }

    // Wait for thread to exit cleanly
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
SyncClient::~SyncClient()
{
  shutdown();

  // Now it's safe to delete the thread
  delete timeout_thread;
}

}} // namespaces





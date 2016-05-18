//==========================================================================
// ObTools::Tube: sync-request.cc
//
// Implementation of synchronous request cache
//
// Copyright (c) 2010 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-tube.h"
#include "ot-log.h"

namespace ObTools { namespace Tube {

//------------------------------------------------------------------------
// Handle timeouts
void SyncRequestCache::do_timeouts(Log::Streams& log, int timeout,
				   const string& name)
{
  MT::Lock lock(request_mutex); 
  Time::Stamp now = Time::Stamp::now();
  for(map<id_t, Request>::iterator p = requests.begin(); 
      p!=requests.end(); ++p)
  {
    Request& req = p->second;

    if ((now-req.started).seconds() >= timeout)
    {
      log.summary << name << ": Request " << static_cast<int>(p->first)
                  << " timed out\n";
      req.ready.notify_one();  // Response is still invalid
    }
  }
}

//------------------------------------------------------------------------
// Set up a request entry to wait for a response
// (call before actually sending message, in case response is instant)
void SyncRequestCache::start_request(Message& request, 
				     Net::EndPoint client,
				     const string& name)
{
  Log::Streams log;

  MT::Lock lock(request_mutex); 

  // Get a new ID and increment counter
retry:
  id_t id = request_id;
  request_id = (request_id+1) & MAX_REQUEST_ID;

  // Check if it already exists - this shouldn't happen unless the counter
  // has wrapped round because requests are being held up for ages at the
  // server.  Note if all possible IDs are used and blocked, this will
  // lock up!
  if (requests.find(id) != requests.end())
  {
    log.error << name << ": Warning - duplicate ID " << id << " skipped\n";
    goto retry;
  }

  OBTOOLS_LOG_IF_DEBUG(log.debug << name << ": Sending request ID "
                                 << static_cast<int>(id)
                                 << " - " << request.stag() << endl;)

  // Start the clock
  requests.emplace(piecewise_construct, forward_as_tuple(id),
                   forward_as_tuple(client));

  // Fix the flags on the request
  request.flags |= FLAG_RESPONSE_REQUIRED;
  request.flags |= id << SHIFT_REQUEST_ID;
}

//------------------------------------------------------------------------
// Block waiting for a response to the given request
// Returns whether valid response received
bool SyncRequestCache::wait_response(const Message& request,
                                     Message& response)
{
  // Lock mutex - wait will unlock it then relock on exit
  MT::Lock lock(request_mutex); 
  id_t id = static_cast<id_t>((request.flags & MASK_REQUEST_ID)
                              >> SHIFT_REQUEST_ID);
  Request& req = requests[id];

  // Wait for signal, unlocking and then relocking mutex
  req.ready.wait(lock);

  // Copy response 
  response = req.response;

  // Remove request record
  requests.erase(id);

  // Return true if response is valid (can be invalid on thread shutdown)
  return response.is_valid();
}

//------------------------------------------------------------------------
// Handle a response - returns true if it was recognised as a response
// to one of our requests, false if it is a new message from the other side
bool SyncRequestCache::handle_response(const Message& response,
                                       const string& name)
{
  Log::Streams log;

  // Check if it's a response
  if (response.flags & FLAG_RESPONSE_PROVIDED)
  {
    // Get ID
    id_t id = static_cast<id_t>((response.flags & MASK_REQUEST_ID)
                                >>SHIFT_REQUEST_ID);

    OBTOOLS_LOG_IF_DEBUG(log.debug << name
                                   << ": Got response message for ID "
                                   << static_cast<int>(id) << " - "
                                   << response.stag() << endl;)

    // Lookup ID in mutex
    MT::Lock lock(request_mutex); 
    map<id_t, Request>::iterator p = requests.find(id);
    if (p!=requests.end())
    {
      Request& req = p->second;

      // Copy message into response and signal readiness
      req.response = response;
      req.ready.notify_one();
    }
    else
    {
      log.error << name << ": Response for unknown ID " << static_cast<int>(id)
		<< " - " << response.stag() << endl;
    }

    // Either way, we handled it
    return true;
  }
  else return false;  // Let caller handle it
}

//------------------------------------------------------------------------
// Shut down cleanly for a specific client
void SyncRequestCache::shutdown(Net::EndPoint client)
{
  // Signal all request conditions to free up requesting threads
  MT::Lock lock(request_mutex); 
  for(map<id_t, Request>::iterator p = requests.begin(); 
      p!=requests.end(); ++p)
  {
    Request& req = p->second;
    if (req.client == client) req.ready.notify_one();  // Leaving empty response

    // The waiting thread will delete the request
  }
}

//------------------------------------------------------------------------
// Shut down cleanly for all clients
void SyncRequestCache::shutdown()
{
  // Signal all request conditions to free up requesting threads
  {
    MT::Lock lock(request_mutex); 
    for(map<id_t, Request>::iterator p = requests.begin(); 
	p!=requests.end(); ++p)
    {
      Request& req = p->second;
      req.ready.notify_one();  // Leaving empty response
      
      // The waiting thread will delete the request
    }
  }
}

//------------------------------------------------------------------------
// Destructor
SyncRequestCache::~SyncRequestCache()
{
  shutdown();

  // Now wait for all requests to be deleted before finally deleting myself,
  // to ensure nothing is waiting for the shutdown
  while (requests.size())
    this_thread::sleep_for(chrono::milliseconds{10});
}

}} // namespaces





//==========================================================================
// ObTools::XMLMesh:Server correlator.cc
//
// Implementation of XMLMesh request-response correlator
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "server.h"
#include "ot-log.h"
#include "ot-text.h"
#include <sstream>

namespace ObTools { namespace XMLMesh { 

//==========================================================================
// Request-response correlation
struct Correlation
{
  string id;                     // Request message ID
  Client client;                 // Client we're acting for
  string source_path;            // Original path for the request 

  Correlation(const string& _id,
	      Client& _client,
	      const string& _source_path):
    id(_id), client(_client), source_path(_source_path) {}
};

ostream& operator<<(ostream&s, const Correlation& c);

//==========================================================================
// Correlator service
class Correlator: public Service
{
private:
  static const int DEFAULT_TIMEOUT = 60;

  unsigned long id_serial; 

  // Cache: map of our ID to correlation
  typedef Cache::UseTimeoutPointerCache<string, Correlation> CacheType;
  CacheType request_cache;

public:
  //------------------------------------------------------------------------
  // Constructor 
  Correlator(XML::Element& cfg);

  //------------------------------------------------------------------------
  // Signal various global events, independent of message routing
  void signal(Signal sig, Client& client);

  //------------------------------------------------------------------------
  // Implementation of Service virtual interface - q.v. server.h
  bool handle(RoutingMessage& msg);

  //------------------------------------------------------------------------
  // Tick function - times out correlations
  void tick();
};

//------------------------------------------------------------------------
// Default Constructor 
Correlator::Correlator(XML::Element& cfg): 
    Service(cfg), 
    id_serial(0), 
    request_cache(cfg.get_attr_int("timeout", DEFAULT_TIMEOUT)) 
{
  Log::Summary << "Correlator Service '" << id << "' started\n"; 
}

//------------------------------------------------------------------------
// Implementation of Service virtual interface - q.v. server.h
bool Correlator::handle(RoutingMessage& msg)
{
  // Work out if it's a response or not
  string our_ref = msg.message.get_ref();

  // Look at responses going in either direction - they may be generated
  // by external clients with forward routing, or our own services with
  // reverse routing.  Since we turn them round and force reverse routing
  // anyway, we won't see them twice.
  if (our_ref.size())
  {
    // It's a response
    // Look it up
    Correlation *cr = request_cache.lookup(our_ref);
    if (cr)
    {
      Log::Detail << "Correlator: Found correlation:\n  " << *cr << endl;
      
      // Substitute original path and make it a response so it goes into
      // reverse routing
      msg.path = MessagePath(cr->source_path);
      msg.reversing = true;

      // Remove it from cache
      request_cache.remove(our_ref);
    }
    else
    {
      Log::Error << "Can't find correlation for response ref:" 
		 << our_ref << endl;
      return false;
    }
  }
  // Only look at original requests, before they get
  // reversed by the publisher - otherwise we'll generate two 
  // correlations for each one
  else if (!msg.reversing)
  {
    // It's a request
    // Check for rsvp
    if (msg.message.get_rsvp())
    {
      // Grab ID
      string id = msg.message.get_id();

      // Create correlated request and enter in the cache
      Correlation *cr = new Correlation(id, msg.client, msg.path.to_string());
      request_cache.add(id, cr);
      
      // Log it
      Log::Detail << "Correlator: Opened correlation:\n  " << *cr << endl;
    }
  }

  return true;  // Allow it to be forwarded/reversed
}

//------------------------------------------------------------------------
// Signal various global events, independent of message routing
void Correlator::signal(Signal sig, Client& client)
{
  switch (sig)
  {
    case Service::CLIENT_STARTED:
      // Ignore for now
      break;

    case Service::CLIENT_FINISHED:
    {
      // Check cache for any with this client, and delete it
      for(CacheType::iterator p = request_cache.begin();
	  p!=request_cache.end();
	  )
      {
	CacheType::iterator q = p++;  // Protect while deleting
	if (client == q->client)
	{
	  Log::Summary << "Deleted correlation " << *q 
		       << " for finished client\n";
	  request_cache.remove(q.id());
	}
      }

      break;
    }
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
  s << "[" << c.id << " -> " << c.source_path << " for " 
    << c.client.client << " ]";
  return s;
}

//==========================================================================
// Auto-register
OT_XMLMESH_REGISTER_SERVICE(Correlator, "correlator");

}} // namespaces





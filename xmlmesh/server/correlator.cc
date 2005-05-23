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

class Correlator;   // Forward

//==========================================================================
// Request-response correlation
// Implements MessageTracker interface
struct Correlation: public MessageTracker
{
  Correlator& correlator;        // Ref back to owning correlator
  string id;                     // Request message ID
  ServiceClient client;          // Client we're acting for
  string source_path;            // Original path for the request 
  list<RoutingMessage *> copies; // List of copies of message
  bool forwarded;                // Whether forwarded 
  bool replied;                  // Whether replied

  //------------------------------------------------------------------------
  // Constructor
  Correlation(Correlator& _corr, const string& _id,
	      ServiceClient& _client,
	      const string& _source_path):
    correlator(_corr), id(_id), client(_client), source_path(_source_path),
    forwarded(false), replied(false) {}

  //------------------------------------------------------------------------
  // MessageTracker interface implementations
  void notify_forwarded(RoutingMessage *) { forwarded = true; }
  void attach(RoutingMessage *msg) { copies.push_back(msg); }
  void detach(RoutingMessage *msg);

  //------------------------------------------------------------------------
  // Internal notification
  void notify_replied() { replied = true; }

  //------------------------------------------------------------------------
  // Destructor
  ~Correlation();
};

//==========================================================================
// Correlator service
class Correlator: public Service
{
private:
  static const int DEFAULT_TIMEOUT = 60;

  // Cache: map of our ID to correlation
  typedef Cache::UseTimeoutPointerCache<string, Correlation> CacheType;
  CacheType request_cache;

public:
  //------------------------------------------------------------------------
  // Constructor 
  Correlator(XML::Element& cfg);

  //------------------------------------------------------------------------
  // Signal various global events, independent of message routing
  void signal(Signal sig, ServiceClient& client);

  //------------------------------------------------------------------------
  // Implementation of Service virtual interface - q.v. server.h
  bool handle(RoutingMessage& msg);

  //------------------------------------------------------------------------
  // Tick function - times out correlations
  void tick();

  //------------------------------------------------------------------------
  // Handle orphan messages (messages that cannot now be replied to)
  void handle_orphan(const string& id, ServiceClient& client, 
		     Correlation *cr);
};


//==========================================================================
// Correlation implementation

//------------------------------------------------------------------------
// Correlation stream operator
ostream& operator<<(ostream&s, const Correlation& c)
{
  s << "[" << c.id << " -> " << c.source_path << " for " 
    << c.client.client << " ]";
  return s;
}

//------------------------------------------------------------------------
// Detach a copy of a message (before it dies)
void Correlation::detach(RoutingMessage *msg)
{
  // Remove this message from the list
  copies.remove(msg);

  // If now empty, and not forwarded or replied, there will be no response,
  // so get correlator to fail it
  if (copies.empty() && !forwarded && !replied) 
  {
    correlator.handle_orphan(id, client, this);

    // Set replied so we don't do it again when the correlation times out
    replied = true;
  }
}

//------------------------------------------------------------------------
// Destructor
Correlation::~Correlation()
{
  // Make sure we are cut loose from any messages which are still
  // being tracked by us
  while (!copies.empty())
  {
    RoutingMessage *msg = copies.front();
    copies.pop_front();
    msg->tracker = 0;
  }

  // If not replied, we must send back an orphan error
  if (!replied) correlator.handle_orphan(id, client, this);
}

//==========================================================================
// Correlator implementation

//------------------------------------------------------------------------
// Default Constructor 
Correlator::Correlator(XML::Element& cfg): 
    Service(cfg), 
    request_cache(cfg.get_attr_int("timeout", DEFAULT_TIMEOUT)) 
{
  log.summary << "Correlator Service '" << id << "' started\n"; 
}

//------------------------------------------------------------------------
// Implementation of Service virtual interface - q.v. server.h
bool Correlator::handle(RoutingMessage& msg)
{
  Log::Streams tlog;   // Thread-safe log

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
      tlog.detail << "Correlator: Found correlation:\n  " << *cr << endl;

      // Create new copy message to be re-originated here
      ServiceClient client(this, msg.client.client);
      MessagePath path(cr->source_path);
      RoutingMessage newmsg(client, msg.message, path);
      originate(newmsg);

      // Notify reply and remove it from cache
      // (will detach itself from messages)
      cr->notify_replied();
      request_cache.remove(our_ref);

      // Don't continue with this message in normal routing
      return false;
    }
    else
    {
      tlog.error << "Can't find correlation for response ref:" 
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
      Correlation *cr = new Correlation(*this, id, msg.client, 
					msg.path.to_string());
      request_cache.add(id, cr);

      // Put the correlation in as a message tracker so we can trace what
      // happens to this message
      msg.track(cr);

      // Log it
      tlog.detail << "Correlator: Opened correlation:\n  " << *cr << endl;
    }
  }

  return true;  // Allow it to be forwarded/reversed
}

//------------------------------------------------------------------------
// Signal various global events, independent of message routing
void Correlator::signal(Signal sig, ServiceClient& client)
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
	  Log::Stream summary_log(Log::logger, Log::LEVEL_SUMMARY);

	  summary_log << "Deleted correlation " << *q 
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
// Handle orphan messages (messages that cannot now be replied to)
void Correlator::handle_orphan(const string& id, ServiceClient& client, 
			       Correlation *cr)
{
  log.error << "Correlation " << *cr << " orphaned with no response\n";

  FaultMessage response(id, SOAP::Fault::CODE_RECEIVER, 
			"Nothing to handle this request");
  ServiceClient newclient(this, client.client);
  MessagePath path(cr->source_path);
  RoutingMessage newmsg(newclient, response, path);
  originate(newmsg);
}


//==========================================================================
// Auto-register
OT_XMLMESH_REGISTER_SERVICE(Correlator, "correlator");

}} // namespaces





//==========================================================================
// ObTools::XMLMesh:Server correlator.cc
//
// Implementation of XMLMesh request-response correlator
//
// Copyright (c) 2003-2015 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
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
  string source_path;            // Original path for the request
  list<RoutingMessage *> copies; // List of copies of message
  bool forwarded;                // Whether forwarded
  bool replied;                  // Whether replied

  //------------------------------------------------------------------------
  // Constructor
  Correlation(Correlator& _corr, const string& _id,
	      const string& _source_path):
    correlator(_corr), id(_id), source_path(_source_path),
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
  // Implementation of Service virtual interface - q.v. server.h
  bool handle(RoutingMessage& msg);

  //------------------------------------------------------------------------
  // Tick function - times out correlations
  void tick();

  //------------------------------------------------------------------------
  // Handle orphan messages (messages that cannot now be replied to)
  void handle_orphan(const string& id, Correlation *cr);
};


//==========================================================================
// Correlation implementation

//------------------------------------------------------------------------
// Correlation stream operator
ostream& operator<<(ostream&s, const Correlation& c)
{
  s << "[" << c.id << " -> " << c.source_path << "]";
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
    correlator.handle_orphan(id, this);

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
  if (!replied) correlator.handle_orphan(id, this);
}

//==========================================================================
// Correlator implementation

//------------------------------------------------------------------------
// Default Constructor
Correlator::Correlator(XML::Element& cfg):
    Service(cfg),
    request_cache(cfg.get_attr_int("timeout", DEFAULT_TIMEOUT))
{
  Log::Summary log;
  log << "Correlator Service '" << id << "' started\n";
}

//------------------------------------------------------------------------
// Implementation of Service virtual interface - q.v. server.h
bool Correlator::handle(RoutingMessage& msg)
{
  Log::Streams log;

  switch (msg.type)
  {
    case RoutingMessage::CONNECTION:
      break;

    case RoutingMessage::MESSAGE:
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
        // Look it up and detach it
        Correlation *cr = request_cache.detach(our_ref);
        if (cr)
        {
          log.detail << "Correlator: Found correlation:\n  " << *cr << endl;

          // Create new copy message to be re-originated here
          MessagePath path(cr->source_path);
          RoutingMessage newmsg(msg.message, path);
          originate(newmsg);

          // Notify reply and delete correlation
          // (will detach itself from messages)
          cr->notify_replied();
          delete cr;

          // Don't continue with this message in normal routing
          return false;
        }
        else
        {
          log.error << "Can't find correlation for response ref:"
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
          Correlation *cr = new Correlation(*this, id, msg.path.to_string());
          request_cache.add(id, cr);

          // Put the correlation in as a message tracker so we can trace what
          // happens to this message
          msg.track(cr);

          // Log it
          log.detail << "Correlator: Opened correlation:\n  " << *cr << endl;
        }
      }
    }
    break;

    case RoutingMessage::DISCONNECTION:
    {
      // Check cache for any with this path, and delete them
      // Note we need write lock even for scan because we may need to convert
      string msg_path = msg.path.to_string();
      MT::RWWriteLock lock(request_cache.mutex);
      for(CacheType::iterator p = request_cache.begin();
	  p!=request_cache.end();
	  )
      {
	CacheType::iterator q = p++;  // Protect while deleting
	if (msg_path == q->source_path)
	{
	  log.summary << "Deleted correlation " << *q
                      << " for disconnected client\n";
	  request_cache.remove(q.id());
	}
      }
    }
    break;
  }

  return true;  // Allow it to be forwarded/reversed
}

//------------------------------------------------------------------------
// Tick function - times out correlations
void Correlator::tick()
{
  request_cache.tidy();
}

//------------------------------------------------------------------------
// Handle orphan messages (messages that cannot now be replied to)
void Correlator::handle_orphan(const string& id, Correlation *cr)
{
  Log::Error log;
  log << "Correlation " << *cr << " orphaned with no response\n";

  FaultMessage response(id, SOAP::Fault::CODE_RECEIVER,
			"Nothing to handle this request");
  MessagePath path(cr->source_path);
  RoutingMessage newmsg(response, path);
  originate(newmsg);
}


//==========================================================================
// Auto-register
OT_XMLMESH_REGISTER_SERVICE(Correlator, "correlator");

}} // namespaces





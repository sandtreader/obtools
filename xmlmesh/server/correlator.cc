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
#include <unistd.h>   // <--- add

namespace ObTools { namespace XMLMesh {

class Correlator;   // Forward

//==========================================================================
// Request-response correlation
// Implements MessageTracker interface
struct Correlation: public MessageTracker
{
  Correlator& correlator;        // Ref back to owning correlator
  const string id;               // Request message ID
  const string source_path;      // Original path for the request
  list<RoutingMessage *> copies; // List of copies of message
  bool forwarded;                // Whether forwarded
  bool replied;                  // Whether replied
  bool client_disconnected; 

  //------------------------------------------------------------------------
  // Constructor
  Correlation(Correlator& _corr, const string& _id,
              const string& _source_path):
    correlator(_corr), id(_id), source_path(_source_path),
    forwarded(false), replied(false), client_disconnected(false) {}

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
  Correlator(const XML::Element& cfg);

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

//--------------------------------------------------------------------------
// Correlation stream operator
ostream& operator<<(ostream&s, const Correlation& c)
{
  s << "[" << c.id << " -> " << c.source_path << "]";
  return s;
}

//--------------------------------------------------------------------------
// Detach a copy of a message (before it dies)
void Correlation::detach(RoutingMessage *msg)
{
  Log::Error log;
  log << "Detach message 2222 '" << msg << "' \n";
  // Remove this message from the list
  copies.remove(msg);

  // If now empty, and not forwarded or replied, there will be no response,
  // so get correlator to fail it
  if (copies.empty() && !forwarded && !replied)
  {
    Log::Error log;
    log << "handle_orphan 2222 \n";
    correlator.handle_orphan(id, this);

    // Set replied so we don't do it again when the correlation times out
    replied = true;
  }
}

//--------------------------------------------------------------------------
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
  if (!replied) {
    Log::Error log;
    log << "handle_orphan 1111 \n";
    correlator.handle_orphan(id, this);
  }
}

//==========================================================================
// Correlator implementation

//--------------------------------------------------------------------------
// Default Constructor
Correlator::Correlator(const XML::Element& cfg):
    Service(cfg),
    request_cache(cfg.get_attr_int("timeout", DEFAULT_TIMEOUT))
{
  Log::Summary log;
  log << "Correlator Service '" << id << "' started\n";
}

//--------------------------------------------------------------------------
// Implementation of Service virtual interface - q.v. server.h
bool Correlator::handle(RoutingMessage& msg)
{
  Log::Streams log;

  log.detail << "Correlator::handle " << msg.type << "\n";

  switch (msg.type)
  {
    case RoutingMessage::CONNECTION:
      break;

    case RoutingMessage::MESSAGE:
    {

      log.detail << "Correlator: msg.ptr=" << &msg
            << " path='" << msg.path.to_string()
            << "' reversing=" << (msg.reversing ? "true" : "false")
            << " rsvp=" << (msg.message.get_rsvp() ? "true" : "false")
            << "\n";

      // Work out if it's a response or not
      const string our_ref = msg.message.get_ref();

      // Look at responses going in either direction - they may be generated
      // by external clients with forward routing, or our own services with
      // reverse routing.  Since we turn them round and force reverse routing
      // anyway, we won't see them twice.
      if (our_ref.size())
      {
        log.detail << "Correlator: Message has ref: " << our_ref << endl;
        // It's a response
        // Look it up and detach it
        log.detail << "Correlator: response ref='" << our_ref << "'" << " message:\n"
           << msg.message.get_text() << endl;
        Correlation *cr = request_cache.detach(our_ref);
        if (cr)
        {
          log.detail << "Correlator: Found correlation:\n  " << *cr << endl;

          // Create new copy message to be re-originated here
          const MessagePath path(cr->source_path);
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
          
          // Dump incoming ref details and cache contents
          std::ostringstream o;
          for(unsigned char c: our_ref) { o << std::hex << (int)c << " "; }
          log.error << "Can't find correlation for response ref:" << our_ref
                    << " len=" << our_ref.size() << " hex=" << o.str()
                    << " PID=" << getpid() << endl;
          // Dump cache keys
          MT::RWReadLock lock(request_cache.mutex);
          int cache_count = 0;
         for(CacheType::iterator p = request_cache.begin(); p!=request_cache.end(); ++p) {
            ++cache_count;
            log.error << "  cache key='" << p.id() << "'\n";
          }
          log.error << "  cache size=" << cache_count << " PID=" << getpid() << endl;
          return false;
        
        }
      }
      // Only look at original requests, before they get
      // reversed by the publisher - otherwise we'll generate two
      // correlations for each one
      else if (!msg.reversing)
      {
        log.detail << "Correlator: Message has no ref\n";
        // It's a request
        // Check for rsvp
        if (msg.message.get_rsvp())
        {
          // Grab ID
          const string id = msg.message.get_id();

          // Create correlated request and enter in the cache
          Correlation *cr = new Correlation(*this, id, msg.path.to_string());

          log.detail << "Correlator: opening correlation for id=" << id
           << " path=" << msg.path.to_string() << endl;

          // Debug: dump id length and hex
          {
            std::ostringstream o; 
            for(unsigned char c: id) { o << std::hex << (int)c << " "; }
            log.detail << "DEBUG id.len=" << id.size() << " hex=" << o.str() << endl;
          }

          // Avoid race / duplicate correlations:
          // If a correlation for this id already exists, attach this message
          // to the existing correlation and use that instead of creating a
          // second entry. Use a write lock while inspecting/modifying cache.
          {
            MT::RWWriteLock lock(request_cache.mutex);
            bool reused = false;
            for(CacheType::iterator p = request_cache.begin(); p != request_cache.end(); ++p)
            {
              if (p.id() == id)
              {
                Correlation *existing = &*p;
                log.detail << "Correlator: Found existing correlation for id=" << id << ", reusing\n";
                existing->attach(&msg);
                msg.track(existing);
                reused = true;
                break;
              }
            }

            if (!reused)
            {
              // Ensure the correlation knows about this live message immediately
              cr->attach(&msg);
              request_cache.add(id, cr);
              msg.track(cr);
            }
          }

          // Log cache contents and PID for debugging
          {
            Log::Summary clog;
            clog << "Correlator PID=" << getpid() << " cache keys after add:";
            MT::RWReadLock lock(request_cache.mutex);
            for(CacheType::iterator p = request_cache.begin(); p!=request_cache.end(); ++p)
              clog << " '" << p.id() << "'";
            clog << "\n";
          }

          // Put the correlation in as a message tracker so we can trace what
          // happens to this message
          //msg.track(cr);

          // Log it
          log.detail << "Correlator: Opened correlation:\n  " << *cr << endl;
        }else{
          log.detail << "Correlator: Message is not RSVP, no correlation opened\n";
        }
      }
    }
    break;

    case RoutingMessage::DISCONNECTION:
    {
      // Check cache for any with this path, and mark them disconnected
      const string msg_path = msg.path.to_string();
      MT::RWWriteLock lock(request_cache.mutex);
      for(CacheType::iterator p = request_cache.begin();
          p!=request_cache.end();
          ++p)
      {
        CacheType::iterator q = p;
        if (msg_path == q->source_path)
        {
          log.summary << "Marked correlation " << *q
                      << " as client_disconnected\n";
          q->client_disconnected = true;
        }
      }
    }
    break;
  }

  return true;  // Allow it to be forwarded/reversed
}

//--------------------------------------------------------------------------
// Tick function - times out correlations
void Correlator::tick()
{
  request_cache.tidy();

  // Remove any correlations explicitly marked as disconnected if safe
  MT::RWWriteLock lock(request_cache.mutex);
  for(CacheType::iterator p = request_cache.begin(); p!=request_cache.end(); )
  {
    CacheType::iterator q = p++;
    if (q->client_disconnected)
    {
      // Only remove if no outstanding copies and not waiting for a reply
      if (q->copies.empty() || q->replied)
      {
        Log::Error log;
        log << "Removing correlation " << *q
                    << " previously marked disconnected\n";
        request_cache.remove(q.id());
      }
    }
  }
}

//--------------------------------------------------------------------------
// Handle orphan messages (messages that cannot now be replied to)
void Correlator::handle_orphan(const string& id, Correlation *cr)
{
  Log::Error log;
  log << "Correlation " << *cr << " orphaned with no response\n";

  const FaultMessage response(id, SOAP::Fault::CODE_RECEIVER,
                              "Nothing to handle this request");
  const MessagePath path(cr->source_path);
  RoutingMessage newmsg(response, path);
  originate(newmsg);
}


//==========================================================================
// Auto-register
OT_XMLMESH_REGISTER_SERVICE(Correlator, "correlator");

}} // namespaces

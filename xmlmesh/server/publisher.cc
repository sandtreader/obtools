//==========================================================================
// ObTools::XMLMesh:Server: publisher.cc
//
// Implementation of Publisher service for XMLMesh
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "server.h"
#include "ot-log.h"
#include "ot-text.h"

#include <unistd.h>
#include <sstream>

namespace ObTools { namespace XMLMesh {

class PublishService;  // forward

//==========================================================================
// Subscription record
class Subscription
{
public:
  string subject;          // Subject pattern
  string path;             // Client return path

  Subscription(const string& _subject, const string& _path):
    subject(_subject), path(_path)
  {}
};

//==========================================================================
// Publisher Service
class Publisher: public Service
{
private:
  string subject_pattern;            // Pattern of allowed subjects
  MT::RWMutex mutex;                 // On subscriptions list
  list<Subscription> subscriptions;

  bool handle_subscription(RoutingMessage& msg);
  bool subscribe(const string& subject, const string& path);
  void unsubscribe(const string& subject, const string& path);
  void unsubscribe_all(const string& path);

public:
  //------------------------------------------------------------------------
  // Constructor
  Publisher(XML::Element& cfg);

  //------------------------------------------------------------------------
  // Handle any message
  bool handle(RoutingMessage& msg);
};

//------------------------------------------------------------------------
// Constructor - take subject and transport to attach to
Publisher::Publisher(XML::Element& cfg):
  Service(cfg),
  subject_pattern(cfg.get_attr("subject", "*"))
{
  Log::Summary log;
  log << "Publish Service '" << id << "' started for subjects '"
      << subject_pattern << "'\n";
}

//------------------------------------------------------------------------
// Handle any message
bool Publisher::handle(RoutingMessage& msg)
{
  Log::Streams log;

  switch (msg.type)
  {
    case RoutingMessage::CONNECTION:
      break;

    case RoutingMessage::MESSAGE:
    {
      string subject = msg.message.get_subject();
      log.detail << "Publish service received message subject " << subject
                 << " from " << msg.path.to_string() << endl;

      // Check for xmlmesh.subscription messages first - note we let them
      // continue to other subscribers if they're not bogus
      if (Text::pattern_match("xmlmesh.subscription.*", subject)
          && !handle_subscription(msg))
        return false;

      // Try each subscription in turn to see if it wants it
      MT::RWReadLock lock(mutex);
      for(list<Subscription>::iterator p = subscriptions.begin();
          p!=subscriptions.end();
          p++)
      {
        Subscription& sub = *p;
        if (Text::pattern_match(sub.subject, subject))
        {
          // Create new RoutingMessage from the inbound one, with us
          // as originator, and with the same path, but set as response
          // Note however that the message isn't modified - no ref set
          MessagePath path(sub.path);
          RoutingMessage submsg(msg.message, path);

          // If old message was being tracked, attach new one as well
          if (msg.tracker) submsg.track(msg.tracker);

          originate(submsg);
        }
      }
    }
    break;

    case RoutingMessage::DISCONNECTION:
    {
      // Unsubscribe everything that uses this
      unsubscribe_all(msg.path.to_string());
    }
    break;
  }

  return true;  // Not likely to have forward routing, but possible
}

//------------------------------------------------------------------------
// Handle an xmlmesh.subscription message
bool Publisher::handle_subscription(RoutingMessage& msg)
{
  Log::Streams log;

  // Unpack it
  SubscriptionMessage smsg(msg.message);
  string path = msg.path.to_string();

  if (!smsg)
  {
    log.error << "Subscription: Bogus message from " << path << " dropped\n";
    respond(msg, SOAP::Fault::CODE_SENDER, "Illegal subscription message");
    return false;
  }

  log.summary << "Subscription request from " << path << ":\n";
  log.summary << smsg << endl;

  // Handle it
  switch (smsg.operation)
  {
    case SubscriptionMessage::JOIN:
      if (subscribe(smsg.subject, path))
      {
	respond(msg);
	return false;  // Take it
      }
      break;

    case SubscriptionMessage::LEAVE:
      unsubscribe(smsg.subject, path);
      respond(msg);
      return false;

    default:
      respond(msg, SOAP::Fault::CODE_SENDER, "Illegal subscription operation");
      return false;
  }

  // Let others see it
  return true;
}

//------------------------------------------------------------------------
// Subscribe a client
bool Publisher::subscribe(const string& subject,
			  const string& path)
{
  // Check pattern is one we can accept subscription for
  if (Text::pattern_match(subject_pattern, subject))
  {
    // Unsubscribe from this first
    unsubscribe(subject, path);

    // (Re)subscribe
    Subscription sub(subject, path);

    Log::Detail log;
    log << "Client " << path << " subscribed to " << subject << endl;

    MT::RWWriteLock lock(mutex);
    subscriptions.push_back(sub);
    return true;
  }
  else return false;
}

//------------------------------------------------------------------------
// Unsubscribe a client from a particular (set of) subject(s)
// Uses pattern match to allow general unsubscribe
// E.g. foo.* unsubscribes foo.blah.*, foo.splat as well as foo.*
void Publisher::unsubscribe(const string& subject,
			    const string& path)
{
  MT::RWWriteLock lock(mutex);  // Require write now because may need it later
  for(list<Subscription>::iterator p = subscriptions.begin();
      p!=subscriptions.end();
     )
  {
    list<Subscription>::iterator q = p++;  // Move safely before deletion
    Subscription& sub = *q;

    if (sub.path == path
	&& Text::pattern_match(subject, sub.subject))
    {
      Log::Detail log;
      log << "Client " << path << " unsubscribed from " << sub.subject << endl;
      subscriptions.erase(q);
    }
  }
}

//------------------------------------------------------------------------
// Unsubscribe a client entirely
void Publisher::unsubscribe_all(const string& path)
{
  MT::RWWriteLock lock(mutex);  // Require write now because may need it later
  for(list<Subscription>::iterator p = subscriptions.begin();
      p!=subscriptions.end();
      )
  {
    list<Subscription>::iterator q = p++;  // Move safely before deletion
    Subscription& sub = *q;
    if (sub.path == path)
    {
      Log::Detail log;
      log << "Client " << path << " unsubscribed from " << sub.subject << endl;

      subscriptions.erase(q);
    }
  }
}

//==========================================================================
// Auto-register
OT_XMLMESH_REGISTER_SERVICE(Publisher, "publisher");

}} // namespaces





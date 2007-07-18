//==========================================================================
// ObTools::XMLMesh:Server: publisher.cc
//
// Implementation of Publisher service for XMLMesh
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
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
  ServiceClient client;    // Originator's client reference
  
  Subscription(const string& _subject, const string& _path,
	       ServiceClient& _client):
    subject(_subject), path(_path), client(_client) 
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

  bool handle_subscription(RoutingMessage& msg, Log::Streams& tlog);
  bool subscribe(const string& subject, const string& path,
		 ServiceClient& client, Log::Streams& tlog);
  void unsubscribe(const string& subject, const string& path,
		   ServiceClient& client, Log::Streams& tlog);
  void unsubscribe_all(ServiceClient& client, Log::Streams& tlog); 

public:
  //------------------------------------------------------------------------
  // Constructor
  Publisher(XML::Element& cfg);

  //------------------------------------------------------------------------
  // Signal various global events, independent of message routing
  void signal(Signal sig, ServiceClient& client);

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
  log.summary << "Publish Service '" << id << "' started for subjects '"
	      << subject_pattern << "'\n";
}

//------------------------------------------------------------------------
// Signal various global events, independent of message routing
void Publisher::signal(Signal sig, ServiceClient& client)
{
  switch (sig)
  {
    case Service::CLIENT_STARTED:
      // Ignore for now
      break;

    case Service::CLIENT_FINISHED:
      // Force unsubscription on everything
      Log::Streams tlog;
      tlog.summary << "Forcibly unsubscribing client " 
		   << client.client << "\n";

      unsubscribe_all(client, tlog);
      break;
  }
}

//------------------------------------------------------------------------
// Handle any message
bool Publisher::handle(RoutingMessage& msg)
{
  string subject = msg.message.get_subject();
  Log::Streams tlog;

  tlog.detail << "Publish service received message subject " << subject 
	      << " from client " << msg.client.client << endl;

  // Check for xmlmesh.subscription messages first - note we let them
  // continue to other subscribers if they're not bogus
  if (Text::pattern_match("xmlmesh.subscription.*", subject)
      && !handle_subscription(msg, tlog))
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
      ServiceClient client(this, msg.client.client);
      MessagePath path(sub.path);
      RoutingMessage submsg(client, msg.message, path);

      // If old message was being tracked, attach new one as well
      if (msg.tracker) submsg.track(msg.tracker);

      originate(submsg);
    }
  }

  return true;  // Not likely to have forward routing, but possible
}

//------------------------------------------------------------------------
// Handle an xmlmesh.subscription message
bool Publisher::handle_subscription(RoutingMessage& msg, Log::Streams& tlog)
{
  // Unpack it 
  SubscriptionMessage smsg(msg.message);
  string path = msg.path.to_string();

  if (!smsg)
  {
    tlog.error << "Subscription: Bogus message from " << path << " dropped\n";
    respond(msg, SOAP::Fault::CODE_SENDER, "Illegal subscription message");
    return false;
  }

  tlog.summary << "Subscription request from " << path << ":\n";
  tlog.summary << smsg << endl;

  // Handle it
  switch (smsg.operation)
  {
    case SubscriptionMessage::JOIN:
      if (subscribe(smsg.subject, path, msg.client, tlog))
      {
	respond(msg);
	return false;  // Take it
      }
      break;

    case SubscriptionMessage::LEAVE:
      unsubscribe(smsg.subject, path, msg.client, tlog);
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
			  const string& path,
			  ServiceClient& client,
			  Log::Streams& tlog)
{
  // Check pattern is one we can accept subscription for
  if (Text::pattern_match(subject_pattern, subject))
  {
    // Unsubscribe from this first
    unsubscribe(subject, path, client, tlog);

    // (Re)subscribe
    Subscription sub(subject, path, client);

    tlog.detail << "Client " << path << " subscribed to "
		<< subject << "\n";

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
			    const string& path,
			    ServiceClient& client,
			    Log::Streams& tlog)
{
  MT::RWWriteLock lock(mutex);  // Require write now because may need it later
  for(list<Subscription>::iterator p = subscriptions.begin();
      p!=subscriptions.end();
     )
  {
    list<Subscription>::iterator q = p++;  // Move safely before deletion
    Subscription& sub = *q;

    // Note, we still check path, in case we get multiple subscriptions
    // for the same client over multiple paths - weird, but possible
    if (sub.path == path
	&& sub.client == client
	&& Text::pattern_match(subject, sub.subject))
    {
      tlog.detail << "Client " << path << " unsubscribed from "
		  << sub.subject << "\n";

      subscriptions.erase(q);
    }
  }
}

//------------------------------------------------------------------------
// Unsubscribe a client entirely
void Publisher::unsubscribe_all(ServiceClient& client,
				Log::Streams& tlog)
{
  MT::RWWriteLock lock(mutex);  // Require write now because may need it later
  for(list<Subscription>::iterator p = subscriptions.begin();
      p!=subscriptions.end();
      )
  {
    list<Subscription>::iterator q = p++;  // Move safely before deletion
    Subscription& sub = *q;
    if (sub.client == client)
    {
      tlog.detail << "Client " << sub.path << " unsubscribed from "
		  << sub.subject << "\n";

      subscriptions.erase(q);
    }
  }
}

//==========================================================================
// Auto-register
OT_XMLMESH_REGISTER_SERVICE(Publisher, "publisher");

}} // namespaces





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
#include <netinet/in.h>
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
  list<Subscription> subscriptions;

  bool handle_subscription(RoutingMessage& msg);
  bool subscribe(const string& subject, const string& path,
		 ServiceClient& client);
  void unsubscribe(const string& subject, const string& path,
		   ServiceClient& client);
  void unsubscribe_all(ServiceClient& client); 

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
  Log::Summary << "Publish Service '" << id << "' started for subjects '"
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
      Log::Summary << "Forcibly unsubscribing client " 
		   << client.client << "\n";

      unsubscribe_all(client);
      break;
  }
}

//------------------------------------------------------------------------
// Handle any message
bool Publisher::handle(RoutingMessage& msg)
{
  string subject = msg.message.get_subject();

  Log::Detail << "Publish service received message subject " << subject 
	      << " from client " << msg.client.client << endl;

  // Check for xmlmesh.subscription messages first - note we let them
  // continue to other subscribers if they're not bogus
  if (Text::pattern_match("xmlmesh.subscription.*", subject)
      && !handle_subscription(msg))
    return false;

  // Try each subscription in turn to see if it wants it
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
      originate(submsg);
    }
  }

  return true;  // Not likely to have forward routing, but possible
}

//------------------------------------------------------------------------
// Handle an xmlmesh.subscription message
bool Publisher::handle_subscription(RoutingMessage& msg)
{
  // Unpack it 
  SubscriptionMessage smsg(msg.message);
  string path = msg.path.to_string();
  
  if (!smsg)
  {
    Log::Error << "Subscription: Bogus message from " << path << " dropped\n";
    respond(msg, SOAP::Fault::CODE_SENDER, "Illegal subscription message");
    return false;
  }

  Log::Summary << "Subscription request from " << path << ":\n";
  Log::Summary << smsg << endl;

  // Handle it
  switch (smsg.operation)
  {
    case SubscriptionMessage::JOIN:
      if (subscribe(smsg.subject, path, msg.client))
      {
	respond(msg);
	return false;  // Take it
      }
      break;

    case SubscriptionMessage::LEAVE:
      unsubscribe(smsg.subject, path, msg.client);
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
			  ServiceClient& client)
{
  // Check pattern is one we can accept subscription for
  if (Text::pattern_match(subject_pattern, subject))
  {
    // Unsubscribe from this first
    unsubscribe(subject, path, client);

    // (Re)subscribe
    Subscription sub(subject, path, client);
    subscriptions.push_back(sub);

    Log::Detail << "Client " << path << " subscribed to "
		<< subject << "\n";

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
			    ServiceClient& client)
{
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
      Log::Detail << "Client " << path << " unsubscribed from "
		  << sub.subject << "\n";

      subscriptions.erase(q);
    }
  }
}

//------------------------------------------------------------------------
// Unsubscribe a client entirely
void Publisher::unsubscribe_all(ServiceClient& client)
{
  for(list<Subscription>::iterator p = subscriptions.begin();
      p!=subscriptions.end();
      )
  {
    list<Subscription>::iterator q = p++;  // Move safely before deletion
    Subscription& sub = *q;
    if (sub.client == client)
    {
      Log::Detail << "Client " << sub.path << " unsubscribed from "
		  << sub.subject << "\n";

      subscriptions.erase(q);
    }
  }
}

//==========================================================================
// Auto-register
OT_XMLMESH_REGISTER_SERVICE(Publisher, "publisher");

}} // namespaces





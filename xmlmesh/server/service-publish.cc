//==========================================================================
// ObTools::XMLMesh:Server: service-publish.cc
//
// Implementation of Publish service for XMLMesh
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "service-publish.h"
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
  Transport *transport;    // Which transport client is on
  Net::EndPoint client;    // Client details
  string subject;          // Subject pattern

  Subscription(Transport *_transport, const Net::EndPoint& _client,
	       const string& _subject):
    transport(_transport), client(_client), subject(_subject) {}
};

//==========================================================================
// Publish Service 
class PublishService: public Service, public MessageHandler
{
private:
  string subject_pattern;            // Pattern of allowed subjects
  const Transport *transport;        // Transport allowed (0 => all)

  list<Subscription> subscriptions;

  //------------------------------------------------------------------------
  // Handle a subscription message
  bool handle_subscription(IncomingMessage& msg);

public:
  //------------------------------------------------------------------------
  // Constructor - take subject and transport to attach to 
  PublishService(Server& _server, const string& _subject, 
		 const Transport *_transport);


  //------------------------------------------------------------------------
  // Service signal that a client has started/finished
  void signal_client(Transport *transport, Net::EndPoint& client,
		     Signal signal);

  //------------------------------------------------------------------------
  // Handle any message (by virtue of being a MessageHandler)
  bool handle(IncomingMessage& msg);

  //------------------------------------------------------------------------
  // Subscribe a client - returns whether accepted
  bool subscribe(Transport *transport, 
		 const Net::EndPoint& client, 
		 const string& subject); 

  //------------------------------------------------------------------------
  // Unubscribe a client
  void unsubscribe(Transport *transport, 
		   const Net::EndPoint& client, 
		   const string& subject); 
};

//------------------------------------------------------------------------
// Constructor - take subject and transport to attach to 
PublishService::PublishService(Server& _server, const string& _subject, 
			       const Transport *_transport):
    Service(_server), subject_pattern(_subject), transport(_transport) 
{
  Log::Summary << "Publish Service started on ";
  if (transport)
    Log::Summary << "transport " << transport->name;
  else
    Log::Summary << "all transports";

  Log::Summary << ", for subjects " << subject_pattern << endl;

  // Register ourselves for everything
  server.attach_handler("*", *this);
}

//------------------------------------------------------------------------
// Service signal that a client has started/finished
void PublishService::signal_client(Transport *transport, Net::EndPoint& client,
				   Signal signal)
{
  switch (signal)
  {
    case Service::CLIENT_STARTED:
      // Ignore for now
      break;

    case Service::CLIENT_FINISHED:
      // Force unsubscription on everything
      Log::Summary << "Forcibly unsubscribing client " << transport->name 
		   << ": " << client << "\n";
      unsubscribe(transport, client, "*");
      break;
  }
}

//------------------------------------------------------------------------
// Handle any message
bool PublishService::handle(IncomingMessage& msg)
{
  string subject = msg.message.get_subject();

  Log::Detail << "Publish service received message subject " << subject 
	      << endl;

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
      // Show it to correlator in case there is a response
      server.correlate(msg, sub.transport, sub.client);

      // Send to subscribed client 
      server.send(msg.message, sub.transport, sub.client);
    }
  }
}

//------------------------------------------------------------------------
// Handle an xmlmesh.subscription message
bool PublishService::handle_subscription(IncomingMessage& msg)
{
  // Check transport is allowed
  if (!transport || transport == msg.transport)
  {
    // Unpack it 
    SubscriptionMessage smsg(msg.message);

    if (!smsg)
    {
      Log::Error << "Subscription: Bogus message from " << msg.transport->name 
		 << ": " << msg.client << " dropped\n";
      server.respond(ErrorMessage::FATAL, "Illegal XML", msg);
      return false;
    }

    Log::Summary << "Subscription request from " << msg.transport->name 
		 << ": " << msg.client << ":\n";
    Log::Summary << smsg << endl;

    // Handle it
    switch (smsg.operation)
    {
      case SubscriptionMessage::JOIN:
	if (subscribe(msg.transport, msg.client, smsg.subject))
	{
	  server.respond(msg);
	  return false;  // Take it
	}
	break;

      case SubscriptionMessage::LEAVE:
	unsubscribe(msg.transport, msg.client, smsg.subject);
	{
	  server.respond(msg); 
	  return false;
	}
	break;

      default:
	server.respond(ErrorMessage::FATAL, "Illegal operation", msg);
	return false;
    }
  }
 
  // Let others see it
  return true;
}

//------------------------------------------------------------------------
// Subscribe a client
bool PublishService::subscribe(Transport *transport, 
			       const Net::EndPoint& client, 
			       const string& subject)
{
  // Check pattern is one we can accept subscription for
  if (Text::pattern_match(subject_pattern, subject))
  {
    Subscription sub(transport, client, subject);
    subscriptions.push_back(sub);

    Log::Detail << "Client " << transport->name 
		<< ": " << client << " subscribed to "
		<< subject << "\n";

    return true;
  }
  else return false;
}

//------------------------------------------------------------------------
// Unsubscribe a client
// Uses pattern match to allow general unsubscribe
// E.g. foo.* unsubscribes foo.blah.*, foo.splat as well as foo.*
void PublishService::unsubscribe(Transport *transport, 
				 const Net::EndPoint& client, 
				 const string& subject)
{
restart:
  for(list<Subscription>::iterator p = subscriptions.begin();
      p!=subscriptions.end();
      p++)
  {
    Subscription& sub = *p;
    if (sub.transport == transport && sub.client == client
	&& Text::pattern_match(subject, sub.subject))
    {
      Log::Detail << "Client " << transport->name 
		  << ": " << client << " unsubscribed from "
		  << sub.subject << "\n";

      subscriptions.erase(p);
      goto restart;  // We're deleting what we're standing on
    }
  }
}

//==========================================================================
// Publish Service Factory

//------------------------------------------------------------------------
//Singleton instance
PublishServiceFactory PublishServiceFactory::instance;

//------------------------------------------------------------------------
//Create method
Service *PublishServiceFactory::create(Server& server, XML::Element& xml)
{
  string subject = xml.get_attr("subject", "*");
  string trans   = xml.get_attr("transport");
  const Transport *transport = 0;

  // Lookup transport
  if (trans.size()) transport = server.lookup_transport(trans);

  return new PublishService(server, subject, transport);
}

//------------------------------------------------------------------------
//Registration method
void PublishServiceFactory::register_into(Server& server)
{
  server.register_service("publish", &instance);
}

}} // namespaces





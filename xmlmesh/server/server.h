//==========================================================================
// ObTools::XMLMesh:Server: server.h
//
// Internal definitions for XMLMesh Server
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#ifndef __OBTOOLS_XMLMESH_SERVER_H
#define __OBTOOLS_XMLMESH_SERVER_H

#include <string>
#include "ot-net.h"
#include "ot-mt.h"
#include "ot-xmlmesh.h"

namespace ObTools { namespace XMLMesh {

//Make our lives easier without polluting anyone else
using namespace std;

class Transport;  // Forward

//==========================================================================
// Message queue data for incoming messages, and typedef for queue
struct IncomingMessage
{
  Transport *transport;        // Transport we received it from
  Net::EndPoint client;        // Client on that transport
  Message message;             // The message

  enum Flags
  {
    RESPONDED = 1,             // Set when someone responds
    STARTED = 2,               // Set on empty message when client starting
    FINISHED = 4,              // Set on empty message when client finished
  }; 

  int flags;

  IncomingMessage(Transport *_transport, Net::EndPoint _client, 
		  Message _message, int _flags=0):
    transport(_transport), client(_client), message(_message),
    flags(_flags) {}
};

typedef MT::Queue<IncomingMessage> IncomingMessageQueue;

//==========================================================================
// Message Handler (abstract interface)
class MessageHandler
{
public:
  //------------------------------------------------------------------------
  // Handle a message
  // Returns whether message should continue to be distributed
  virtual bool handle(IncomingMessage& msg) = 0;
};

//==========================================================================
// Handler registration
struct HandlerRegistration
{
  string subject_pattern;
  MessageHandler& handler;

  HandlerRegistration(const string& _pattern, MessageHandler& _handler):
    subject_pattern(_pattern), handler(_handler) {}
};

//==========================================================================
// Message Distributor
class Distributor
{
private:
  list<HandlerRegistration> handlers;       // List of active handlers

public:
  //------------------------------------------------------------------------
  // Default Constructor 
  Distributor() {}

  //------------------------------------------------------------------------
  // Attach a new message handler on the given subject pattern
  void attach_handler(const string& subject, MessageHandler& h);

  //------------------------------------------------------------------------
  // Distribute a message
  void distribute(IncomingMessage& msg);
};

//==========================================================================
// Server Transport (abstract interface)
// Low-level transport of raw data
class Transport
{
protected:
  IncomingMessageQueue *incoming_q;  

public:
  string name;

  //------------------------------------------------------------------------
  // Default constructor
  Transport(const string& _name): name(_name), incoming_q(0) {}

  //------------------------------------------------------------------------
  // Attach to given incoming queue
  void attach_incoming(IncomingMessageQueue& iq) { incoming_q = &iq; }

  //------------------------------------------------------------------------
  // Send a message to the given client - returns whether successful
  virtual bool send(const Net::EndPoint& client, const string& data) = 0;
};

//==========================================================================
// Server Transport Factory (abstract interface)
class TransportFactory
{
public:
  //------------------------------------------------------------------------
  // Create a server transport from the given XML element
  // Returns 0 if failed
  virtual Transport *create(XML::Element& xml) = 0;
};

class Server;  //forward

//==========================================================================
// Service (abstract interface)
// Enabling module providing various kinds of message service
class Service
{
protected:
  Server& server;

public:
  //------------------------------------------------------------------------
  // Constructor
  Service(Server& _server): server(_server) {}

  //------------------------------------------------------------------------
  // Service signal that a client has started/finished
  enum Signal
  {
    CLIENT_STARTED,
    CLIENT_FINISHED
  };
    
  virtual void signal_client(Transport *transport, Net::EndPoint& client,
			     Signal signal) = 0;

};

//==========================================================================
// Service Factory (abstract interface)
class ServiceFactory
{
public:
  //------------------------------------------------------------------------
  // Create a service from the given XML element
  // Returns 0 if failed
  virtual Service *create(Server& server, XML::Element& xml) = 0;
};

//==========================================================================
// General XML Bus server using any number of transports
class Server
{
private:
  // Factories for use during configuration
  map<string, TransportFactory *> transport_factories;
  map<string, ServiceFactory *>         service_factories;

  // List of active modules
  list<Transport *> transports;            // List of active transports
  map<string, Transport *> transport_ids;  // Map of transports ids

  list<Service *> services;                 // List of active services

  // Internal state
  Distributor distributor;                  // Message distributor
  IncomingMessageQueue incoming_q;          // Queue of incoming messages

  bool create_transport(XML::Element& xml);
  bool create_service(XML::Element& xml); 

public:
  //------------------------------------------------------------------------
  // Constructor 
  Server();

  //------------------------------------------------------------------------
  // Register a transport type
  void register_transport(const string& name, TransportFactory *factory);

  //------------------------------------------------------------------------
  // Register a service type
  void register_service(const string& name, ServiceFactory *factory);

  //------------------------------------------------------------------------
  // Signal all services that a client has started or finished
  void signal_services(Transport *transport, Net::EndPoint& client,
		       Service::Signal signal);

  //------------------------------------------------------------------------
  // Look up a transport by name (use result only for comparison)
  Transport *lookup_transport(const string& name);

  //------------------------------------------------------------------------
  // Attach a new message handler on the given subject pattern
  void attach_handler(const string& subject, MessageHandler& h);

  //------------------------------------------------------------------------
  // Send a new message to the client on the transport given
  // Returns whether successful
  bool send(Message &msg, Transport *transport, Net::EndPoint& client);

  //------------------------------------------------------------------------
  // Return a message as a response to the request given
  // Returns whether successul
  bool respond(Message& response, IncomingMessage& request);

  //------------------------------------------------------------------------
  // Return OK to request given
  // Returns whether successul
  bool respond(IncomingMessage& request);

  //------------------------------------------------------------------------
  // Return an error to request given
  // Returns whether successul
  bool respond(ErrorMessage::Severity severity,
	       const string& text,
	       IncomingMessage& request);

  //------------------------------------------------------------------------
  // Load modules etc. from XML config
  void configure(XML::Configuration& config);

  //------------------------------------------------------------------------
  // Run method - never returns
  void run(); 

  //------------------------------------------------------------------------
  // Destructor
  ~Server();
};



//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_XMLMESH_SERVER_H




//==========================================================================
// ObTools::XMLBus:Server: server.h
//
// Internal definitions for XMLBus Server
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#ifndef __OBTOOLS_XMLBUS_SERVER_H
#define __OBTOOLS_XMLBUS_SERVER_H

#include <string>
#include "ot-net.h"
#include "ot-mt.h"
#include "ot-xmlbus.h"

namespace ObTools { namespace XMLBus {

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Message queue data for incoming messages, and typedef for queue
struct IncomingMessage
{
  Net::EndPoint client;
  Message message;
};

typedef MT::Queue<IncomingMessage> IncomingMessageQueue;

//==========================================================================
// Server Transport (abstract interface)
// Low-level transport of raw data
class ServerTransport
{
private:
  IncomingMessageQueue *incoming_q;  

public:
  //------------------------------------------------------------------------
  // Default constructor
  ServerTransport(): incoming_q(0) {}

  //------------------------------------------------------------------------
  // Attach to given incoming queue
  void attach_incoming(IncomingMessageQueue& iq) { incoming_q = &iq; }

  //------------------------------------------------------------------------
  // Send a message to the given client - returns whether successful
  virtual bool send(const Net::EndPoint& client, const string& data) = 0;
};

//==========================================================================
// General XML Bus server using any number of transports
class Server
{
private:
  IncomingMessageQueue incoming_q;          // Queue of incoming messages
  list<ServerTransport *> transports;       // List of active transports

public:
  //------------------------------------------------------------------------
  // Constructor 
  Server();

  //------------------------------------------------------------------------
  // Attach a new transport
  // Transport will be deleted on destruction
  void attach_transport(ServerTransport *t);

  //------------------------------------------------------------------------
  // Destructor
  ~Server();
};



//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_XMLBUS_SERVER_H




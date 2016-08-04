//==========================================================================
// ObTools::XMLMesh: ot-xmlmesh-otmp.h
//
// Internal definitions for OTMP protocol, client and server
//
// Copyright (c) 2007 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_XMLMESH_OTMP_H
#define __OBTOOLS_XMLMESH_OTMP_H

#include "ot-tube.h"

namespace ObTools { namespace XMLMesh { namespace OTMP {

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// OTMP protocol uses the generic 'Tube' protocol defined in ot-tube.h

// Standard protocol port
const int DEFAULT_PORT = 29167;

// Standard OTMP tags
enum Tag
{
  TAG_MESSAGE = 0x4f544d53   // OTMS - Message carrying
};

//==========================================================================
// OTMP message with fixed tag
struct Message: public Tube::Message
{
  Message(const string& _data="", Tube::flags_t _flags=0):
    Tube::Message(TAG_MESSAGE, _data, _flags) {}
};

//==========================================================================
// Client message with fixed tag
struct ClientMessage: public Tube::ClientMessage
{
  // Constructor for message
  ClientMessage(const SSL::ClientDetails& _client,
                const string& _data="", Tube::flags_t _flags=0):
    Tube::ClientMessage(_client, TAG_MESSAGE, _data, _flags) {}
};

//==========================================================================
// OTMP client
class Client: public Tube::Client
{
private:
  //------------------------------------------------------------------------
  // Overridable function to filter message tags - return true if tag
  // is recognised.  Only allows 'OTMS'
  virtual bool tag_recognised(Tube::tag_t tag) { return tag == TAG_MESSAGE; }

public:
  //------------------------------------------------------------------------
  // Constructor - takes server endpoint (address+port)
  Client(const Net::EndPoint& _server): Tube::Client(_server, "OTMP") {}
};

//==========================================================================
// Handy typedef for client message queues
typedef MT::Queue<ClientMessage> ClientMessageQueue;

//==========================================================================
// OTMP server
// Note, unlike the client it delivers messages to a given message queue
// rather than providing a poll/wait interface;  this is because it is
// expecting to aggregate its messages with a number of other servers
class Server: public Tube::Server
{
private:
  ClientMessageQueue& receive_q;  // Note:  Not mine

  //------------------------------------------------------------------------
  // Overridable function to filter message tags - return true if tag
  // is recognised.  Only allows 'OTMS'
  virtual bool tag_recognised(Tube::tag_t tag) const
  { return tag == TAG_MESSAGE; }

  //------------------------------------------------------------------------
  // Abstract function to handle an incoming client message
  // Whether connection should be allowed to continue
  virtual bool handle_message(const Tube::ClientMessage& msg);

public:
  //------------------------------------------------------------------------
  // Constructor - takes receive queue for incoming messages
  // port=0 means take default port for protocol
  // timeout is keepalive timeout (secs)
  // The rest is thread/socket tuning - see Net::TCPServer
  Server(ClientMessageQueue& receive_queue,
         int port=0, int backlog=5,
         int min_spare_threads=1, int max_threads=10, int timeout=0);
};

//==========================================================================
}}} //namespaces
#endif // !__OBTOOLS_XMLMESH_OTMP_H




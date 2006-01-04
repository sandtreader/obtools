//==========================================================================
// ObTools::XMLMesh: ot-xmlmesh-otmp.h
//
// Internal definitions for OTMP protocol, client and server
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#ifndef __OBTOOLS_XMLMESH_OTMP_H
#define __OBTOOLS_XMLMESH_OTMP_H

#include <string>
#include <map>
#include "ot-net.h"
#include "ot-log.h"
#if !defined(_SINGLE)
#include "ot-mt.h"
#endif

namespace ObTools { namespace XMLMesh { namespace OTMP { 

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Definition of ObTools Message Protocol (OTMP) - simple binary TLV protocol
// for encapsulating messages in a stream and providing a measure of 
// synchronisation sanity check

// The OTMP protocol is as follows.  
// All integers are network byte-order (NBO) => MSB first
// The stream is broken into type-length-value 'chunks', just like (e.g.)
// a TIFF file.  There is no stream header or trailer.
//
// stream:
//     <chunk>
//     <chunk>
//     <chunk>
//
// A chunk is a tag indicating the chunk type (only one is currently defined),
// the length of the message, some flags, and the data of the message.  
//
// Because there aren't many tags defined out of the 32-bit range, the tag 
// also acts as a sanity check on synchronisation.
//
// chunk: 
//    0:        4-byte tag, first char at 0 (equivalently, 32-bit NBO integer)
//    4:        4-byte NBO integer length ('L')
//    8:        32-bits of flags (none yet defined, set to 0), NBO
//    12-L+12: 'L' bytes of message, unterminated and unpadded

// Error behaviour:  
//   If a stream ends cleanly before the first chunk, or between chunks, this 
//     is fine
//   If a stream fails before the first chunk, or between chunks, an error
//     should be logged
//   If a stream fails or ends within a chunk, the message should be dropped 
//     and an error logged
//   If any chunk begins with an unrecognised tag, the stream should be killed
//     and an error logged

// Standard protocol port
const int DEFAULT_PORT = 29167;

// Standard OTMP tags
enum Tag
{
  TAG_MESSAGE = 0x4f544d53   // OTMS - Message carrying
};

// Internal struct for carrying messages 
// This is NOT used for directly encoding the stream!
struct Message
{
  // Length is implicit in data.size()
  uint32_t flags;
  string data;  

  Message(const string& _data="", int _flags=0): 
    flags(_flags), data(_data) {}
};

//==========================================================================
// OTMP client
class Client
{
private:
  // Network stuff
  Net::EndPoint server;
  Net::TCPClient *socket;

#if defined(_SINGLE)
  Log::Streams log;            // Our log streams (in MT, threads provide)
#else
  // Thread and queue stuff
  MT::Mutex mutex;             // Global client mutex used for socket
                               // creation and restart
  MT::Thread *send_thread;
  MT::Queue<Message> send_q;

  MT::Thread *receive_thread;
  MT::Queue<Message> receive_q;
#endif

  bool alive;                  // Not being killed

  // Internal functions
  bool restart_socket(Log::Streams& log);

public:
  //------------------------------------------------------------------------
  // Constructors - takes server endpoint (address+port)
  Client(Net::EndPoint _server);

  //------------------------------------------------------------------------
  // Check it hasn't been killed
  bool is_alive() { return alive; }

#if !defined(_SINGLE)
  //------------------------------------------------------------------------
  // Background functions called by threads - do not use directly
  bool receive_messages(Log::Streams& log);
  bool send_messages(Log::Streams& log);
#endif

  //------------------------------------------------------------------------
  // Send a message - never blocks, but can fail if the queue is full
  // Whether message queued
  bool send(Message& msg);

#if !defined(_SINGLE)
  //------------------------------------------------------------------------
  // Check whether a message is available before blocking in wait()
  bool poll();
#endif

  //------------------------------------------------------------------------
  // Receive a message - blocks waiting for one to arrive
  // Returns false if the connection was restarted 
  bool wait(Message& msg);

  //------------------------------------------------------------------------
  // Shut down client cleanly
  void shutdown();

  //------------------------------------------------------------------------
  // Destructor
  virtual ~Client();
};

#if !defined(_SINGLE)  // None of the server stuff works singlethreaded
//==========================================================================
// Handy typedef of a session map - maps endpoints to sessions
typedef map<Net::EndPoint, struct ClientSession *> SessionMap;

//==========================================================================
// Client Session structure - record of a single connection held by server
struct ClientSession
{
  Net::TCPSocket& socket;
  Net::EndPoint client;
  SessionMap& map;
  bool alive;

  // Thread and queue stuff
  MT::Queue<Message> send_q;  

  // Constructor
  // Adds this session to the given map - destructor removes it again
  ClientSession(Net::TCPSocket& _socket, Net::EndPoint _client,
		SessionMap& _map):
    socket(_socket), client(_client), map(_map), alive(true)
  { _map[_client] = this; }

  // Destructor
  // Ensures removal from the map on exception
  ~ClientSession() { map.erase(client); }
};

//==========================================================================
// Client-specific Message Structure - includes client endpoint
// Note:  It's tempting to put a ClientSession ref in here so we can send
// stuff back directly;  however this causes major garbage collection 
// headaches if the session dies while something still has a handle on this
// message;  it's safer to re-lookup the endpoint each time.
struct ClientMessage
{
  Net::EndPoint client;
  Message msg;

  enum Action
  {
    STARTED,
    MESSAGE,
    FINISHED
  };

  Action action;

  // Constructor for message
  ClientMessage(Net::EndPoint _client, const string& _data="", int _flags=0):
    client(_client), msg(_data,_flags), action(MESSAGE) {}

  // Constructor for other action
  ClientMessage(Net::EndPoint _client, Action _action):
    client(_client), msg(), action(_action) {}
};

//==========================================================================
// Handy typedef for client message queues
typedef MT::Queue<ClientMessage> ClientMessageQueue;

//==========================================================================
// OTMP server
// Note, unlike the client it delivers messages to a given message queue
// rather than providing a poll/wait interface;  this is because it is
// expecting to aggregate its messages with a number of other servers
// Also, unlike the client, this _is_ a TCPServer, rather than owning
// one
class Server: public Net::TCPServer
{
private:
  ClientMessageQueue& receive_q;  // Note:  Not mine
  SessionMap client_sessions;  // Map of sessions
  list<Net::MaskedAddress> filters;  // List of allowed client masks
  bool alive;                  // Not being killed

public:
  //------------------------------------------------------------------------
  // Constructor - takes receive queue for incoming messages
  // port=0 means take default port for protocol
  // The rest is thread/socket tuning - see Net::TCPServer
  Server(ClientMessageQueue& receive_queue,
	 int port=0, int backlog=5, 
	 int min_spare_threads=1, int max_threads=10);

  //------------------------------------------------------------------------
  // Check it hasn't been killed
  bool is_alive() { return alive; }

  //--------------------------------------------------------------------------
  // Allow a given client address to connect (optionally with netmask)
  void allow(Net::MaskedAddress addr)
  { filters.push_back(addr); }

  //--------------------------------------------------------------------------
  // Allow any client
  void open() { filters.push_back(Net::MaskedAddress(0,0)); }

  //--------------------------------------------------------------------------
  // TCPServer verify method
  bool verify(Net::EndPoint ep);

  //------------------------------------------------------------------------
  // TCPServer process method - handles new connections
  void process(ObTools::Net::TCPSocket& s, 
	       ObTools::Net::EndPoint client);

  //------------------------------------------------------------------------
  // Background functions called by threads - do not use directly
  bool receive_messages();
  bool send_messages();

  //------------------------------------------------------------------------
  // Send a message - never blocks, but can fail if the queue is full
  // Sends the message to the endpoint given (most likely where it came from)
  // Whether message queued
  bool send(ClientMessage& msg);
};

#endif // !_SINGLE

//==========================================================================
}}} //namespaces
#endif // !__OBTOOLS_XMLMESH_OTMP_H




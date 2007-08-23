//==========================================================================
// ObTools::Tube: ot-tube.h
//
// Public definitions for ObTools::Tube
// Generic, symmetric binary message connection
// 
// Copyright (c) 2007 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#ifndef __OBTOOLS_TUBE_H
#define __OBTOOLS_TUBE_H

#include "ot-net.h"
#include "ot-mt.h"
#include "ot-chan.h"
#include "ot-log.h"

namespace ObTools { namespace Tube { 

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Definition of ObTools Tube protocol - a simple binary TLV protocol
// for encapsulating messages in a stream and providing a measure of 
// synchronisation sanity check

// All integers are network byte-order (NBO) => MSB first
// The stream is broken into type-length-value 'chunks', just like (e.g.)
// a TIFF file.  There is no stream header or trailer.
//
// stream:
//     <chunk>
//     <chunk>
//     <chunk>
//
// A chunk is a tag indicating the chunk type (application specific)
// the length of the message, some flags (application specific), and the data 
// of the message.  
//
// Because there aren't many tags defined out of the 32-bit range, the tag 
// also acts as a sanity check on synchronisation.
//
// chunk: 
//    0:        4-byte tag, first char at 0 (equivalently, 32-bit NBO integer)
//    4:        4-byte NBO integer length ('L')
//    8:        32-bits of flags (none yet defined, set to 0), NBO
//    12-L+12: 'L' bytes of message, unterminated and unpadded
//
// Note: The top 8 bits (first byte) of flags are reserved for this protocol
//
// Error behaviour:  
//   If a stream ends cleanly before the first chunk, or between chunks, this 
//     is fine
//   If a stream fails before the first chunk, or between chunks, an error
//     should be logged
//   If a stream fails or ends within a chunk, the message should be dropped 
//     and an error logged
//   If any chunk begins with an unrecognised tag, the stream should be killed
//     and an error logged

//==========================================================================
// Handy typedefs

typedef uint32_t tag_t;
typedef uint32_t flags_t;

//==========================================================================
// Internal struct for carrying messages 
// This is NOT used for directly encoding the stream!
struct Message
{
  tag_t tag;
  // Length is implicit in data.size()
  tag_t flags;
  string data;  

  Message(): tag(0), flags(0) {}
  Message(tag_t _tag, const string& _data="", int _flags=0): 
    tag(_tag), flags(_flags), data(_data) {}
};

//==========================================================================
// Tube client
class Client
{
  // Network stuff
  Net::EndPoint server;
  Net::TCPClient *socket;

  // Thread and queue stuff
  MT::Mutex mutex;             // Global client mutex used for socket
                               // creation and restart
  MT::Thread *send_thread;
  MT::Queue<Message> send_q;

  MT::Thread *receive_thread;
  MT::Queue<Message> receive_q;

  bool alive;                  // Not being killed

  // Internal functions
  bool restart_socket(Log::Streams& log);

  //------------------------------------------------------------------------
  // Overridable function to filter message tags - return true if tag
  // is recognised.  By default, allows any tag
  virtual bool tag_recognised(tag_t /*tag*/) { return true; }

public:
  // Name for logging
  string name;

  //------------------------------------------------------------------------
  // Constructor - takes server endpoint (address+port) and optional name
  Client(Net::EndPoint _server, const string& _name="Tube");

  //------------------------------------------------------------------------
  // Check it hasn't been killed
  bool is_alive() { return alive; }

  //------------------------------------------------------------------------
  // Background functions called by threads - do not use directly
  bool receive_messages(Log::Streams& log);
  bool send_messages(Log::Streams& log);

  //------------------------------------------------------------------------
  // Send a message - never blocks, but can fail if the queue is full
  // Whether message queued
  bool send(Message& msg);

  //------------------------------------------------------------------------
  // Check whether a message is available before blocking in wait()
  bool poll();

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

//==========================================================================
// Session map - maps endpoints to sessions
class SessionMap
{
public:
  MT::RWMutex mutex;  // On map
  map<Net::EndPoint, struct ClientSession *> sessions;

  //------------------------------------------------------------------------
  // Add a session
  void add(Net::EndPoint client, struct ClientSession *s)
  {
    MT::RWWriteLock lock(mutex);
    sessions[client] = s;
  }

  //------------------------------------------------------------------------
  // Remove a session
  void remove(Net::EndPoint client)
  {
    MT::RWWriteLock lock(mutex);
    sessions.erase(client);
  }
};

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
  { map.add(_client, this); }

  // Destructor
  // Ensures removal from the map on exception
  ~ClientSession() { map.remove(client); }
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
  ClientMessage(Net::EndPoint _client, tag_t _tag, 
		const string& _data="", flags_t _flags=0):
    client(_client), msg(_tag, _data,_flags), action(MESSAGE) {}

  // Constructor for other action
  ClientMessage(Net::EndPoint _client, Action _action):
    client(_client), msg(), action(_action) {}
};

//==========================================================================
// Tube server
// Unlike the client this is abstract and designed to be subclassed with
// a class providing a handle_message method
// Also, unlike the client, this _is_ a TCPServer, rather than owning
// one
class Server: public Net::TCPServer
{
private:
  SessionMap client_sessions;        // Map of sessions
  list<Net::MaskedAddress> filters;  // List of allowed client masks
  bool alive;                        // Not being killed

  //------------------------------------------------------------------------
  // Overridable function to filter message tags - return true if tag
  // is recognised.  By default, allows any tag
  virtual bool tag_recognised(tag_t /*tag*/) { return true; }

  //------------------------------------------------------------------------
  // Abstract function to handle an incoming client message
  // Whether connection should be allowed to continue
  virtual bool handle_message(ClientMessage& msg)=0;

public:
  // Name for logging
  string name;

  //------------------------------------------------------------------------
  // Constructor - takes port to listen on
  // The rest is thread/socket tuning - see Net::TCPServer
  Server(int port, const string& _name="Tube", int backlog=5, 
	 int min_spare_threads=1, int max_threads=10);

  //------------------------------------------------------------------------
  // Check it hasn't been killed
  bool is_alive() { return alive; }

  //--------------------------------------------------------------------------
  // Allow a given client address to connect (optionally with netmask)
  void allow(Net::MaskedAddress addr) { filters.push_back(addr); }

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

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_TUBE_H




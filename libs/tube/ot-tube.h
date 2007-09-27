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
//    8:        32-bits of flags (see below), NBO
//    12-L+12: 'L' bytes of message, unterminated and unpadded
//
// Note: The top 16 bits (first two bytes) of flags are reserved for this 
// protocol in synchronous request/response mode (see SyncClient/SyncServer
// below)
//
//    Bit 31:     Response required  (message ID is valid)
//    Bit 30:     Response provided  (message ID gives reference)
//
//    Bits 16-23: 8-bit message ID
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
typedef unsigned char id_t;

//==========================================================================
// Flags
enum 
{
  FLAG_RESPONSE_REQUIRED  = 0x80000000UL,
  FLAG_RESPONSE_PROVIDED  = 0x40000000UL,

  MASK_REQUEST_ID        = 0x00FF0000UL,
  SHIFT_REQUEST_ID       = 16
};

//==========================================================================
// Defaults

const int DEFAULT_TIMEOUT = 5;

//==========================================================================
// Internal struct for carrying messages (message.cc)
// This is NOT used for directly encoding the stream!
struct Message
{
  tag_t tag;               // tag=0 indicates invalid
  // Length is implicit in data.size()
  tag_t flags;
  string data;  

  //------------------------------------------------------------------------
  // Constructors
  Message(): tag(0), flags(0) {}
  Message(tag_t _tag, const string& _data="", int _flags=0): 
    tag(_tag), flags(_flags), data(_data) {}

  //------------------------------------------------------------------------
  // Check for validity
  bool is_valid() { return tag!=0; }
  bool operator!() { return !tag; }

  //------------------------------------------------------------------------
  // Get a friendly string version of the tag
  string stag();
};

//==========================================================================
// Basic asynchronous Tube client
class Client
{
protected:
  // Network stuff
  Net::EndPoint server;
  Net::TCPClient *socket;

  // Thread and queue stuff
  MT::Mutex mutex;             // Global client mutex used for socket
                               // creation and restart
  MT::Thread *send_thread;
  MT::MQueue<Message> send_q;
  int max_send_queue;          // Maximum send queue before we block send()

  MT::Thread *receive_thread;
  MT::MQueue<Message> receive_q;

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
  // Set maximum send queue
  void set_max_send_queue(int q) { max_send_queue = q; }

  //------------------------------------------------------------------------
  // Background functions called by threads - do not use directly
  bool receive_messages(Log::Streams& log);
  bool send_messages(Log::Streams& log);

  //------------------------------------------------------------------------
  // Send a message
  // Can busy-wait if send queue is more than max_send_queue
  void send(Message& msg);

  //------------------------------------------------------------------------
  // Check whether a message is available before blocking in wait()
  virtual bool poll();

  //------------------------------------------------------------------------
  // Receive a message - blocks waiting for one to arrive
  // Returns false if the connection was restarted 
  virtual bool wait(Message& msg);

  //------------------------------------------------------------------------
  // Shut down client cleanly
  virtual void shutdown();

  //------------------------------------------------------------------------
  // Destructor
  virtual ~Client();
};

//==========================================================================
// Synchronous request-response client, but still providing a wait() 
// interface for asynchronous messaging
class SyncClient: public Client
{
private:
  int timeout;      // Request timeout in seconds

  // Request record 
  struct Request
  {
    Time::Stamp started;
    Message response;
    MT::BasicCondVar ready;
    Request(): started(Time::Stamp::now()) {}
  };

  // Request map, by ID
  MT::Mutex request_mutex;
  id_t request_id;
  map<id_t, Request> requests; 

  // Thread to run timeouts
  MT::Thread *timeout_thread;

public:
  //------------------------------------------------------------------------
  // Constructor - takes server endpoint (address+port), request timeout
  // (in seconds) and optional name
  SyncClient(Net::EndPoint _server, int _timeout=DEFAULT_TIMEOUT, 
	     const string& _name="Tube");

  //------------------------------------------------------------------------
  // Handle timeouts - called by background thread - do not call directly
  void do_timeouts(Log::Streams& log);

  //------------------------------------------------------------------------
  // Request/response - blocks waiting for a response, or timeout/failure
  // Returns whether a response was received, fills in response if so
  bool request(Message& request, Message& response);

  //------------------------------------------------------------------------
  // Override of wait() which filters out responses, while leaving async
  // message to be returned normally
  // NB!  poll() will still return true for responses, so wait() may block
  bool wait(Message& msg);

  //------------------------------------------------------------------------
  // Shut down client cleanly
  virtual void shutdown();

  //------------------------------------------------------------------------
  // Destructor
  virtual ~SyncClient();
};

//==========================================================================
// Automatic synchronous request-response client, handling waiting internally
// and only providing a synchronous request() interface
class AutoSyncClient: public SyncClient
{
private:
  // Thread to run wait()
  MT::Thread *dispatch_thread;

public:
  //------------------------------------------------------------------------
  // Constructor - takes server endpoint (address+port), request timeout
  // (in seconds) and optional name
  AutoSyncClient(Net::EndPoint _server, int _timeout=DEFAULT_TIMEOUT, 
		 const string& _name="Tube");

  //------------------------------------------------------------------------
  // Overrideable function to handle an asynchronous message - by default
  // just errors
  virtual void handle_async_message(Message& msg);

  //------------------------------------------------------------------------
  // Shut down client cleanly
  virtual void shutdown();

  //------------------------------------------------------------------------
  // Destructor
  virtual ~AutoSyncClient();
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
  MT::MQueue<Message> send_q;  

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
  int max_send_queue;          // Maximum send queue before we block send()

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

  //------------------------------------------------------------------------
  // Set maximum send queue
  void set_max_send_queue(int q) { max_send_queue = q; }

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
  // Send a message
  // Note:  It is safe to call this inside the handle_message() method
  // Whether message queued (client still connected)
  bool send(ClientMessage& msg);
};

//==========================================================================
// Tube server for synchronous requests/responses
// Provides a simpler interface to handle request-response messages
// Also passes async messages to handle_async_message() but implements
// this here as just logging an error 
// Note send() can still be used to send async messages back
// Note:  Requires a thread to call run()
class SyncServer: public Server
{
private:
  //------------------------------------------------------------------------
  // Function to handle an incoming client message, called from parent
  bool handle_message(ClientMessage& msg);

  //------------------------------------------------------------------------
  // Abstract function to handle a request - implement in subclass
  // Return whether request handled OK, and fill in response
  virtual bool handle_request(ClientMessage& request, Message& response) = 0;

  //------------------------------------------------------------------------
  // Function to handle asynchronous messages (not requiring a response)
  // Implemented here just to log an error, but can be overridden if you
  // still need to receive async messages
  // Also called for STARTED and FINISHED psuedo-messages
  // Return whether connection should be allowed to continue
  virtual bool handle_async_message(ClientMessage& msg);

public:
  //------------------------------------------------------------------------
  // Constructor - as Server
  SyncServer(int port, const string& _name="Tube", int backlog=5, 
	     int min_spare_threads=1, int max_threads=10):
    Server(port, _name, backlog, min_spare_threads, max_threads) {}
};


//==========================================================================
// Tube server for synchronous requests/responses with its own run()
// thread 
class AutoSyncServer: public SyncServer
{
  Net::TCPServerThread run_thread;

public:
  //------------------------------------------------------------------------
 // Constructor - as Server
  AutoSyncServer(int port, const string& _name="Tube", int backlog=5, 
		 int min_spare_threads=1, int max_threads=10):
    SyncServer(port, _name, backlog, min_spare_threads, max_threads),
    run_thread(*this) {}
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_TUBE_H




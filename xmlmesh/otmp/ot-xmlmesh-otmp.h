//==========================================================================
// ObTools::XMLBus: ot-xmlbus-otmp.h
//
// Internal definitions for OTMP protocol, client and server
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#ifndef __OBTOOLS_XMLBUS_OTMP_H
#define __OBTOOLS_XMLBUS_OTMP_H

#include <string>
#include "ot-net.h"
#include "ot-mt.h"

namespace ObTools { namespace XMLBus {

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
const int OTMP_DEFAULT_PORT = 29167;

// Standard OTMP tags
enum OTMPTag
{
  OTMP_TAG_MESSAGE = 0x4f544d53   // OTMS - Message carrying
};

// Internal struct for carrying messages 
// This is NOT used for directly encoding the stream!
struct OTMPMessage
{
  // Length is implicit in data.size()
  uint32_t flags;
  string data;  

  OTMPMessage(const string& d="", int f=0): data(d), flags(f) {}
};

//==========================================================================
// OTMP client
class OTMPClient
{
private:
  // Network stuff
  Net::IPAddress server_address;
  int server_port;
  Net::TCPClient *socket;

  // Thread and queue stuff
  MT::Mutex mutex;             // Global client mutex used for socket
                               // creation and restart
  MT::Thread *send_thread;
  MT::Queue<OTMPMessage> send_q;

  MT::Thread *receive_thread;
  MT::Queue<OTMPMessage> receive_q;

  bool restart_socket();

public:
  //------------------------------------------------------------------------
  // Constructors - take server address
  // port=0 means use default port for protocol
  OTMPClient(Net::IPAddress address, int port=0);

  //------------------------------------------------------------------------
  // Background functions called by threads - do not use directly
  bool receive_messages();
  bool send_messages();

  //------------------------------------------------------------------------
  // Send a message - never blocks, but can fail if the queue is full
  // Whether message queued
  bool send(OTMPMessage& msg);

  //------------------------------------------------------------------------
  // Check whether a message is available before blocking in wait()
  bool poll();

  //------------------------------------------------------------------------
  // Receive a message - blocks waiting for one to arrive
  // Returns whether one was read - will only return false if something fails
  bool wait(OTMPMessage& msg);

  //------------------------------------------------------------------------
  // Destructor
  virtual ~OTMPClient();
};

//==========================================================================
// OTMP server
// Note, unlike the client it delivers messages to a given message queue
// rather than providing a poll/wait interface;  this is because it is
// expecting to aggregate its messages with a number of other servers
// Also, unlike the client, this _is_ a TCPServer, rather than owning
// one
class OTMPServer: public Net::TCPServer
{
private:
  // Thread and queue stuff
  MT::Thread *send_thread;
  MT::Queue<OTMPMessage> send_q;  

  MT::Thread *receive_thread;
  MT::Queue<OTMPMessage>& receive_q;  // Note:  Not mine

public:
  //------------------------------------------------------------------------
  // Constructor - takes receive queue for incoming messages
  // port=0 means take default port for protocol
  // The rest is thread/socket tuning - see Net::TCPServer
  OTMPServer(MT::Queue<OTMPMessage>& receive_queue,
	     int port=0, int backlog=5, 
	     int min_spare_threads=1, int max_threads=10);

  //------------------------------------------------------------------------
  // TCPServer process method - handles new connections
  void process(ObTools::Net::TCPSocket& s, 
	       ObTools::Net::IPAddress client_address,
	       int client_port);

  //------------------------------------------------------------------------
  // Background functions called by threads - do not use directly
  bool receive_messages();
  bool send_messages();

  //------------------------------------------------------------------------
  // Send a message - never blocks, but can fail if the queue is full
  // Whether message queued
  bool send(OTMPMessage& msg);
};


//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_XMLBUS_OTMP_H




//==========================================================================
// ObTools::XMLMesh:OTMP: server.cc
//
// Implementation of raw OTMP server 
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-xmlmesh-otmp.h"
#include "ot-log.h"

#include <unistd.h>
#include <netinet/in.h>
#include <sstream>

namespace ObTools { namespace XMLMesh { namespace OTMP {

//--------------------------------------------------------------------------
// Send handler thread class
// Pulls messages off the given queue and sends them to the given socket
class ServerSendThread: public MT::Thread
{
  ClientSession& session;
  Log::Streams log;  // Private to this thread

  void run() 
  { 
    for(;;)
    {
      // Block for a message
      Message msg = session.send_q.wait();

      // Deal with it
      OBTOOLS_LOG_IF_DEBUG(log.debug << "OTMP(ssend): Sending message length " 
			   << msg.data.size() 
			   << " (flags " << msg.flags << ")\n";)
      OBTOOLS_LOG_IF_DUMP(log.dump << msg.data << endl;)

      try // Handle SocketErrors
      {
	// Write chunk header
	session.socket.write_nbo_int(TAG_MESSAGE);
	session.socket.write_nbo_int(msg.data.size());
	session.socket.write_nbo_int(msg.flags); // Flags

	// Write data
	session.socket.write(msg.data);
      }
      catch (Net::SocketError se)
      {
	log.error << "OTMP(ssend): " << se << endl;
	cancel();
      }
    }
  }

public:
  ServerSendThread(ClientSession& _session): session(_session) { start(); }
};

//--------------------------------------------------------------------------
// TCPServer verify method
bool Server::verify(Net::EndPoint ep)
{
  // Check host against filters
  for(list<Net::MaskedAddress>::iterator p = filters.begin();
      p!=filters.end();
      p++)
    if (*p == ep.host) return true;

  Log::Stream error_log(Log::logger, Log::LEVEL_ERROR);
  error_log << "OTMP(serv): Rejected connection from " << ep << endl;
  return false;
}

//------------------------------------------------------------------------
// TCPServer process method - called in worker thread to handle connection
void Server::process(Net::TCPSocket& socket, 
		     Net::EndPoint client)
{
  Log::Streams log;  // Our private log

  const char *obit = "ended";

  log.summary << "OTMP(serv): Got connection from " << client << endl;

  // Create client session and map it (autoremoved on destruction)
  ClientSession session(socket, client, client_sessions);

  // Start send thread
  ServerSendThread send_thread(session);

  // Tell the queue the client has arrived
  ClientMessage bmsg(client, ClientMessage::STARTED);
  receive_q.send(bmsg);

  // Loop receiving messages and posting to receive_q
  // Stop if send thread unhappy, too
  while (!!socket && !!send_thread)
  {
    try
    {
      // Try to read a 4-byte tag
      uint32_t tag;
      if (!socket.read_nbo_int(tag)) break;  // Clean shutdown

      if (tag == TAG_MESSAGE)
      {
	// Handle a TLV block
	uint32_t len   = socket.read_nbo_int();
	uint32_t flags = socket.read_nbo_int();

	OBTOOLS_LOG_IF_DEBUG(log.debug << "OTMP(srecv): Message length " 
			     << len << " (flags " << flags << ")\n";)

	// Read the data
	string content;
	if (!socket.read(content, len))
	{
	  log.error << "OTMP(srecv): Short message read - socket died\n";
	  obit = "died";
	  break;
	}

	OBTOOLS_LOG_IF_DUMP(log.dump << content << endl;)

	// Post up a message
	ClientMessage msg(client, content, flags);
	receive_q.send(msg);
      }
      else
      {
	// Unrecognised tag
	log.error << "OTMP(recv): Unrecognised tag " 
		  << hex << tag << dec << " - out-of-sync?\n";
	obit = "unsynced";
	break;
      }
    }
    catch (Net::SocketError se)
    {
      log.error << "OTMP(recv): " << se << endl;
      obit = "failed";
      break;
    }
  }

  // Tell the queue the client has gone
  ClientMessage emsg(client, ClientMessage::FINISHED);
  receive_q.send(emsg);

  if (!!send_thread) 
    send_thread.cancel();
  else
    obit = "failed (send)";

  log.summary << "OTMP(serv): Connection from " << client
	      << " " << obit << endl;
} 


//------------------------------------------------------------------------
// Constructor
Server::Server(ClientMessageQueue& receive_queue,
	       int port, int backlog, 
	       int min_spare_threads, int max_threads):
  TCPServer((port?port:DEFAULT_PORT), backlog, 
	    min_spare_threads, max_threads),
  receive_q(receive_queue)
{

}


//------------------------------------------------------------------------
// Send a message - never blocks, but can fail if the queue is full
// Whether message queued
bool Server::send(ClientMessage& msg)
{
  // Look up client in map, and queue it on there
  SessionMap::iterator p = client_sessions.find(msg.client);
  if (p != client_sessions.end())
  {
    ClientSession *cs = p->second;
    cs->send_q.send(msg.msg);
    return true;  
  }

  return false;
}

}}} // namespaces





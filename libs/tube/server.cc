//==========================================================================
// ObTools::Tube: server.cc
//
// Implementation of tube server 
//
// Copyright (c) 2007 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-tube.h"
#include "ot-log.h"

#include <unistd.h>
#include <sstream>

namespace ObTools { namespace Tube {

//--------------------------------------------------------------------------
// Send handler thread class
// Pulls messages off the given queue and sends them to the given socket
class ServerSendThread: public MT::Thread
{
  ClientSession& session;
  Log::Streams log;  // Private to this thread

  void run() 
  { 
    while (session.alive)
    {
      // Block for a message
      Message msg = session.send_q.wait();
      if (!session.alive) break;

      // Deal with it
      OBTOOLS_LOG_IF_DEBUG(log.debug << "tube(ssend): Sending message "
			   << hex << msg.tag << dec << ", length " 
			   << msg.data.size() 
			   << " (flags " << msg.flags << ")\n";)
      OBTOOLS_LOG_IF_DUMP(log.dump << msg.data << endl;)

      try // Handle SocketErrors
      {
	// Write chunk header
	session.socket.write_nbo_int(msg.tag);
	session.socket.write_nbo_int(msg.data.size());
	session.socket.write_nbo_int(msg.flags); // Flags

	// Write data
	session.socket.write(msg.data);
      }
      catch (Net::SocketError se)
      {
	log.error << "tube(ssend): " << se << endl;
	break;
      }
    }

    OBTOOLS_LOG_IF_DEBUG(log.debug << "tube(ssend): Thread shutting down\n";)
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
  error_log << "tube(serv): Rejected connection from " << ep << endl;
  return false;
}

//------------------------------------------------------------------------
// TCPServer process method - called in worker thread to handle connection
void Server::process(Net::TCPSocket& socket, 
		     Net::EndPoint client)
{
  Log::Streams log;  // Our private log

  const char *obit = "ended";

  log.summary << "tube(serv): Got connection from " << client << endl;

  // Create client session and map it (autoremoved on destruction)
  ClientSession session(socket, client, client_sessions);

  // Start send thread and detach it
  ServerSendThread send_thread(session);
  send_thread.detach();

  // Tell the queue the client has arrived
  ClientMessage bmsg(client, ClientMessage::STARTED);
  handle_message(bmsg);

  // Loop receiving messages and posting to receive_q
  // Stop if send thread unhappy, too
  while (alive && !!socket && !!send_thread)
  {
    try
    {
      // Try to read a 4-byte tag
      uint32_t tag;
      if (!socket.read_nbo_int(tag) || !alive) break;  // Clean shutdown

      // Verify tag
      if (tag_recognised(tag))
      {
	// Handle a TLV block
	uint32_t len   = socket.read_nbo_int();
	uint32_t flags = socket.read_nbo_int();

	OBTOOLS_LOG_IF_DEBUG(log.debug << "tube(srecv): Message "
			     << hex << tag << dec << ", length " 
			     << len << " (flags " << flags << ")\n";)

	// Read the data
	string content;
	if (!socket.read(content, len))
	{
	  log.error << "tube(srecv): Short message read - socket died\n";
	  obit = "died";
	  break;
	}

	OBTOOLS_LOG_IF_DUMP(log.dump << content << endl;)

	// Post up a message
	ClientMessage msg(client, tag, content, flags);
	if (!handle_message(msg))
	{
	  obit = "killed by server";
	  break;
	}
      }
      else
      {
	// Unrecognised tag
	log.error << "tube(recv): Unrecognised tag " 
		  << hex << tag << dec << " - out-of-sync?\n";
	obit = "unsynced";
	break;
      }
    }
    catch (Net::SocketError se)
    {
      log.error << "tube(srecv): " << se << endl;
      obit = "failed";
      break;
    }
  }

  // Tell the server the client has gone
  ClientMessage emsg(client, ClientMessage::FINISHED);
  handle_message(emsg);

  if (!!send_thread)
  {
    OBTOOLS_LOG_IF_DEBUG(log.debug << "tube(serv): Shutting down send\n";)

    // Shut down session cleanly
    session.alive=false;

    // Kill socket
    socket.shutdown();

    // Wait for it to die
    while (!!send_thread)
    {
      session.send_q.send(Message());  // Wake up thread with bogus message
      MT::Thread::usleep(50000);
      if (!send_thread) break;
      log.error << "Send thread won't die - waiting\n";
      MT::Thread::usleep(250000);
    }
  }
  else
    obit = "failed (send)";

  log.summary << "tube(serv): Connection from " << client
	      << " " << obit << endl;
} 


//------------------------------------------------------------------------
// Constructor
Server::Server(int port, int backlog, 
	       int min_spare_threads, int max_threads):
  TCPServer(port, backlog, min_spare_threads, max_threads),
  alive(true)
{

}

//------------------------------------------------------------------------
// Send a message - never blocks, but can fail if the queue is full
// Note:  It is safe to call this inside the handle_message() method
// Whether message queued
bool Server::send(ClientMessage& msg)
{
  // Look up client in map, and queue it on there
  MT::RWReadLock lock(client_sessions.mutex);
  map<Net::EndPoint, ClientSession *>::iterator p 
    = client_sessions.sessions.find(msg.client);
  if (p != client_sessions.sessions.end())
  {
    ClientSession *cs = p->second;
    cs->send_q.send(msg.msg);
    return true;  
  }

  return false;
}

}} // namespaces





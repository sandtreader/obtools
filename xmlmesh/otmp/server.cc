//==========================================================================
// ObTools::XMLBus: server.cc
//
// Implementation of raw OTMP server 
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-xmlbus-otmp.h"
#include "ot-log.h"

#include <unistd.h>
#include <netinet/in.h>
#include <sstream>

namespace ObTools { namespace XMLBus { namespace OTMP {

//--------------------------------------------------------------------------
// Send handler thread class
// Pulls messages off the given queue and sends them to the given socket
class SendThread: public MT::Thread
{
  MT::Queue<Message>& send_q;
  Net::TCPSocket& socket;

  void run() 
  { 
    for(;;)
    {
      // Block for a message
      Message msg = send_q.wait();

      // Deal with it
      if (Log::debug_ok)
	Log::Debug << "OTMP(ssend): Sending message length " 
		   << msg.data.size() 
		   << " (flags " << msg.flags << ")\n";
      if (Log::dump_ok) Log::Dump << msg.data << endl;

      try // Handle SocketErrors
      {
	// Write chunk header
	socket.write_nbo_int(TAG_MESSAGE);
	socket.write_nbo_int(msg.data.size());
	socket.write_nbo_int(msg.flags); // Flags

	// Write data
	socket.write(msg.data);
      }
      catch (Net::SocketError se)
      {
	Log::Error << "OTMP(ssend): " << se << endl;
	cancel();
      }
    }
  }

public:
  SendThread(MT::Queue<Message>& q, Net::TCPSocket &s): 
    send_q(q), socket(s) { start(); }
};

//------------------------------------------------------------------------
// TCPServer process method - called in worker thread to handle connection
void Server::process(Net::TCPSocket& socket, 
			 Net::IPAddress client_address,
			 int client_port)
{
  const char *obit = "ended";

  Log::Summary << "OTMP(serv): Got connection from " << client_address 
	       << ":" << client_port << endl;
  
  // Start send thread
  SendThread send_thread(send_q, socket);

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

	if (Log::debug_ok)
	  Log::Debug << "OTMP(srecv): Message length " << len 
		     << " (flags " << flags << ")\n";

	// Read the data
	string content;
	if (!socket.read(content, len))
	{
	  Log::Error << "OTMP(srecv): Short message read - socket died\n";
	  obit = "died";
	  break;
	}

	if (Log::dump_ok) Log::Dump << content << endl;

	// Post up a message
	Message msg(content, flags);
	receive_q.send(msg);
      }
      else
      {
	// Unrecognised tag
	Log::Error << "OTMP(recv): Unrecognised tag " 
		   << hex << tag << dec << " - out-of-sync?\n";
	obit = "unsynced";
	break;
      }
    }
    catch (Net::SocketError se)
    {
      Log::Error << "OTMP(recv): " << se << endl;
      obit = "failed";
      break;
    }
  }

  if (!!send_thread) 
    send_thread.cancel();
  else
    obit = "failed (send)";

  Log::Summary << "OTMP(serv): Connection from " << client_address 
	       << ":" << client_port << " " << obit << endl;
} 


//------------------------------------------------------------------------
// Constructor
Server::Server(MT::Queue<Message>& receive_queue,
		       int port, int backlog, 
		       int min_spare_threads, int max_threads):
  receive_q(receive_queue),
  TCPServer((port?port:DEFAULT_PORT), backlog, 
	    min_spare_threads, max_threads)
{

}


//------------------------------------------------------------------------
// Send a message - never blocks, but can fail if the queue is full
// Whether message queued
bool Server::send(Message& msg)
{
  send_q.send(msg);  // Never fails, will eat all memory first
  return true;  
}

}}} // namespaces





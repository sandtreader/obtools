//==========================================================================
// ObTools::XMLBus: client.cc
//
// Implementation of raw OTMP client 
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-xmlbus-otmp.h"
#include "ot-log.h"

#include <unistd.h>
#include <netinet/in.h>
#include <sstream>

// Time to sleep for if socket dies and won't come back
#define DEAD_SOCKET_SLEEP_TIME 10

namespace ObTools { namespace XMLBus {

//==========================================================================
// Background traffic handler threads

//--------------------------------------------------------------------------
// Receive handler thread class
// Just repeatedly calls back into receive_messages
class ReceiveThread: public MT::Thread
{
  OTMPClient& client;

  void run() 
  { 
    for(;;)
    {
      // Loop while socket is happy
      while (client.receive_messages());

      // Log fault and sleep before retrying
      Log::Error << "OTMP(recv): Socket failed, can't restart\n";
      Log::Error << "OTMP(recv): Sleeping for " 
		 << DEAD_SOCKET_SLEEP_TIME << " seconds\n";
      sleep(DEAD_SOCKET_SLEEP_TIME);
    }
  }

public:
  ReceiveThread(OTMPClient &_client): client(_client) { start(); }
};

//------------------------------------------------------------------------
// Receive some messages, if any
// Blocks waiting for incoming messages, returns whether everything OK
bool OTMPClient::receive_messages()
{
  // Check socket exists and is connected - if not, try to reconnect it
  if ((!socket || !*socket) && !restart_socket()) return false;

  // Wait for message to come in and post it up
  try // Handle SocketErrors
  {
    // Read a 4-byte tag
    uint32_t tag = socket->read_nbo_int();

    switch (tag)
    {
      case OTMP_TAG_PING:
      case OTMP_TAG_MESSAGE:
	// Handle a TLV block
	break;

      default:
	// Unrecognised tag
	Log::Error << "OTMP(recv): Unrecognised tag - out-of-sync?\n";
	//Try to restart socket
	return restart_socket();
    }

    uint32_t len   = socket->read_nbo_int();
    uint32_t flags = socket->read_nbo_int();

    switch (tag)
    {
      case OTMP_TAG_PING:
	// !!! handle it
	break;

      case OTMP_TAG_MESSAGE:
	if (Log::debug_ok)
	  Log::Debug << "OTMP(recv): Message length " << len 
		     << " (flags " << flags << ")\n";
	// Read the data
	string content;
	if (!socket->read(content, len))
	{
	  Log::Error << "OTMP(recv): Short message read - socket died\n";
	  return restart_socket();
	}

	if (Log::dump_ok) Log::Dump << content << endl;
	break;
    }
  }
  catch (Net::SocketError se)
  {
    Log::Error << "OTMP(recv): " << se << endl;
    //Try to restart socket
    return restart_socket();
  }

  return true;
}

//--------------------------------------------------------------------------
// Send handler thread class
// Just repeatedly calls back into send_messages
class SendThread: public MT::Thread
{
  OTMPClient& client;

  void run() 
  { 
    for(;;)
    {
      // Loop while socket is happy
      while (client.send_messages());

      // Log fault and sleep before retrying
      Log::Error << "OTMP(send): Socket failed, can't restart\n";
      Log::Error << "OTMP(send): Sleeping for " 
		 << DEAD_SOCKET_SLEEP_TIME << " seconds\n";
      sleep(DEAD_SOCKET_SLEEP_TIME);
    }
  }

public:
  SendThread(OTMPClient &_client): client(_client) { start(); }
};

//------------------------------------------------------------------------
// Send out some messages, if any
// Blocks waiting for outgoing messages, returns whether everything OK
bool OTMPClient::send_messages()
{
  // Check socket exists and is connected - if not, try to reconnect it
  if ((!socket || !*socket) && !restart_socket()) return false;

  // Wait for message to go out, and send it
  OTMPMessage msg = send_q.wait();
 
  // Deal with it
  if (Log::debug_ok)
    Log::Debug << "OTMP(send): Sending message length " << msg.data.size() 
	       << " (flags " << msg.flags << ")\n";
  if (Log::dump_ok) Log::Dump << msg.data << endl;

  try // Handle SocketErrors
  {
    // Write chunk header
    socket->write_nbo_int(OTMP_TAG_MESSAGE);
    socket->write_nbo_int(msg.data.size());
    socket->write_nbo_int(msg.flags); // Flags

    // Write data
    socket->write(msg.data);
  }
  catch (Net::SocketError se)
  {
    Log::Error << "OTMP(send): " << se << endl;
    //Try to restart socket
    return restart_socket();
  }

  return true;
}

//------------------------------------------------------------------------
// Restart a dead or non-existent socket
bool OTMPClient::restart_socket()
{
  // Lock client global mutex while we mess with this
  MT::Lock lock(mutex);

  // If we have a socket, kill it
  if (socket) 
  {
    delete socket;
    socket = 0;
    Log::Summary << "OTMP: Restarting socket\n";
  }

  // Try and get a new one
  socket = new Net::TCPClient(server_address, server_port);

  if (!*socket)
  {
    delete socket;
    socket = 0;
    Log::Error << "OTMP: Can't open socket to " 
	       << server_address << " port " << server_port << endl;
    return false;
  }
  else
  {
    Log::Summary << "OTMP: Opened socket to " 
		 << server_address << " port " << server_port << endl;

    return true;
  }
}


//==========================================================================
// Foreground stuff

//------------------------------------------------------------------------
// Constructors
OTMPClient::OTMPClient(Net::IPAddress address, int port):
  server_address(address),
  server_port(port?port:OTMP_DEFAULT_PORT) 
{
  socket = 0;

  //Try to start socket
  restart_socket();

  //Start send and receive threads
  receive_thread = new ReceiveThread(*this);
  send_thread    = new SendThread(*this);
}


//------------------------------------------------------------------------
// Send a message - never blocks, but can fail if the queue is full
// Whether message queued
bool OTMPClient::send(OTMPMessage& msg)
{
  send_q.send(msg);  // Never fails, will eat all memory first
  return true;  
}

//------------------------------------------------------------------------
// Check whether a message is available before blocking in wait()
bool OTMPClient::poll()
{
  return receive_q.poll();
}

//------------------------------------------------------------------------
// Receive a message - blocks waiting for one to arrive
// Returns whether one was read - will only return false if something fails
bool OTMPClient::wait(OTMPMessage& msg)
{
  msg = receive_q.wait();  // Never fails
  return true;
}

//------------------------------------------------------------------------
// Destructor
OTMPClient::~OTMPClient()
{
  delete receive_thread;
  delete send_thread;
  if (socket) delete socket;
}

}} // namespaces





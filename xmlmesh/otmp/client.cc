//==========================================================================
// ObTools::XMLMesh:OTMP: client.cc
//
// Implementation of raw OTMP client 
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-xmlmesh-otmp.h"
#include "ot-log.h"

#include <unistd.h>
#include <netinet/in.h>
#include <sstream>

// Time to sleep for if socket dies and won't come back
#define DEAD_SOCKET_SLEEP_TIME 10

namespace ObTools { namespace XMLMesh { namespace OTMP {

//==========================================================================
// Background traffic handler threads

//--------------------------------------------------------------------------
// Receive handler thread class
// Just repeatedly calls back into receive_messages
class ReceiveThread: public MT::Thread
{
  Client& client;

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
  ReceiveThread(Client &_client): client(_client) { start(); }
};

//------------------------------------------------------------------------
// Restart a dead or non-existent socket
// Note:  Ensure this is called only in one thread (the receive thread)
bool Client::restart_socket()
{
  // Lock the mutex while we try to restart
  MT::Lock lock(mutex);

  // Delete old socket, if any
  if (socket) delete socket;

  // Try and get a new one
  socket = new Net::TCPClient(server);

  if (!*socket)
  {
    // Leave it valid but unhappy to avoid crashing send thread

    Log::Error << "OTMP: Can't open socket to " << server << endl;
    return false;
  }
  else
  {
    Log::Summary << "OTMP: Opened socket to " << server << endl;
    return true;
  }
}

//------------------------------------------------------------------------
// Receive some messages, if any
// Blocks waiting for incoming messages, returns whether everything OK
bool Client::receive_messages()
{
  // Check socket exists and is connected - if not, try to reconnect it
  if ((!socket || !*socket) && !restart_socket()) return false;

  // Wait for message to come in and post it up
  try // Handle SocketErrors
  {
    // Read a 4-byte tag
    uint32_t tag = socket->read_nbo_int();

    if (tag == TAG_MESSAGE)
    {
      // Handle a TLV block
      uint32_t len   = socket->read_nbo_int();
      uint32_t flags = socket->read_nbo_int();

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

      // Post up a message
      Message msg(content, flags);
      receive_q.send(msg);
    }
    else
    {
      // Unrecognised tag
      Log::Error << "OTMP(recv): Unrecognised tag - out-of-sync?\n";
      //Try to restart socket
      return restart_socket();
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
  Client& client;

  void run() 
  { 
    for(;;) client.send_messages();
  }

public:
  SendThread(Client &_client): client(_client) { start(); }
};

//------------------------------------------------------------------------
// Send out some messages, if any
// Blocks waiting for outgoing messages, returns whether everything OK
bool Client::send_messages()
{
  // Wait for message to go out, and send it
  Message msg = send_q.wait();
 
  // Check that socket is OK - if not, sleep hoping the receive thread
  // can reanimate it
  while (!socket || !*socket)
  {
    Log::Summary << "OTMP(send): Socket is dead - waiting for improvement\n";
    sleep(DEAD_SOCKET_SLEEP_TIME);
  }

  // Deal with it
  if (Log::debug_ok)
    Log::Debug << "OTMP(send): Sending message length " << msg.data.size() 
	       << " (flags " << msg.flags << ")\n";
  if (Log::dump_ok) Log::Dump << msg.data << endl;

  try // Handle SocketErrors
  {
    // Lock the mutex while we use the socket - the receive thread's restart
    // might jump in here and kill it under us, otherwise
    MT::Lock lock(mutex);

    // Write chunk header
    socket->write_nbo_int(TAG_MESSAGE);
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


//==========================================================================
// Foreground stuff

//------------------------------------------------------------------------
// Constructors
Client::Client(Net::EndPoint _server): server(_server)
{
  socket = 0;

  //Start send and receive threads - receive thread will 'restart' socket
  receive_thread = new ReceiveThread(*this);
  send_thread    = new SendThread(*this);
}


//------------------------------------------------------------------------
// Send a message - never blocks, but can fail if the queue is full
// Whether message queued
bool Client::send(Message& msg)
{
  send_q.send(msg);  // Never fails, will eat all memory first
  return true;  
}

//------------------------------------------------------------------------
// Check whether a message is available before blocking in wait()
bool Client::poll()
{
  return receive_q.poll();
}

//------------------------------------------------------------------------
// Receive a message - blocks waiting for one to arrive
// Returns whether one was read - will only return false if something fails
bool Client::wait(Message& msg)
{
  msg = receive_q.wait();  // Never fails
  return true;
}

//------------------------------------------------------------------------
// Destructor
Client::~Client()
{
  delete receive_thread;
  delete send_thread;
  if (socket) delete socket;
}

}}} // namespaces





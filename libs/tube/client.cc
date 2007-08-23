//==========================================================================
// ObTools::Tube: client.cc
//
// Implementation of tube client
//
// Copyright (c) 2007 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-tube.h"
#include "ot-log.h"

#include <unistd.h>
#include <sstream>

// Time to sleep for if socket dies and won't come back
#define DEAD_SOCKET_SLEEP_TIME 10

namespace ObTools { namespace Tube {

//==========================================================================
// Background traffic handler threads

//--------------------------------------------------------------------------
// Receive handler thread class
// Just repeatedly calls back into receive_messages
class ClientReceiveThread: public MT::Thread
{
  Client& client;
  Log::Streams log;  // Private to this thread

  void run() 
  { 
    while (client.is_alive())
    {
      // Loop while socket is happy
      while (client.receive_messages(log));

      if (client.is_alive())
      {
	// Log fault and sleep before retrying
	log.error << client.name << " (recv): Socket failed, can't restart\n";
	log.error << client.name << " (recv): Sleeping for " 
		  << DEAD_SOCKET_SLEEP_TIME << " seconds\n";
	sleep(DEAD_SOCKET_SLEEP_TIME);
      }
    }

    OBTOOLS_LOG_IF_DEBUG(log.debug << client.name 
			 << " (recv): Thread shut down\n";)
  }

public:
  ClientReceiveThread(Client &_client): client(_client) { start(); }
};

//------------------------------------------------------------------------
// Restart a dead or non-existent socket
// Note:  Ensure this is called only in one thread (the receive thread)
bool Client::restart_socket(Log::Streams& log)
{
  // Lock the mutex while we try to restart
  MT::Lock lock(mutex);

  // Remember if we had a socket before (not just starting)
  bool starting = (socket==0);

  // Delete old socket, if any
  if (socket) delete socket;

  // Try and get a new one
  socket = new Net::TCPClient(server);

  if (!*socket)
  {
    // Leave it valid but unhappy to avoid crashing send thread

    log.error << name << ": Can't open socket to " << server << endl;
    return false;
  }
  else
  {
    log.detail << name << ": Opened socket to " << server << endl;

    if (!starting)
    {
      // Send empty message to flag restart
      Message msg;
      receive_q.send(msg);
    }

    return true;
  }
}

//------------------------------------------------------------------------
// Receive some messages, if any
// Blocks waiting for incoming messages, returns whether everything OK
bool Client::receive_messages(Log::Streams& log)
{
  // Check socket exists and is connected - if not, try to reconnect it
  if ((!socket || !*socket) && !restart_socket(log)) return false;

  // Wait for message to come in and post it up
  try // Handle SocketErrors
  {
    // Read a 4-byte tag
    uint32_t tag = socket->read_nbo_int();

    // Verify acceptability
    if (tag_recognised(tag))
    {
      // Handle a TLV block
      uint32_t len   = socket->read_nbo_int();
      uint32_t flags = socket->read_nbo_int();

      OBTOOLS_LOG_IF_DEBUG(log.debug << name << " (recv): Message "
			   << hex << tag << dec << ", length " << len 
			   << " (flags " << flags << ")\n";)

      // Read the data
      string content;
      if (!socket->read(content, len))
      {
	log.error << name << " (recv): Short message read - socket died\n";
	return restart_socket(log);
      }

      OBTOOLS_LOG_IF_DUMP(log.dump << content << endl;)

      // Post up a message
      Message msg(tag, content, flags);
      receive_q.send(msg);
    }
    else
    {
      // Unrecognised tag
      log.error << name << " (recv): Unrecognised tag " 
		<< hex << tag << dec << " - out-of-sync?\n";
      //Try to restart socket
      return restart_socket(log);
    }
  }
  catch (Net::SocketError se)
  {
    if (alive)
    {
      log.error << name << " (recv): " << se << endl;
      return restart_socket(log);
    }
    else return false;
  }

  return true;
}

//--------------------------------------------------------------------------
// Send handler thread class
// Just repeatedly calls back into send_messages
class ClientSendThread: public MT::Thread
{
  Client& client;
  Log::Streams log;  // Private to this thread

  void run() 
  { 
    while (client.is_alive()) client.send_messages(log);

    OBTOOLS_LOG_IF_DEBUG(log.debug << client.name << " (send): Thread shut down\n";)
  }

public:
  ClientSendThread(Client &_client): client(_client) { start(); }
};

//------------------------------------------------------------------------
// Send out some messages, if any
// Blocks waiting for outgoing messages, returns whether everything OK
bool Client::send_messages(Log::Streams& log)
{
  // Wait for message to go out, and send it
  Message msg = send_q.wait();

  // Check if we're still alive
  if (!alive) return false;

  // Check that socket is OK - if not, sleep hoping the receive thread
  // can reanimate it
  while (!socket || !*socket)
  {
    log.detail << name <<" (send): Socket is dead - waiting for improvement\n";
    MT::Thread::sleep(DEAD_SOCKET_SLEEP_TIME);
  }

  // Deal with it
  OBTOOLS_LOG_IF_DEBUG(log.debug << name << " (send): Sending message "
		       << hex << msg.tag << dec << ", length " 
		       << msg.data.size() << " (flags " << msg.flags << ")\n";)
  OBTOOLS_LOG_IF_DUMP(log.dump << msg.data << endl;)

  try // Handle SocketErrors
  {
    // Lock the mutex while we use the socket - the receive thread's restart
    // might jump in here and kill it under us, otherwise
    MT::Lock lock(mutex);

    // Write chunk header
    socket->write_nbo_int(msg.tag);
    socket->write_nbo_int(msg.data.size());
    socket->write_nbo_int(msg.flags); // Flags

    // Write data
    socket->write(msg.data);
  }
  catch (Net::SocketError se)
  {
    if (alive)
    {
      log.error << name << " (send): " << se << endl;
      //Try to restart socket
      return restart_socket(log);
    }
    else return false;
  }

  return true;
}

//==========================================================================
// Foreground stuff

//------------------------------------------------------------------------
// Constructors
Client::Client(Net::EndPoint _server, const string& _name): 
  server(_server), alive(true), name(_name)
{
  socket = 0;

  //Start send and receive threads - receive thread will 'restart' socket
  receive_thread = new ClientReceiveThread(*this);
  send_thread    = new ClientSendThread(*this);
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
// Returns false if the connection was restarted 
bool Client::wait(Message& msg)
{
  msg = receive_q.wait();    // Never fails
  return msg.data.size()!=0; // Empty message on restart
}

//------------------------------------------------------------------------
// Shut down client cleanly
void Client::shutdown()
{
  if (alive)
  {
    alive = false; // Don't try to restart socket when we close it

    // Shutdown the socket, to force failure on blocking calls in threads
    if (socket) socket->shutdown();

    // Send a bogus message on the queue to force the send thread to wake up
    send_q.send(Message());

    // Wait for threads to exit cleanly
    for(int i=0; i<5; i++)
    {
      if (!*receive_thread && !*send_thread) break;
      MT::Thread::usleep(10000);
    }

    // If still not dead, cancel them
    if (!!*receive_thread) receive_thread->cancel();
    if (!!*send_thread) send_thread->cancel();
  }
}

//------------------------------------------------------------------------
// Destructor
Client::~Client()
{
  shutdown();

  // Now it's safe to delete them
  delete receive_thread;
  delete send_thread;
  if (socket) delete socket;
}

}} // namespaces





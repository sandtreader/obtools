//==========================================================================
// ObTools::Tube: client.cc
//
// Implementation of tube client
//
// Copyright (c) 2007 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-tube.h"
#include "ot-log.h"
#include "ot-misc.h"

// Timeout on initial connect
#define SOCKET_CONNECT_TIMEOUT 5

// Time to sleep for if socket dies and won't come back
#define DEAD_SOCKET_SLEEP_TIME 10
#define RESTART_SOCKET_SLEEP_TIME 1

// Default maximum send queue length
#define DEFAULT_MAX_SEND_QUEUE 1024

// Time to wait (us) if send queue full
#define SEND_BUSY_WAIT_TIME 10

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
      while (client.receive_messages(log)) {}

      if (client.is_alive())
      {
        // Log fault and sleep before retrying
        log.error << client.name << " (recv): Socket failed, can't restart\n";
        log.error << client.name << " (recv): Sleeping for "
                  << DEAD_SOCKET_SLEEP_TIME << " seconds\n";

        // Sleep, checking for shutdown
        for(int i=0; client.is_alive() && i<100*DEAD_SOCKET_SLEEP_TIME; i++)
          this_thread::sleep_for(chrono::milliseconds{10});
      }
    }

    OBTOOLS_LOG_IF_DEBUG(log.debug << client.name
                         << " (recv): Thread shut down\n";)
  }

public:
  ClientReceiveThread(Client &_client): client(_client) {}
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
  if (socket)
  {
    // Ensure we zero the pointer for anyone who's watching before we
    // delete
    SSL::TCPSocket *dead_socket = socket;
    socket = 0;
    delete dead_socket;
  }

  // Try and get a new one
  socket = new SSL::TCPClient(ctx, server, SOCKET_CONNECT_TIMEOUT);

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
  if (!check_socket() && !restart_socket(log)) return false;

  // Wait for message to come in and post it up
  try // Handle SocketErrors
  {
    // Read a 4-byte tag
    uint32_t tag = socket->read_nbo_int();

    Message msg(tag);

    // Verify acceptability
    if (tag_recognised(tag))
    {
      // Handle a TLV block
      uint32_t len   = socket->read_nbo_int();
      msg.flags = socket->read_nbo_int();

      OBTOOLS_LOG_IF_DEBUG(log.debug << name << " (recv): Message "
                           << msg.stag() << ", length " << len
                           << " (flags " << hex << msg.flags << dec << ")\n";)

      // Read the data
      if (!socket->read(msg.data, len))
      {
        log.error << name << " (recv): Short message read - socket died\n";
        return restart_socket(log);
      }

      OBTOOLS_LOG_IF_DUMP(Misc::Dumper dumper(log.dump);
                          dumper.dump(msg.data);)

      // Post up a message
      receive_q.send(msg);
    }
    else
    {
      // Unrecognised tag
      log.error << name << " (recv): Unrecognised tag "
                << msg.stag() << " - out-of-sync?\n";
      //Try to restart socket
      return restart_socket(log);
    }
  }
  catch (Net::SocketError se)
  {
    if (alive)
    {
      log.error << name << " (recv): " << se << endl;

      // Sleep, checking for shutdown
      for(int i=0; alive && i<100*RESTART_SOCKET_SLEEP_TIME; i++)
        this_thread::sleep_for(chrono::milliseconds{10});

      // Restart if not shut down
      if (alive)
      {
        log.summary << name << " (recv): Attempting to restart socket\n";
        return restart_socket(log);
      }
    }

    return false;
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
  ClientSendThread(Client &_client): client(_client) {}
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
  while (!check_socket())
  {
    log.detail << name <<" (send): Socket is dead - waiting for improvement\n";
    for(int i=0; alive && i<100*DEAD_SOCKET_SLEEP_TIME; i++)
      this_thread::sleep_for(chrono::milliseconds{10});
    if (!alive) return false;
  }

  // Deal with it
  OBTOOLS_LOG_IF_DEBUG(log.debug << name << " (send): Sending message "
                       << msg.stag() << ", length "
                       << msg.data.size()
                       << " (flags " << hex << msg.flags << dec << ")\n";)
  OBTOOLS_LOG_IF_DUMP(Misc::Dumper dumper(log.dump);
                      dumper.dump(msg.data);)

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

      // Sleep, checking for shutdown
      for(int i=0; alive && i<100*RESTART_SOCKET_SLEEP_TIME; i++)
        this_thread::sleep_for(chrono::milliseconds{10});

      // Try to restart socket if not shut down
      if (alive)
      {
        log.summary << name << " (send): Attempting to restart socket\n";
        return restart_socket(log);
      }
    }

    return false;
  }

  return true;
}

//==========================================================================
// Foreground stuff

//------------------------------------------------------------------------
// Constructor - no SSL
Client::Client(const Net::EndPoint& _server, const string& _name):
  server(_server), ctx(0), max_send_queue(DEFAULT_MAX_SEND_QUEUE),
  alive(true), name(_name)
{
  socket = 0;

  // Try to start socket the first time, to try to ensure it's up before we
  // start to send messages
  Log::Streams log;
  restart_socket(log);

  // Start send and receive threads
  receive_thread = new ClientReceiveThread(*this);
  send_thread    = new ClientSendThread(*this);
}

//------------------------------------------------------------------------
// Constructor with SSL
Client::Client(const Net::EndPoint& _server, SSL::Context *_ctx,
               const string& _name):
  server(_server), ctx(_ctx), max_send_queue(DEFAULT_MAX_SEND_QUEUE),
  alive(true), name(_name)
{
  socket = 0;

  // Try to start socket the first time, to try to ensure it's up before we
  // start to send messages
  Log::Streams log;
  restart_socket(log);

  // Start send and receive threads
  receive_thread = new ClientReceiveThread(*this);
  send_thread    = new ClientSendThread(*this);
}

//------------------------------------------------------------------------
// Start send and receive threads
void Client::start()
{
  if (receive_thread)
    receive_thread->start();
  if (send_thread)
    send_thread->start();
}

//------------------------------------------------------------------------
// Send a message
// Can busy-wait if send queue is more than max_send_queue
void Client::send(Message& msg)
{
  while (send_q.waiting() > max_send_queue)  // Must allow zero to work
    this_thread::sleep_for(chrono::milliseconds{SEND_BUSY_WAIT_TIME});

  send_q.send(msg);
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
  return msg.is_valid();     // Empty message on restart
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

    // Likewise on the receive queue to tell the user
    receive_q.send(Message());

    // Wait for threads to exit cleanly - enough time for a TCP connection
    // to time out fully, and then some
    for(int i=0; i<SOCKET_CONNECT_TIMEOUT*100+50; i++)
    {
      if (!*receive_thread && !*send_thread) break;
      this_thread::sleep_for(chrono::milliseconds{10});
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
  receive_thread = 0;
  delete send_thread;
  send_thread = 0;
  if (socket) delete socket;
}

}} // namespaces





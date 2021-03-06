//==========================================================================
// ObTools::Tube: server.cc
//
// Implementation of tube server
//
// Copyright (c) 2007 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-tube.h"
#include "ot-log.h"
#include "ot-misc.h"

// Default maximum send queue length
#define DEFAULT_MAX_SEND_QUEUE 1024

// Time to wait (us) if send queue full
#define SEND_BUSY_WAIT_TIME 10

namespace ObTools { namespace Tube {

//--------------------------------------------------------------------------
// Send handler thread class
// Pulls messages off the given queue and sends them to the given socket
class ServerSendThread: public MT::Thread
{
  Server& server;
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
      OBTOOLS_LOG_IF_DEBUG(log.debug << server.name
                           << " (ssend): Sending message "
                           << msg.stag() << ", length "
                           << msg.data.size()
                           << " (flags " << hex << msg.flags << dec << ")\n";)
      OBTOOLS_LOG_IF_DUMP(Misc::Dumper dumper(log.dump);
                          dumper.dump(msg.data);)

      try // Handle SocketErrors
      {
        // Write chunk header
        session.socket.write_nbo_int(msg.tag);
        session.socket.write_nbo_int(msg.data.size());
        session.socket.write_nbo_int(msg.flags); // Flags

        // Write data
        session.socket.write(msg.data);
      }
      catch (const Net::SocketError& se)
      {
        log.error << server.name << " (ssend): " << se << endl;
        break;
      }
    }

    OBTOOLS_LOG_IF_DEBUG(log.debug << server.name
                         << " (ssend): Thread shutting down\n";)
  }

public:
  ServerSendThread(Server& _server, ClientSession& _session):
    server(_server), session(_session) { start(); }
};

//--------------------------------------------------------------------------
// TCPServer verify method
bool Server::verify(Net::EndPoint ep) const
{
  // Check host against filters
  for(list<Net::MaskedAddress>::const_iterator p = filters.begin();
      p!=filters.end();
      p++)
    if (*p == ep.host) return true;

  Log::Stream error_log(Log::logger, Log::Level::error);
  error_log << name << ": Rejected connection from " << ep << endl;
  return false;
}

//--------------------------------------------------------------------------
// TCPServer process method - called in worker thread to handle connection
void Server::process(SSL::TCPSocket& socket,
                     const SSL::ClientDetails& client)
{
  Log::Streams log;  // Our private log

  const char *obit = "ended";

  log.summary << name << ": Got connection from " << client << endl;

  // Enable keepalives
  socket.enable_keepalive();

  // Also set timeout on the socket, in case the client unexpectedly disappears
  if (client_timeout) socket.set_timeout(client_timeout);

  // Create client session and map it (autoremoved on destruction)
  ClientSession session(socket, client.address, client_sessions);

  // Start send thread and detach it
  ServerSendThread send_thread(*this, session);
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

      ClientMessage msg(client, tag);

      // Verify tag
      if (tag_recognised(tag))
      {
        // Handle a TLV block
        uint32_t len   = socket.read_nbo_int();
        msg.msg.flags = socket.read_nbo_int();

        OBTOOLS_LOG_IF_DEBUG(log.debug << name << ": Received message "
                             << msg.msg.stag() << ", length "
                             << len << " (flags "
                             << hex << msg.msg.flags << dec << ")\n";)

        // Read the data
        if (!socket.read(msg.msg.data, len))
        {
          log.error << name << ": Short message read - socket died\n";
          obit = "died";
          break;
        }

        OBTOOLS_LOG_IF_DUMP(Misc::Dumper dumper(log.dump);
                            dumper.dump(msg.msg.data);)

        // Post up a message
        if (!handle_message(msg))
        {
          obit = "killed by server";
          break;
        }
      }
      else
      {
        // Unrecognised tag
        log.error << name << ": Unrecognised tag "
                  << msg.msg.stag() << " - out-of-sync?\n";
        obit = "unsynced";
        break;
      }
    }
    catch (const Net::SocketError& se)
    {
      log.error << name << ": " << se << endl;
      obit = "failed";
      break;
    }
  }

  // Tell the server the client has gone
  ClientMessage emsg(client, ClientMessage::FINISHED);
  handle_message(emsg);

  if (!!send_thread)
  {
    OBTOOLS_LOG_IF_DEBUG(log.debug << name << ": Shutting down send\n";)

    // Shut down session cleanly
    session.alive=false;

    // Kill socket
    socket.shutdown();

    // Wait for it to die
    while (!!send_thread)
    {
      session.send_q.send(Message());  // Wake up thread with bogus message
      this_thread::sleep_for(chrono::milliseconds{50});
      if (!send_thread) break;
      log.error << "Send thread won't die - waiting\n";
      this_thread::sleep_for(chrono::milliseconds{250});
    }
  }
  else
    obit = "failed (send)";

  log.summary << name << ": Connection from " << client
              << " " << obit << endl;
}


//--------------------------------------------------------------------------
// Constructor
Server::Server(int port, const string& _name, int backlog,
               int min_spare_threads, int max_threads, int _client_timeout):
  SSL::TCPServer(0, port, backlog, min_spare_threads, max_threads),
  alive(true), client_timeout(_client_timeout),
  max_send_queue(DEFAULT_MAX_SEND_QUEUE), name(_name)
{

}

//--------------------------------------------------------------------------
// Constructor with SSL
Server::Server(SSL::Context *_ctx, int port,
               const string& _name, int backlog,
               int min_spare_threads, int max_threads,
               int _client_timeout):
  SSL::TCPServer(_ctx, port, backlog, min_spare_threads, max_threads),
  alive(true), client_timeout(_client_timeout),
  max_send_queue(DEFAULT_MAX_SEND_QUEUE), name(_name)
{

}

//--------------------------------------------------------------------------
// Constructor
Server::Server(Net::EndPoint local, const string& _name, int backlog,
               int min_spare_threads, int max_threads,
               int _client_timeout):
  SSL::TCPServer(0, local, backlog, min_spare_threads, max_threads),
  alive(true), client_timeout(_client_timeout),
  max_send_queue(DEFAULT_MAX_SEND_QUEUE), name(_name)
{

}

//--------------------------------------------------------------------------
// Constructor with SSL
Server::Server(SSL::Context *_ctx, Net::EndPoint local,
               const string& _name, int backlog,
               int min_spare_threads, int max_threads,
               int _client_timeout):
  SSL::TCPServer(_ctx, local, backlog, min_spare_threads, max_threads),
  alive(true), client_timeout(_client_timeout),
  max_send_queue(DEFAULT_MAX_SEND_QUEUE), name(_name)
{

}

//--------------------------------------------------------------------------
// Send a message
// Note:  It is safe to call this inside the handle_message() method
// Whether message queued (client still connected)
bool Server::send(ClientMessage& msg)
{
  // Look up client in map, and queue it on there
  MT::RWReadLock lock(client_sessions.mutex);
  map<Net::EndPoint, ClientSession *>::iterator p
    = client_sessions.sessions.find(msg.client.address);
  if (p != client_sessions.sessions.end())
  {
    ClientSession *cs = p->second;

    while (cs->send_q.waiting() > max_send_queue) // Zero must work
      this_thread::sleep_for(chrono::milliseconds{SEND_BUSY_WAIT_TIME});

    cs->send_q.send(msg.msg);
    return true;
  }

  return false;
}

//------------------------------------------------------------------------
// Shutdown
void Server::shutdown()
{
  alive = false;
  SSL::TCPServer::shutdown();
}


}} // namespaces





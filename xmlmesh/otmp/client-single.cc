//==========================================================================
// ObTools::XMLMesh:OTMP: client-single.cc
//
// Single-threaded implementation of raw OTMP client 
// Still implements XMLMesh::OTMP::Client
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

//------------------------------------------------------------------------
// Constructors
Client::Client(Net::EndPoint _server): server(_server)
{
  // Try and get a client
  socket = 0;
  restart_socket();
}

//------------------------------------------------------------------------
// Restart a dead or non-existent socket
bool Client::restart_socket()
{
  // Delete old socket, if any
  if (socket) delete socket;

  // Try and get a new one
  socket = new Net::TCPClient(server);

  if (!*socket)
  {
    Log::Error << "OTMP: Can't open socket to " << server << endl;
    delete socket;
    socket = 0;
    return false;
  }
  else
  {
    Log::Detail << "OTMP: Opened socket to " << server << endl;
    return true;
  }
}

//------------------------------------------------------------------------
// Send a message - blocks for message to go out
// Whether successful 
bool Client::send(Message& msg)
{
  if (!socket && !restart_socket())
  {
    Log::Error << "XMLMesh can't send - no socket\n";
    return false;
  }

  if (Log::debug_ok)
    Log::Debug << "OTMP(send): Sending message length " << msg.data.size() 
	       << " (flags " << msg.flags << ")\n";
  if (Log::dump_ok) Log::Dump << msg.data << endl;

  // Write chunk header
  socket->write_nbo_int(TAG_MESSAGE);
  socket->write_nbo_int(msg.data.size());
  socket->write_nbo_int(msg.flags); 

  // Write data
  socket->write(msg.data);

  return true;  
}

//------------------------------------------------------------------------
// Receive a message - blocks waiting for one to arrive
// Returns false if the socket is restarted
bool Client::wait(Message& msg)
{
  if (!socket && !restart_socket())
  {
    Log::Error << "XMLMesh can't receive - no socket\n";
    return false;
  }

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
	restart_socket();
	return false;
      }
      
      if (Log::dump_ok) Log::Dump << content << endl;
      
      // Post up a message
      msg = Message(content, flags);
      return true;
    }
    else
    {
      // Unrecognised tag
      Log::Error << "OTMP(recv): Unrecognised tag - out-of-sync?\n";
      //Try to restart socket
      restart_socket();
      return false;
    }
  }
  catch (Net::SocketError se)
  {
    Log::Error << "OTMP(recv): " << se << endl;
    //Try to restart socket
    restart_socket();
    return false;
  }
}

//------------------------------------------------------------------------
// Destructor
Client::~Client()
{
  if (socket) delete socket;
}

}}} // namespaces





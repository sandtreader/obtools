//==========================================================================
// ObTools::XMLBus: ot-xmlbus.h
//
// Public definitions for XMLBus client library
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#ifndef __OBTOOLS_XMLBUS_H
#define __OBTOOLS_XMLBUS_H

#include <string>
#include "ot-net.h"
#include "ot-mt.h"
#include "ot-xmlbus-otmp.h"

namespace ObTools { namespace XMLBus {

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// XMLBus message
class Message
{
public:
  string id;           // Unique ID 
  string subject;      // Subject (used for subscription etc.)
  string content;      // Message content

  //------------------------------------------------------------------------
  // Constructor
  // ID is manufactured here
  Message(const string& _subject, const string& _content);
};

//==========================================================================
// XML Bus client
class Client: private OTMPClient
{
public:
  //------------------------------------------------------------------------
  // Constructors - take server address
  // port=0 means use default port for protocol
  Client(Net::IPAddress address, int port=0);

  //------------------------------------------------------------------------
  // Send a message - never blocks, but can fail if the queue is full
  // Whether message queued
  bool send(Message& msg);

  //------------------------------------------------------------------------
  // Receive a message - never blocks, returns whether one was received
  bool poll(Message& msg);

  //------------------------------------------------------------------------
  // Receive a message - blocks waiting for one to arrive
  // Returns whether one was read - will only return false if something fails
  bool wait(Message& msg);
};


//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_XMLBUS_H




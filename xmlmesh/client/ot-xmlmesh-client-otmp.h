//==========================================================================
// ObTools::XMLBus: ot-xmlbus-client-otmp.h
//
// Definition of OTMP-based XMLBus client 
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#ifndef __OBTOOLS_XMLBUS_OTMP_CLIENT_H
#define __OBTOOLS_XMLBUS_OTMP_CLIENT_H

#include "ot-xmlbus-client.h"
#include "ot-xmlbus-otmp.h"

namespace ObTools { namespace XMLBus {

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// OTMP Client Transport
class OTMPClientTransport: public ClientTransport
{
private:
  OTMP::Client otmp;

public:
  //------------------------------------------------------------------------
  // Constructor - take server address
  OTMPClientTransport(Net::EndPoint server): otmp(server) {}

  // Implementations of ClientTransport virtuals (q.v. ot-xmlbus.h)
  bool send(const string& data);
  bool poll();
  bool wait(string& data);
};

//==========================================================================
// OTMP-based XMLBus Client
class OTMPClient: public Client
{
private:
  OTMPClientTransport transport;

public:
  //------------------------------------------------------------------------
  // Constructor - take server address
  // port=0 means use default port for protocol
  OTMPClient(Net::EndPoint server): transport(server), Client(transport) {}
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_XMLBUS_OTMP_H




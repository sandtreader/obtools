//==========================================================================
// ObTools::XMLBus:Server: transport-otmp.h
//
// Definition of OTMP ServerTransport
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#ifndef __OBTOOLS_XMLBUS_SERVER_TRANS_OTMP_H
#define __OBTOOLS_XMLBUS_SERVER_TRANS_OTMP_H

#include "server.h"
#include "ot-xmlbus-otmp.h"

namespace ObTools { namespace XMLBus {

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// OTMP Server Transport 
class OTMPServerTransport: public ServerTransport
{
private:
  OTMP::Server otmp;
  MT::Queue<OTMP::ClientMessage> receive_q;

public:
  //------------------------------------------------------------------------
  // Constructor - default to standard OTMP port
  OTMPServerTransport(int port=0);

  //------------------------------------------------------------------------
  // Implementation of ServerTransport virtual interface - q.v. server.h
  bool send(const Net::EndPoint& client, const string& data);
};


//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_XMLBUS_SERVER_TRANS_OTMP_H




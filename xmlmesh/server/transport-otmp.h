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

namespace ObTools { namespace XMLBus {

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// OTMP Server Transport Factory
class OTMPServerTransportFactory: public ServerTransportFactory
{
  ServerTransport *create(XML::Element& xml);
  static OTMPServerTransportFactory instance;  // singleton

public:
  static void register_into(Server& server);
};


//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_XMLBUS_SERVER_TRANS_OTMP_H




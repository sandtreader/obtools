//==========================================================================
// ObTools::XMLMesh:Server: transport-otmp-server.h
//
// Definition of OTMP Server Transport
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
//==========================================================================

#ifndef __OBTOOLS_XMLMESH_TRANS_OTMP_SERVER_H
#define __OBTOOLS_XMLMESH_TRANS_OTMP_SERVER_H

#include "server.h"

namespace ObTools { namespace XMLMesh {

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// OTMP Server Transport Factory
class OTMPServerTransportFactory: public TransportFactory
{
  Transport *create(XML::Element& xml);
  static OTMPServerTransportFactory instance;  // singleton

public:
  static void register_into(Server& server);
};


//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_XMLMESH_TRANS_OTMP_SERVER_H




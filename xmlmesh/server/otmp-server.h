//==========================================================================
// ObTools::XMLMesh:Server: otmp-server.h
//
// Definition of OTMP Server Service
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#ifndef __OBTOOLS_XMLMESH_OTMP_SERVER_H
#define __OBTOOLS_XMLMESH_OTMP_SERVER_H

#include "server.h"

namespace ObTools { namespace XMLMesh {

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// OTMP Server Factory
class OTMPServerFactory: public ServiceFactory
{
  Service *create(Server& server, XML::Element& xml);
  static OTMPServerFactory instance;  // singleton

public:
  static void register_into(Server& server);
};


//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_XMLMESH_OTMP_SERVER_H




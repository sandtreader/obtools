//==========================================================================
// ObTools::XMLMesh:Server: publisher.h
//
// Definition of 'publisher' service 
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#ifndef __OBTOOLS_XMLMESH_PUBLISHER_H
#define __OBTOOLS_XMLMESH_PUBLISHER_H

#include "server.h"

namespace ObTools { namespace XMLMesh {

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Publisher Factory
class PublisherFactory: public ServiceFactory
{
  Service *create(Server& server, XML::Element& xml);
  static PublisherFactory instance;  // singleton

public:
  static void register_into(Server& server);
};


//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_XMLMESH_PUBLISHER_H




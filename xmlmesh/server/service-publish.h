//==========================================================================
// ObTools::XMLMesh:Server: service-publish.h
//
// Definition of 'publish' service 
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#ifndef __OBTOOLS_XMLMESH_SERVICE_PUBLISH_H
#define __OBTOOLS_XMLMESH_SERVICE_PUBLISH_H

#include "server.h"

namespace ObTools { namespace XMLMesh {

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Publish Service Factory
class PublishServiceFactory: public ServiceFactory
{
  Service *create(Server& server, XML::Element& xml);
  static PublishServiceFactory instance;  // singleton

public:
  static void register_into(Server& server);
};


//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_XMLMESH_SERVICE_PUBLISH_H




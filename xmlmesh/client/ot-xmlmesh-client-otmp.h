//==========================================================================
// ObTools::XMLMesh: ot-xmlmesh-client-otmp.h
//
// Definition of OTMP-based XMLMesh client 
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_XMLMESH_OTMP_CLIENT_H
#define __OBTOOLS_XMLMESH_OTMP_CLIENT_H

#include "ot-xmlmesh-client.h"
#include "ot-xmlmesh-otmp.h"

namespace ObTools { namespace XMLMesh {

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

  // Implementations of ClientTransport virtuals (q.v. ot-xmlmesh.h)
  bool send(const string& data);
  bool poll();
  bool wait(string& data);
};

//==========================================================================
// OTMP-based XMLMesh Client
class OTMPClient: public Client
{
private:
  OTMPClientTransport transport;

public:
  //------------------------------------------------------------------------
  // Constructor - take server address
  // port=0 means use default port for protocol
  OTMPClient(Net::EndPoint server): Client(transport), transport(server) {}
};

//==========================================================================
// OTMP-based XMLMesh MultiClient
class OTMPMultiClient: public MultiClient
{
private:
  OTMPClientTransport transport;

public:
  //------------------------------------------------------------------------
  // Constructor - take server address
  // port=0 means use default port for protocol
  // NB: MultiClient is constructed before transport whatever we say
  // - therefore leave starting it to constructor body
  OTMPMultiClient(Net::EndPoint server):
    MultiClient(transport), transport(server) { start(); }

  // Constructor specifying workers
  OTMPMultiClient(Net::EndPoint server, 
		  int min_spare_workers, int max_workers): 
    MultiClient(transport, min_spare_workers, max_workers), 
    transport(server) { start(); }
};

//==========================================================================
// Mesh message interface configured from standard config file <xmlmesh>
class OTMPMessageInterface
{
  OTMPMultiClient *client;

public:
  //--------------------------------------------------------------------------
  // Constructor, taking 'xmlmesh' element
  OTMPMessageInterface(XML::Element& config, 
		       ObTools::Message::Broker& broker);

  //--------------------------------------------------------------------------
  // Destructor - destroys message interface
  ~OTMPMessageInterface();
};


//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_XMLMESH_OTMP_H




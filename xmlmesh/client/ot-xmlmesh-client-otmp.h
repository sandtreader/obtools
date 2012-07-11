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
  void shutdown();
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

  //------------------------------------------------------------------------
  // Destructor - force shutdown early so we can shutdown MultiClient dispatch
  // thread before transport is destroyed
  ~OTMPMultiClient() { shutdown(); }
};

//==========================================================================
// Mesh message interface configured from standard config file <xmlmesh>
template<class CONTEXT>
class OTMPMessageInterface
{
  OTMPMultiClient *client;

public:
  //--------------------------------------------------------------------------
  // Constructor, taking 'xmlmesh' element
  OTMPMessageInterface(CONTEXT& context, XML::Element& config,
                       ObTools::Message::Broker<CONTEXT>& broker):
    client(0)
  {
    Log::Streams log;
    XML::XPathProcessor xpath(config);

    // Set up mesh connection - note, no default here, so if not present,
    // we disable it
    string host = xpath.get_value("server/@host");
    if (host.empty()) return;

    int port = xpath.get_value_int("server/@port", OTMP::DEFAULT_PORT);

    Net::IPAddress addr(host);
    if (!addr)
    {
      log.error << "Can't resolve XMLMesh host: " << host << endl;
      return;
    }

    Net::EndPoint ep(addr, port);
    log.summary << "Connecting to XMLMesh at " << ep << endl;

    // Start mesh client
    client = new OTMPMultiClient(ep);

    // Register our transport into server message broker
    broker.add_transport(new MessageTransport<CONTEXT>(context, *client));
  }

  //------------------------------------------------------------------------
  // MultiClient methods exposed for outgoing messages
  bool request(Message& req, Message& response)
  { return client?client->request(req, response):false; }

  bool request(Message& req)
  { return client?client->request(req):false; }

  bool send(Message& req)
  { return client?client->send(req):false; }

  //--------------------------------------------------------------------------
  // Destructor - destroys message interface
  ~OTMPMessageInterface()
  {
    if (client) delete client;
  }
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_XMLMESH_OTMP_H




//==========================================================================
// ObTools::XMLMesh:Bindings:C: xmlmesh-c.cc
//
// Basic C interface to XMLMesh client
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-xmlmesh-client-otmp.h"
#include "ot-log.h"

using namespace std;
using namespace ObTools;

extern "C" {

//--------------------------------------------------------------------------
// Globals
// Note:  This isn't remotely thread-safe - use CC if you want that
static XMLMesh::OTMPClient *client = 0;

//--------------------------------------------------------------------------
// Initialise the client - connect to the given host & port
// Whether successful (1=OK)
int ot_xmlmesh_init(const char *host, int port)
{
  // Set up logging
  // Use statics to keep them outside this routine, but no-one else
  // needs to access them
  static Log::StreamChannel   chan_out(cout);
  static Log::TimestampFilter tsfilter("%H:%M:%S %a %d %b %Y: ", chan_out);
  static Log::LevelFilter     level_out(Log::LEVEL_SUMMARY, tsfilter);
  Log::logger.connect(level_out);

  // Resolve name
  Net::IPAddress addr(host);
  if (!addr)
  {
    Log::Error << "Can't resolve host: " << host << endl;
    return 0;
  }

  // Start client
  Net::EndPoint server(addr, port);
  client = new XMLMesh::OTMPClient(server);
  return 1;
}

//--------------------------------------------------------------------------
// Send a message with no response expected
// Returns whether successful (1 = OK)
int ot_xmlmesh_send(const char *subject, const char *xml)
{
  if (!client)
  {
    cerr << "XMLMesh C library called but not initialised!\n";
    return 0;
  }

  XMLMesh::Message msg(subject, xml);
  return client->send(msg)?1:0;
}

//--------------------------------------------------------------------------
// Send a request and get response
// Returns whether successful (1 = OK)
// If result_p is not NULL, sets it to malloc'ed result string if successful
// Caller should free(*result_p) when done
// If result_p is NULL, simply checks for OK or error, and fails on error
int ot_xmlmesh_request(const char *subject, const char *xml, char **result_p)
{
  if (!client)
  {
    cerr << "XMLMesh C library called but not initialised!\n";
    return 0;
  }

  XMLMesh::Message request(subject, xml, true);

  // Check if they want a response
  if (result_p)
  {
    XMLMesh::Message response;

    if (client->request(request, response))
    {
      // Unpack response
      string rs = response.get_text();
      *result_p = strdup(rs.c_str());
      return 1;
    }
  }
  else
  {
    // No response needed, handle errors internally
    if (client->request(request)) return 1;
  }
  
  return 0;
}

//--------------------------------------------------------------------------
// Shutdown
void ot_xmlmesh_shutdown()
{
  if (client)
  {
    delete client;
    client = 0;
  }
}

// extern "C"
}

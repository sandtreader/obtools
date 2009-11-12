//==========================================================================
// ObTools::XMLMesh:Bindings:C: xmlmesh-c.cc
//
// Basic C interface to XMLMesh client
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-xmlmesh-client-otmp.h"
#include "ot-log.h"

using namespace std;
using namespace ObTools;

extern "C" {

#include "ot-xmlmesh-c.h"

//--------------------------------------------------------------------------
// Initialise the client library
void ot_xmlmesh_init()
{
#if defined DEBUG
  // Set up logging
  // Use statics to keep them outside this routine, but no-one else
  // needs to access them
  static Log::StreamChannel   chan_out(cout);
  static Log::TimestampFilter tsfilter("%H:%M:%S %a %d %b %Y: ", chan_out);
  static Log::LevelFilter     level_out(Log::LEVEL_SUMMARY, tsfilter);
  Log::logger.connect(level_out);
#endif
}

//--------------------------------------------------------------------------
// Open an XMLMesh connection
OT_XMLMesh_Conn ot_xmlmesh_open(const char *host, int port)
{
  // Resolve name
  Net::IPAddress addr(host);
  if (!addr)
  {
    cerr << "Can't resolve host: " << host << endl;
    return 0;
  }

  // Start client
  Net::EndPoint server(addr, port);
  XMLMesh::OTMPClient *client = new XMLMesh::OTMPClient(server);
  return (OT_XMLMesh_Conn)client;
}

//--------------------------------------------------------------------------
// Send a message with no response expected
// Returns whether successful (1 = OK)
int ot_xmlmesh_send(OT_XMLMesh_Conn conn, const char *subject, const char *xml)
{
  XMLMesh::OTMPClient *client = (XMLMesh::OTMPClient *)conn;
  if (!client)
  {
    cerr << "Bogus XMLMesh connection pointer!\n";
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
int ot_xmlmesh_request(OT_XMLMesh_Conn conn,
		       const char *subject, const char *xml, char **result_p)
{
  XMLMesh::OTMPClient *client = (XMLMesh::OTMPClient *)conn;
  if (!client)
  {
    cerr << "Bogus XMLMesh connection pointer!\n";
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
// Close a connection
void ot_xmlmesh_close(OT_XMLMesh_Conn conn)
{
  XMLMesh::OTMPClient *client = (XMLMesh::OTMPClient *)conn;
  if (!client)
  {
    cerr << "Bogus XMLMesh connection pointer!\n";
  }
  else
  {
    delete client;
  }
}

//--------------------------------------------------------------------------
// Shutdown
void ot_xmlmesh_shutdown()
{
}

// extern "C"
}

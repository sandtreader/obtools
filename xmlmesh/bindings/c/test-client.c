//==========================================================================
// ObTools::XMLMesh: test-client.c
//
// Test harness for XMLMesh C library
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-xmlmesh-c.h"
#include <stdio.h>
#include <stdlib.h>

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  char *res;
  char *host;
  int port = 29167;
  OT_XMLMesh_Conn conn;

  if (argc < 2)
  {
    fprintf(stderr, "Give a hostname and optional port\n");
    return 2;
  }

  host = argv[1];
  if (argc > 2) port = atoi(argv[2]);

  // Initialise library
  ot_xmlmesh_init();
  atexit(ot_xmlmesh_shutdown);

  // Create connection
  conn = ot_xmlmesh_open(host, port);
  if (!conn)
  {
    fprintf(stderr, "XMLMesh won't initialise\n");
    return 4;
  }

  // Send a simple message
  if (ot_xmlmesh_send(conn, "Test", "<foo/>"))
  {
    printf("Simple message sent OK\n");
  }
  else
  {
    fprintf(stderr, "Can't send message\n");
    ot_xmlmesh_close(conn);
    return 2;
  }

  // Send a simple request
  // Happens to be a subscription request which the server will respond to
  if (ot_xmlmesh_request(conn,
			 "xmlmesh.subscription.join", 
			 "<xmlmesh:join subject='foo'/>",
			 &res))
  {
    printf("Subscription request sent OK, returned:\n%s\n", res);
    free(res);
  }
  else
  {
    fprintf(stderr, "Subscription request failed\n");
    ot_xmlmesh_close(conn);
    return 2;
  }

  ot_xmlmesh_close(conn);
  return 0;  
}





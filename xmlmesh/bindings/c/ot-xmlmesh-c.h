//==========================================================================
// ObTools::XMLMesh:Bindings:C: ot-xmlmesh-c.h
//
// Basic C interface to XMLMesh client
// To be included from a C file, compiled with cc
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#if !defined(__OBTOOLS_XMLMESH_C_H)
#define __OBTOOLS_XMLMESH_C_H

//--------------------------------------------------------------------------
// Client connection pointer
typedef void *OT_XMLMesh_Conn;

//--------------------------------------------------------------------------
// Initialise the client library
extern void ot_xmlmesh_init();

//--------------------------------------------------------------------------
// Open an XMLMesh connection
extern OT_XMLMesh_Conn ot_xmlmesh_open(const char *host, int port);

//--------------------------------------------------------------------------
// Send a message with no response expected
// Returns whether successful (1 = OK)
extern int ot_xmlmesh_send(OT_XMLMesh_Conn conn, 
			   const char *subject, const char *xml);

//--------------------------------------------------------------------------
// Send a request and get response
// Returns whether successful (1 = OK)
// If result_p is not NULL, sets it to malloc'ed result string if successful
// Caller should free(*result_p) when done
// If result_p is NULL, simply checks for OK or error, and fails on error
extern int ot_xmlmesh_request(OT_XMLMesh_Conn conn,
			      const char *subject, const char *xml, 
			      char **result_p);

//--------------------------------------------------------------------------
// Close a connection
extern void ot_xmlmesh_close(OT_XMLMesh_Conn conn);

//--------------------------------------------------------------------------
// Shutdown
extern void ot_xmlmesh_shutdown(void);

//==========================================================================
#endif // !__OBTOOLS_XMLMESH_C_H


//==========================================================================
// ObTools::XMLMesh:Bindings:C: ot-xmlmesh-c.h
//
// Basic C interface to XMLMesh client
// To be included from a C file, compiled with cc
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#if !defined(__OBTOOLS_XMLMESH_C_H)
#define __OBTOOLS_XMLMESH_C_H

//--------------------------------------------------------------------------
// Initialise the client - connect to the given host & port
// Whether successful (1=OK)
extern int ot_xmlmesh_init(const char *host, int port);

//--------------------------------------------------------------------------
// Send a message with no response expected
// Returns whether successful (1 = OK)
int ot_xmlmesh_send(const char *subject, const char *xml);

//--------------------------------------------------------------------------
// Send a request and get response
// Returns whether successful (1 = OK)
// If result_p is not NULL, sets it to malloc'ed result string if successful
// Caller should free(*result_p) when done
// If result_p is NULL, simply checks for OK or error, and fails on error
extern int ot_xmlmesh_request(const char *subject, const char *xml, 
			      char **result_p);

//--------------------------------------------------------------------------
// Shutdown
extern void ot_xmlmesh_shutdown(void);

//==========================================================================
#endif // !__OBTOOLS_XMLMESH_C_H


/* =======================================================================
// ObTools::XMLMesh: php_xmlmesh.c
//
// PHP XMLMesh client module
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
// ======================================================================= */

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ot-xmlmesh-c.h"

/* ======================================================================= */
/* Module globals and INI items                                            */

ZEND_BEGIN_MODULE_GLOBALS(xmlmesh)
	 char *mesh_host;
	 int mesh_port;
	 OT_XMLMesh_Conn conn;  // !!! Should be per process!
ZEND_END_MODULE_GLOBALS(xmlmesh)

ZEND_DECLARE_MODULE_GLOBALS(xmlmesh)

#ifdef ZTS
#include "TSRM.h"
#define XMLMESH_G(v) TSRMG(xmlmesh_globals_id, zend_xmlmesh_globals *, v)
#else
#define XMLMESH_G(v) (xmlmesh_globals.v)
#endif

/* ----------------------------------------------------------------------- */
/* Definition of INI-file items                                            */
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("xmlmesh.mesh_port", "29167", PHP_INI_ALL, 
		      OnUpdateInt, mesh_port, 
		      zend_xmlmesh_globals, xmlmesh_globals)
    STD_PHP_INI_ENTRY("xmlmesh.mesh_host", "localhost", PHP_INI_ALL, 
		      OnUpdateString, mesh_host, 
		      zend_xmlmesh_globals, xmlmesh_globals)
PHP_INI_END()

/* ======================================================================= */
/* Public functions                                                        */

ZEND_FUNCTION(xmlmesh_send);
ZEND_FUNCTION(xmlmesh_request);
ZEND_FUNCTION(xmlmesh_simple_request);

zend_function_entry xmlmesh_functions[] = 
{
  ZEND_FE(xmlmesh_send, NULL)
  ZEND_FE(xmlmesh_request, NULL)
  ZEND_FE(xmlmesh_simple_request, NULL)
  {NULL, NULL, NULL}	
};

/* ======================================================================= */
/* Module definition                                                       */

ZEND_MINIT_FUNCTION(xmlmesh);
ZEND_MSHUTDOWN_FUNCTION(xmlmesh);
ZEND_RINIT_FUNCTION(xmlmesh);
ZEND_RSHUTDOWN_FUNCTION(xmlmesh);
ZEND_MINFO_FUNCTION(xmlmesh);

zend_module_entry xmlmesh_module_entry = 
{
  STANDARD_MODULE_HEADER,
  "XMLMesh Client",
  xmlmesh_functions,
  ZEND_MINIT(xmlmesh),
  ZEND_MSHUTDOWN(xmlmesh),
  ZEND_RINIT(xmlmesh),
  ZEND_RSHUTDOWN(xmlmesh),
  ZEND_MINFO(xmlmesh),
  "0.1",
  STANDARD_MODULE_PROPERTIES
};

ZEND_GET_MODULE(xmlmesh)

/* ======================================================================= */
/* Module-level functions                                                  */

/* ----------------------------------------------------------------------- */
/* Global initialiser                                                      */
static void php_xmlmesh_init_globals(zend_xmlmesh_globals *xmlmesh_globals)
{
  xmlmesh_globals->mesh_port = 0;
  xmlmesh_globals->mesh_host = NULL;
  xmlmesh_globals->conn = NULL;
}

/* ----------------------------------------------------------------------- */
/* Module init function                                                    */
ZEND_MINIT_FUNCTION(xmlmesh)
{
  ZEND_INIT_MODULE_GLOBALS(xmlmesh, php_xmlmesh_init_globals, NULL);
  REGISTER_INI_ENTRIES();
  XMLMESH_G(conn) = NULL;

  ot_xmlmesh_init();
  return SUCCESS;
}

/* ----------------------------------------------------------------------- */
/* Module shutdown function                                                */
ZEND_MSHUTDOWN_FUNCTION(xmlmesh)
{
  UNREGISTER_INI_ENTRIES();
  if (XMLMESH_G(conn))
  {
    ot_xmlmesh_close(XMLMESH_G(conn));
    XMLMESH_G(conn) = NULL;
  }
  ot_xmlmesh_shutdown();
  return SUCCESS;
}

/* ----------------------------------------------------------------------- */
/* Request init function                                                   */
ZEND_RINIT_FUNCTION(xmlmesh)
{
  // Create connection if not already made
  // This should happen in the post-fork instance
  if (!XMLMESH_G(conn))
    XMLMESH_G(conn) = ot_xmlmesh_open(XMLMESH_G(mesh_host), 
				      XMLMESH_G(mesh_port));

  return XMLMESH_G(conn)?SUCCESS:FAILURE;
}

/* ----------------------------------------------------------------------- */
/* Request shutdown function                                               */
ZEND_RSHUTDOWN_FUNCTION(xmlmesh)
{
  return SUCCESS;
}

/* ----------------------------------------------------------------------- */
/* Information function for phpinfo()                                      */
ZEND_MINFO_FUNCTION(xmlmesh)
{
  php_info_print_table_start();
  php_info_print_table_header(2, "xmlmesh support", "enabled");
  php_info_print_table_end();

  DISPLAY_INI_ENTRIES();
}

/* ======================================================================= */
/* Public function implementations                                         */

/* ----------------------------------------------------------------------- */
/* Send a one-way XML message with no response                             */
/* bool xmlmesh_send(string subject, string xml);                          */
ZEND_FUNCTION(xmlmesh_send)
{
  char *subject;
  int subject_len;
  char *xml;
  int xml_len;

  if (!XMLMESH_G(conn))
  {
    zend_printf("XMLMesh send failed - no connection\n");
    RETURN_FALSE;
  }

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",
			    &subject, &subject_len,
			    &xml, &xml_len) == FAILURE) return;

  if (ot_xmlmesh_send(XMLMESH_G(conn), subject, xml))
  {
    RETURN_TRUE;
  }
  else
  {
    RETURN_FALSE;
  }
}

/* ----------------------------------------------------------------------- */
/* Send an XML Request and return XML response                             */
/* string xmlmesh_request(string subject, string xml);                     */
/* Returns empty string if request failed                                  */
ZEND_FUNCTION(xmlmesh_request)
{
  char *subject;
  int subject_len;
  char *xml;
  int xml_len;
  char *response;

  if (!XMLMESH_G(conn))
  {
    zend_printf("XMLMesh request failed - no connection\n");
    RETURN_EMPTY_STRING();
  }

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",
			    &subject, &subject_len,
			    &xml, &xml_len) == FAILURE) return;

  if (ot_xmlmesh_request(XMLMESH_G(conn), subject, xml, &response))
  {
    // Copy it to estrdup and free it - inefficient!
    RETVAL_STRING(response, 1);
    free(response);
    return;
  }
  else
  {
    RETURN_EMPTY_STRING();
  }
}

/* ----------------------------------------------------------------------- */
/* Send an XML Request and check for OK or error                           */
/* bool xmlmesh_simple_request(string subject, string xml);                */
ZEND_FUNCTION(xmlmesh_simple_request)
{
  char *subject;
  int subject_len;
  char *xml;
  int xml_len;

  if (!XMLMESH_G(conn))
  {
    zend_printf("XMLMesh request failed - no connection\n");
    RETURN_FALSE;
  }

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",
			    &subject, &subject_len,
			    &xml, &xml_len) == FAILURE) return;

  if (ot_xmlmesh_request(XMLMESH_G(conn), subject, xml, NULL))
  {
    RETURN_TRUE;
  }
  else
  {
    RETURN_FALSE;
  }
}








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

ZEND_FUNCTION(xmlmesh_request);

zend_function_entry xmlmesh_functions[] = 
{
  ZEND_FE(xmlmesh_request, NULL)
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
}

/* ----------------------------------------------------------------------- */
/* Module init function                                                    */
ZEND_MINIT_FUNCTION(xmlmesh)
{
  ZEND_INIT_MODULE_GLOBALS(xmlmesh, php_xmlmesh_init_globals, NULL);
  REGISTER_INI_ENTRIES();

  if (ot_xmlmesh_init(XMLMESH_G(mesh_host), XMLMESH_G(mesh_port)))
    return SUCCESS;
  else
    return FAILURE;
}

/* ----------------------------------------------------------------------- */
/* Module shutdown function                                                */
ZEND_MSHUTDOWN_FUNCTION(xmlmesh)
{
  UNREGISTER_INI_ENTRIES();
  ot_xmlmesh_shutdown();
  return SUCCESS;
}

/* ----------------------------------------------------------------------- */
/* Request init function                                                   */
ZEND_RINIT_FUNCTION(xmlmesh)
{
  return SUCCESS;
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
/* Send an XML Request and return XML response                             */
/* bool xmlmesh_request(string subject, string xml);                       */
ZEND_FUNCTION(xmlmesh_request)
{
  char *subject;
  int subject_len;
  char *xml;
  int xml_len;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",
			    &subject, &subject_len,
			    &xml, &xml_len) == FAILURE) return;

  zend_printf("XMLMesh-Request to %s:%d: '%s'\n",
	      XMLMESH_G(mesh_host),
	      XMLMESH_G(mesh_port),
	      subject);

  if (ot_xmlmesh_send(subject, xml))
  {
    RETURN_TRUE;
  }
  else
  {
    RETURN_FALSE;
  }
}









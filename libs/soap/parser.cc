//==========================================================================
// ObTools::SOAP: parser.cc
//
// Implementation of SOAP parser
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-soap.h"

namespace ObTools { namespace SOAP {

//------------------------------------------------------------------------
// Constructor - use given stream for errors
Parser::Parser(ostream& s): XML::Parser(s)
{ 
  fix_namespace(NS_ENVELOPE, "env"); 
}

//------------------------------------------------------------------------
// Verify the document is valid SOAP
bool Parser::verify()
{
  XML::Element& root = get_root();
  if (root.name != "env:Envelope")
  {
    serr << "Incorrect SOAP envelope: " << root.name << endl;
    return false;
  }

  // Make sure it has a body 
  if (!root.get_child("env:Body"))
  {
    serr << "SOAP message has no Body\n";
    return false;
  }

  return true;
}

}} // namespaces

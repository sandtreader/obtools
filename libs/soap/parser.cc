//==========================================================================
// ObTools::SOAP: parser.cc
//
// Implementation of SOAP parser
//
// Copyright (c) 2003-2011 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-soap.h"

namespace ObTools { namespace SOAP {

//--------------------------------------------------------------------------
// Constructor - use given stream for errors
Parser::Parser(ostream& s): XML::Parser(s)
{
  fix_namespace(NS_ENVELOPE_1_1, "env");
  fix_namespace(NS_ENVELOPE_1_2, "env");
  fix_namespace(NS_ENVELOPE_1_3, "env");
}

//--------------------------------------------------------------------------
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

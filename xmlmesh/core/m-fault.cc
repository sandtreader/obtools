//==========================================================================
// ObTools::XMLMesh:Core: m-fault.cc
//
// Support for xmlmesh.fault (SOAP Fault) messages
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-xmlmesh.h"
#include "ot-log.h"
#include <sstream>

namespace ObTools { namespace XMLMesh {

//--------------------------------------------------------------------------
// Constructor for responses
FaultMessage::FaultMessage(const string& ref, SOAP::Fault::Code _code,
			   const string& _reason):
  Message("xmlmesh.fault", new SOAP::Fault(code, reason), false, ref),
  code(_code),
  reason(_reason)
{
}

//--------------------------------------------------------------------------
// Down-cast constructor from general message on receipt
FaultMessage::FaultMessage(Message& msg): 
  Message()
{
  string text = msg.get_text();

  SOAP::Parser parser(Log::Error);
  SOAP::Fault *fault = new SOAP::Fault(text, parser);
  soap_message = fault;

  code = fault->get_code();
  reason = fault->get_reason();
}

//--------------------------------------------------------------------------
// Get raw SOAP Fault message for unpacking
SOAP::Fault& FaultMessage::get_fault() const
{
  return *dynamic_cast<SOAP::Fault *>(soap_message);
}

//------------------------------------------------------------------------
// << operator to write FaultMessage to ostream
ostream& operator<<(ostream& s, const FaultMessage& m)
{
  SOAP::Fault& fault = m.get_fault();

  s << "SOAP Fault: " << fault.get_reason() 
    << " (" << fault.get_code_string() << ")\n";

  return s;
}

}} // namespaces






//==========================================================================
// ObTools::XMLMesh:Core: m-fault.cc
//
// Support for xmlmesh.fault (SOAP Fault) messages
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-xmlmesh.h"
#include "ot-log.h"
#include <sstream>

namespace ObTools { namespace XMLMesh {

//--------------------------------------------------------------------------
// Constructor for responses
FaultMessage::FaultMessage(const string& ref, SOAP::Fault::Code _code,
			   const string& _reason):
  Message("xmlmesh.fault", new SOAP::Fault(_code, _reason), false, ref),
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

  Log::Stream error_log(Log::logger, Log::LEVEL_ERROR);
  SOAP::Parser parser(error_log);
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
    << " (" << fault.get_code_string() << ")";

  return s;
}

}} // namespaces






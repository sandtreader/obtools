//==========================================================================
// ObTools::XMLMesh:Core: m-error.cc
//
// Support for xmlmesh.error messages
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-xmlmesh.h"
#include "ot-log.h"
#include <sstream>

namespace ObTools { namespace XMLMesh {

//--------------------------------------------------------------------------
// Constructor for responses
ErrorMessage::ErrorMessage(const string& ref, Severity _severity, 
			   const string& _text):
  Message(), severity(_severity), text(_text)
{
  string sev;

  switch (severity)
  {
    case WARNING: sev = "warning"; break;
    case ERROR:   sev = "error";   break;
    case SERIOUS: sev = "serious"; break;
    case FATAL:   sev = "fatal";   break;
  }

  ostringstream mss;
  mss << "  <xmlmesh:error severity=\"" << sev << "\">\n";
  mss << "    " << text << endl;
  mss << "  </xmlmesh:error>";
  string content = mss.str();
 
  // Create subject
  string subject = "xmlmesh.error.";
  subject += sev;

  // Create message from this
  create(subject, content, false, ref);
}

//--------------------------------------------------------------------------
// Down-cast constructor from general message on receipt
ErrorMessage::ErrorMessage(Message& msg): 
  Message(msg.get_text())  // Copy text
{
  // Get the XML
  const XML::Element& xml = get_xml();

  // Get xmlmesh.error element
  const XML::Element& xerr = xml.get_child("xmlmesh:error");

  if (xerr.valid())
  {
    string sev  = xerr.get_attr("severity", "UNKNOWN");
    
    // Translate severity
    if (sev == "warning")
      severity = WARNING;
    else if (sev == "error")
      severity = ERROR;
    else if (sev == "serious")
      severity = SERIOUS;
    else if (sev == "fatal")
      severity = FATAL;
    else 
    {
      // Probably bogus XML
      Log::Error << "Unknown severity in error message:\n";
      Log::Error << get_text() << "\n";
      severity = BOGUS;  
    }

    // Get text
    text = xerr.get_content(); 
  }
  else
  {
    Log::Error << "Error is not an xmlmesh:error:\n";
    Log::Error << get_text() << "\n";
    severity = BOGUS;
    text = "Unparseable error";
  }
}

//------------------------------------------------------------------------
// << operator to write ErrorMessage to ostream
ostream& operator<<(ostream& s, const ErrorMessage& m)
{
  switch (m.severity)
  {
    case ErrorMessage::WARNING: s<<"Warning: "; break;
    case ErrorMessage::ERROR:   s<<"Error: "; break;
    case ErrorMessage::SERIOUS: s<<"SERIOUS ERROR: "; break;
    case ErrorMessage::FATAL:   s<<"FATAL ERROR: "; break;
    case ErrorMessage::BOGUS:   s<<"Problem in error handling: "; break;
  }

  s << m.text;
  return s;
}

}} // namespaces






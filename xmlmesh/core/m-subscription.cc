//==========================================================================
// ObTools::XMLMesh:Core: m-subscription.cc
//
// Support for xmlmesh.subscription messages
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-xmlmesh.h"
#include "ot-log.h"
#include <sstream>

namespace ObTools { namespace XMLMesh {

//--------------------------------------------------------------------------
// Constructor for requests
SubscriptionMessage::SubscriptionMessage(Operation _operation, 
					 const string& _subject):
  Message(), operation(_operation), subject(_subject)
{
  string op;

  switch (operation)
  {
    case JOIN:  op = "join";  break;
    case LEAVE: op = "leave"; break;
  }

  ostringstream mss;
  mss << "  <xmlmesh:" << op << " subject=\"" << subject << "\"/>";
  string content = mss.str();
 
  // Create subject
  string subject = "xmlmesh.subscription.";
  subject += op;

  // Create message from this
  create(subject, content, true);
}

//--------------------------------------------------------------------------
// Down-cast constructor from general message on receipt
SubscriptionMessage::SubscriptionMessage(Message& msg): 
  Message(msg.get_text())  // Copy text
{
  // Get the XML
  XML::Element& xml = get_xml();

  // Get xmlmesh.join/leave elements
  XML::Element& xsub = xml.get_child();

  if (xsub.name == "xmlmesh:join")
    operation = JOIN;
  else if (xsub.name == "xmlmesh:leave")
    operation = LEAVE;
  else
  {
    // Probably bogus XML
    Log::Error << "Unknown XML in subscription message:\n";
    Log::Error << get_text() << "\n";
    operation = BOGUS;  
  }

  // Get subject
  subject = xsub.get_attr("subject", "*"); 
}

//------------------------------------------------------------------------
// << operator to write SubscriptionMessage to ostream
ostream& operator<<(ostream& s, const SubscriptionMessage& m)
{
  switch (m.operation)
  {
    case SubscriptionMessage::JOIN:  s<<"Join ("; break;
    case SubscriptionMessage::LEAVE: s<<"Leave ("; break;
    case SubscriptionMessage::BOGUS: 
      s<<"Problem in subscription handling ("; break;
  }

  s << m.subject << ")";
}

}} // namespaces






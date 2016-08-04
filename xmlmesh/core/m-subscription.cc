//==========================================================================
// ObTools::XMLMesh:Core: m-subscription.cc
//
// Support for xmlmesh.subscription messages
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-xmlmesh.h"
#include "ot-log.h"
#include <sstream>

namespace ObTools { namespace XMLMesh {

//--------------------------------------------------------------------------
// Constructor for requests
SubscriptionMessage::SubscriptionMessage(Operation _operation,
                                         const string& _subject):
  Message((_operation==JOIN)?"xmlmesh.subscription.join"
                            :"xmlmesh.subscription.leave",
          new XML::Element((_operation==JOIN)?"x:join":"x:leave"),
          true),
  operation(_operation),
  subject(_subject)
{
  soap_message->get_body().set_attr("subject", _subject);
}

//--------------------------------------------------------------------------
// Down-cast constructor from general message on receipt
SubscriptionMessage::SubscriptionMessage(Message& msg):
  Message(msg.get_text())  // Copy text
{
  // Get the body
  const XML::Element& body = get_body();

  if (body.name == "x:join")
    operation = JOIN;
  else if (body.name == "x:leave")
    operation = LEAVE;
  else
  {
    // Probably bogus XML
    Log::Stream error_log(Log::logger, Log::LEVEL_ERROR);
    error_log << "Unknown XML in subscription message:\n";
    error_log << get_text() << "\n";
    operation = BOGUS;
  }

  // Get subject
  subject = body.get_attr("subject", "*");
}

//--------------------------------------------------------------------------
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
  return s;
}

}} // namespaces






//==========================================================================
// ObTools::XMLBus:Server distributor.cc
//
// Implementation of XMLBus message distributor
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "server.h"
#include "ot-log.h"
#include "ot-text.h"

namespace ObTools { namespace XMLBus { 

//------------------------------------------------------------------------
// Attach a new message handler on the given subject pattern
// Handlers will be deleted on destruction
void Distributor::attach_handler(const string& subject, MessageHandler& h)
{
  HandlerRegistration reg(subject, h);
  handlers.push_back(reg);
}

//------------------------------------------------------------------------
// Distribute a message
void Distributor::distribute(IncomingMessage& msg)
{
  // Get subject
  XML::Element& xml = msg.message.get_xml();
  string subject = xml.get_attr("subject");

  // Loop over all handlers to see if they want it
  for(list<HandlerRegistration>::iterator p=handlers.begin();
      p!=handlers.end();
      p++)
  {
    HandlerRegistration& reg = *p;

    // Check for subject match - CASE INSENSITIVE
    if (Text::pattern_match(reg.subject_pattern, subject, false))
    {
      // Call the handler - if it wants us to drop the message, stop
      if (!reg.handler.handle(msg)) break;
    }
  }
}

}} // namespaces





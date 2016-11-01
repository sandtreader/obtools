//==========================================================================
// ObTools::XMLMesh::Listener: action.cc
//
// Action handler
//
// Copyright (c) 2016 Paul Clark.  All rights reserved
//==========================================================================

#include "listener.h"
#include "ot-log.h"

namespace ObTools { namespace XMLMesh {

//------------------------------------------------------------------------
// Subscribe to the given subject
void Action::subscribe(XMLMesh::MultiClient& mesh, const string& subject)
{
  subscriber = new Subscriber(mesh, subject, *this);
}

//------------------------------------------------------------------------
// Unsubscribe
void Action::unsubscribe()
{
  if (subscriber) subscriber->disconnect();
  subscriber = 0;
}

//------------------------------------------------------------------------
// Handle a message
void Action::handle(const XMLMesh::Message& msg)
{
  Log::Summary log;
  log << "Received XMLMesh message " << msg.get_subject() << endl;
}


}} // namespaces

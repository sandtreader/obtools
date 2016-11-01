//==========================================================================
// ObTools::XMLMesh::Listener: action.cc
//
// Action handler
//
// Copyright (c) 2016 Paul Clark.  All rights reserved
//==========================================================================

#include "listener.h"
#include "ot-log.h"
#include "ot-exec.h"

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
  Log::Streams log;
  string subject = msg.get_subject();
  log.detail << "Received XMLMesh message " << subject << endl;

  // Substitute the subject in the command
  string expanded_command = Text::subst(command, "$SUBJECT", subject);

  // Run the command
  Exec::Command cmd(expanded_command);
  string output;
  if (!cmd.execute(msg.get_body().to_string(), output))
  {
    log.error << "Failed to run command '" << command << "' for message "
              << subject << endl;

    if (msg.get_rsvp())
      subscriber->get_mesh().respond(SOAP::Fault::CODE_RECEIVER,
                                     "Command failed", msg);
    return;
  }

  if (msg.get_rsvp())
  {
    if (output.empty())
    {
      // Just send OK
      subscriber->get_mesh().respond(msg);
    }
    else
    {
      // Construct response subject by replacing .request (if any)
      // with .response, or just appending it
      string response_subject = Text::subst(subject, ".request", "");
      response_subject += ".response";
      XMLMesh::Message response(response_subject, output, false, msg.get_id());
      subscriber->get_mesh().send(response);
    }
  }
}


}} // namespaces

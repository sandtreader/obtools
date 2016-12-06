//==========================================================================
// ObTools::XMLMesh::Listener: server.cc
//
// Global singleton server object for XMLMesh listener
//
// Copyright (c) 2016 Paul Clark.  All rights reserved
//==========================================================================

#include "listener.h"
#include "ot-log.h"
#include "ot-xmlmesh-client-otmp.h"

namespace ObTools { namespace XMLMesh {

namespace
{
  const auto default_actions_dir = "/etc/obtools/actions/";
}

//--------------------------------------------------------------------------
// Constructor
Server::Server():
  mesh(0)
{
}

//--------------------------------------------------------------------------
// Read settings from configuration
void Server::read_config(const XML::Configuration& config)
{
  // Copy config for later service creation
  config_xml = config.get_root();
}

//--------------------------------------------------------------------------
// Prerun function - run with original user (usually root) privileges
int Server::run_priv()
{
  return 0;
}

//--------------------------------------------------------------------------
// Preconfigure from config file, before daemon() called
int Server::preconfigure()
{
  XML::XPathProcessor config(config_xml);

  return 0;
}

//--------------------------------------------------------------------------
// Pre-run
int Server::pre_run()
{
  Log::Streams log;

  // Configure permanent and transient state
  if (!configure())
  {
    log.error << "Cannot configure server" << endl;
    return 2;
  }

  reconfigure();

  return 0;
}

//--------------------------------------------------------------------------
// Global configuration - called only at startup
// Returns whether successful
bool Server::configure()
{
  Log::Streams log;
  log.summary << "Configuring server permanent state" << endl;

  XML::XPathProcessor config(config_xml);

  // Initialise mesh client
  string host = config.get_value("xmlmesh/@host", "localhost");
  int port = config.get_value_int("xmlmesh/@port",
                                  XMLMesh::OTMP::DEFAULT_PORT);
  Net::EndPoint server(host, port);
  mesh = new XMLMesh::OTMPMultiClient(server);
  return true;
}

//--------------------------------------------------------------------------
// Global re-configuration - called at startup and also at SIGHUP
void Server::reconfigure()
{
  Log::Streams log;
  log.summary << "Configuring server dynamic state" << endl;

  XML::XPathProcessor config(config_xml);

  // Keep a snapshot of the actions in place, to spot changes
  MT::RWWriteLock lock(actions_mutex);
  auto old_actions = actions;

  // Get and inspect the actions directory
  File::Directory actions_dir(config.get_value("actions/@dir",
                                               default_actions_dir));
  if (old_actions.size())
    log.detail << "Updating actions from " << actions_dir << ":\n";
  else
    log.summary << "Reading actions from " << actions_dir << ":\n";
  list<File::Path> paths;
  actions_dir.inspect(paths);
  for(const auto path: paths)
  {
    if (old_actions.size())
      log.detail << " - " << path << endl;
    else
      log.summary << " - " << path << endl;

    // Read it as XML
    XML::Configuration action_config(path.str(), log.error);
    if (action_config.read("action"))
    {
      const auto subject = action_config["subject"];
      const auto command = action_config["command"];
      Action action(command);

      // Does it already exist and is the same?
      auto old_it = old_actions.find(subject);
      if (old_it != old_actions.end())
      {
        if (old_it->second == action)
        {
          log.detail << "   - unchanged action on subject '"
                     << subject << "'\n";
        }
        else
        {
          log.summary << "   - updated action on subject '"
                     << subject << "'\n";
          actions[subject].update(action);
        }

        // Remove so we don't delete
        old_actions.erase(old_it);
      }
      else
      {
        log.summary << "   - new action on subject '" << subject << "'\n";
        actions[subject] = action;
        if (mesh) actions[subject].subscribe(*mesh, subject);
      }
    }
    else log.error << "Can't read action file " << path << "\n";
  }

  // Remove any remaining dead actions
  for(const auto it: old_actions)
  {
    log.summary << "Removing dead action on subject '" << it.first << "'\n";
    auto ait = actions.find(it.first);
    if (ait != actions.end())
    {
      ait->second.unsubscribe();
      actions.erase(ait);
    }
  }
}

//--------------------------------------------------------------------------
// Main loop tick
int Server::tick()
{
  return 0;
}

//--------------------------------------------------------------------------
// Clean up
void Server::cleanup()
{
  Log::Streams log;

  log.summary << "Shutting down...\n";

  // Unsubscribe any remaining actions
  MT::RWWriteLock lock(actions_mutex);
  for(auto& it: actions) it.second.unsubscribe();

  // Shut down mesh
  if (mesh) delete mesh;
  mesh = 0;

  log.summary << "Shutdown complete\n";
}

}} // namespaces

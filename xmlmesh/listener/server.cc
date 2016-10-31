//==========================================================================
// ObTools::XMLMesh::Listener: main.cc
//
// Global singleton server object for XMLMesh listener
//
// Copyright (c) 2016 Paul Clark.  All rights reserved
//==========================================================================

#include "listener.h"
#include "ot-log.h"

namespace ObTools { namespace XMLMesh {

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
  if (mesh) delete mesh;
  mesh = 0;
}

}} // namespaces

//==========================================================================
// ObTools::XMLMesh:Server: main.cc
//
// Main entry point for XMLMesh Server
//
// Copyright (c) 2003-2018 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "server.h"
#include "ot-log.h"

namespace {
const auto server_name    = "ObTools XMLMesh server";
const auto server_version = VERSION;
#ifdef DEBUG
const auto default_config_file = "xmlmesh.debug.cfg.xml";
#else
const auto default_config_file = "/etc/obtools/xmlmesh.cfg.xml";
#endif

const auto config_file_root = "xmlmesh";
const auto default_log_file = "/var/log/obtools/xmlmesh.log";
const auto pid_file         = "/var/run/ot-xmlmesh.pid";
}

using namespace std;
using namespace ObTools;
using namespace ObTools::XMLMesh;

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  Daemon::Shell shell(server, server_name, server_version,
                      default_config_file, config_file_root,
                      default_log_file, pid_file);
  return shell.start(argc, argv);
}


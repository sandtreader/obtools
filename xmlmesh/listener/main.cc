//==========================================================================
// ObTools::XMLMesh::Listener: main.cc
//
// Main file for XMLMesh listener daemon
//
// Copyright (c) 2016 Paul Clark.  All rights reserved
//==========================================================================

#include "listener.h"
#include "ot-log.h"

namespace {
const auto server_name    = "ObTools XMLMesh listener daemon";
const auto server_version = VERSION;
#ifdef DEBUG
const auto default_config_file = "listener.cfg.xml";
#else
const auto default_config_file = "/etc/obtools/listener.cfg.xml";
#endif
const auto config_file_root = "listener";
const auto default_log_file = "/var/log/obtools/listener.log";
const auto pid_file         = "/var/run/ot-xmlmesh-listener.pid";
}

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  XMLMesh::Server server;
  Daemon::Shell shell(server, server_name, server_version,
                      default_config_file, config_file_root,
                      default_log_file, pid_file);
  return shell.start(argc, argv);
}

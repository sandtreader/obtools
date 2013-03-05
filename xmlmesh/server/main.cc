//==========================================================================
// ObTools::XMLMesh:Server: main.cc
//
// Main entry point for XMLMesh Server
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "server.h"
#include "ot-log.h"
#include "ot-file.h"
#include <fstream>
#include <errno.h>
#ifndef __WIN32__
#include <unistd.h>
#endif

#define DEFAULT_LOGFILE "/var/log/obtools/xmlmesh.log"
#define DEFAULT_TIMESTAMP "%a %d %b %H:%M:%*S [%*L]: "
#define PID_FILE        "/var/run/ot-xmlmesh.pid"

using namespace std;
using namespace ObTools;
using namespace ObTools::XMLMesh;

// Global server instance
Server ObTools::XMLMesh::server;

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
#ifdef __WIN32__
  winsock_initialise();
#endif

  // Run initialisation sequence (auto-registration of modules etc.)
  Init::Sequence::run();

  // Grab config filename if specified
  XML::Configuration config;
  if (argc > 1)
    config.add_file(argv[argc-1]);  // Last arg, leaves room for options
  else
  {
    // Option of local or /etc
    config.add_file("xmlmesh.cfg.xml");
    config.add_file("/etc/obtools/xmlmesh.cfg.xml");
  }

  if (!config.read("xmlmesh"))
  {
    cerr << "Can't read configuration file\n";
    return 2;
  }

  // Set up logging
#if defined(DAEMON)
  string logfile = config.get_value("log/@file", DEFAULT_LOGFILE);
  ofstream logstream(logfile.c_str(),ios::app);
  if (!logstream)
  {
    cerr << "xmlmesh-server: Unable to open logfile " << logfile << endl;
    return 2;
  }
  Log::StreamChannel chan_out(logstream);
#else
  Log::StreamChannel chan_out(cout);
#endif
  Log::TimestampFilter tsfilter(config.get_value("log/@timestamp", 
						 DEFAULT_TIMESTAMP), chan_out);
  int log_level = config.get_value_int("log/@level", Log::LEVEL_SUMMARY);
  Log::LevelFilter level_out((Log::Level)log_level, tsfilter);
  Log::logger.connect(level_out);
  Log::Streams log;

#if defined(DAEMON)
  if (daemon(0 ,0))
    log.error << "Can't become daemon: " << strerror(errno) << endl;

  // Create pid file
  ofstream pidfile(PID_FILE);
  pidfile << getpid() << endl;
  pidfile.close();
#endif

#define FURKLE2(_x) #_x
#define FURKLE(_x) FURKLE2(_x)
  log.summary << "ObTools XMLMesh server '" << argv[0] << "' version " 
	      << FURKLE(OT_XMLMESH_VERSION) << " starting\n";
  
  // Configure server 
  server.configure(config);

#ifndef __WIN32__
  // Drop privileges if root
  if (!getuid())
  {
    string username = config["security/@user"];
    string groupname = config["security/@group"];

    // Set group first - needs to still be root
    if (!groupname.empty())
    {
      int gid = File::Path::group_name_to_id(groupname);

      if (gid >= 0)
      {
	log.summary << "Changing to group " << groupname 
		    << " (" << gid << ")\n";
	if (setgid((gid_t)gid))
	{
	  log.error << "Can't change group: " << strerror(errno) << endl;
	  goto shutdown;
	}
      }
      else 
      {
	log.error << "Can't find group " << groupname << "\n";
	goto shutdown;
      }
    }

    if (!username.empty())
    {
      int uid = File::Path::user_name_to_id(username);

      if (uid >= 0)
      {
	log.summary << "Changing to user " << username << " (" << uid << ")\n";
	if (setuid((uid_t)uid))
	{
	  log.error << "Can't change user: " << strerror(errno) << endl;
	  goto shutdown;
	}
      }
      else 
      {
	log.error << "Can't find user " << username << "\n";
	goto shutdown;
      }
    }
  }
#endif
  
  // Run server (never returns)
  server.run();

shutdown:
  return 0;  
}





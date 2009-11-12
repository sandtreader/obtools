//==========================================================================
// ObTools::Angel:angeld server.cc
//
// Implementation of angeld server object
//
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "angeld.h"
#include "ot-file.h"
#include "ot-xmlmesh-client-otmp.h"
#include <errno.h>

#define DEFAULT_PROCESS_DIR "/etc/angel/processes/"

namespace ObTools { namespace Angel { 

//------------------------------------------------------------------------
// Load processes 
bool Server::load_processes()
{
  // Get processes directory
  string proc_dn = get_value("process/@directory", DEFAULT_PROCESS_DIR);

  // Read all process definitions in directory
  File::Directory proc_dir(proc_dn);
  list<File::Path> files;
  if (proc_dir.inspect(files, "*.xml"))
  {
    // Iterate all process files
    for(list<File::Path>::iterator p=files.begin(); p!=files.end(); ++p)
    {
      File::Path fp = *p;
      XML::Configuration proc_config(fp.str());
      if (!proc_config.read("process"))
      {
	log.error << "Can't read process configuration file " << fp << endl;
	continue;
      }

      Process *proc = new Process(proc_config);
      processes.push_back(proc);
      process_ids[proc->id] = proc;

      log.summary << "Loaded process '" << proc->id << "': " 
		  << proc->name << endl;
    }

    return true;
  }
  else 
  {
    log.error << "Can't read process directory '" << proc_dn << "': "
	      << strerror(errno) << endl;
    return false;
  }
}
 
//------------------------------------------------------------------------
// Load config from XML
bool Server::configure()
{
  // Load processes and build dependency graph
  if (!load_processes()) return false;
  create_dependencies();

  // Run processes in dependency order
  start_processes();

  // !!! Set up mesh after processes started!
  // Set up mesh connection
  string host = get_value("xmlmesh/@host", "localhost");
  int port = get_value_int("xmlmesh/@port", 
			   XMLMesh::OTMP::DEFAULT_PORT);

  Net::IPAddress addr(host);
  if (!addr)
  {
    log.error << "Can't resolve XMLMesh host: " << host << endl;
    return false;
  }

  Net::EndPoint ep(addr, port);
  log.summary << "Connecting to XMLMesh at " << ep << endl;

  // Start mesh client
  mesh = new XMLMesh::OTMPMultiClient(ep);
  return true;
}

//------------------------------------------------------------------------
// Lookup a process by name
// Returns process or 0 if not found
Process *Server::lookup(const string& id)
{
  map<string, Process *>::iterator p = process_ids.find(id);
  if (p != process_ids.end())
    return p->second;
  else
    return 0;
}

//------------------------------------------------------------------------
// Create dependency graph
void Server::create_dependencies()
{
  for(list<Process *>::iterator p=processes.begin(); p!=processes.end(); ++p)
  {
    Process *proc = *p;

    // Iterate over all depend_ids loaded from config
    for(list<string>::iterator q=proc->depend_ids.begin();
	q!=proc->depend_ids.end();
	++q)
    {
      Process *dep = lookup(*q);
      if (dep)
      {
	// Connect graph
	proc->add_dependency(dep);
	dep->add_dependant(proc);
      }
      else log.error << "Process '" << proc->id
		     << "' depends on non-existent '" << *q 
		     << "' - dependency ignored\n";
    }
  }
}

//------------------------------------------------------------------------
// Run commands in dependency order
void Server::start_processes()
{
  // Ask all processes to start - they will recurse to start dependencies
  // if they need to
  for(list<Process *>::iterator p=processes.begin(); p!=processes.end(); ++p)
  {
    Process *proc = *p;
    proc->start();
  }
}

//--------------------------------------------------------------------------
// Server run method
void Server::run() 
{ 
  for(;;)
  {
    sleep(1); 

    //!!!
  }
}

//------------------------------------------------------------------------
// Destructor
Server::~Server()
{
  for(list<Process *>::iterator p=processes.begin(); p!=processes.end(); ++p)
    delete *p;
}

}} // namespaces





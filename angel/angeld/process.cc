//==========================================================================
// ObTools::Angel:angeld process.cc
//
// Implementation of angeld process object
//
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "angeld.h"

namespace ObTools { namespace Angel { 

//------------------------------------------------------------------------
// Constructor from XML config 
Process::Process(XML::Configuration& config): mark(false), started(false)
{
  // Get id, name and command
  id = config.get_value("@id");
  name = config.get_value("name");
  command = config.get_value("command");

  // Read dependencies - just the ids for now, the links will be made
  // once all the processes have been read in
  list<XML::Element *> depends = config.get_elements("depends");
  for(list<XML::Element *>::iterator p = depends.begin(); 
      p!=depends.end(); ++p)
  {
    XML::Element &e = **p;
    depend_ids.push_back(e["id"]);
  }
}

//------------------------------------------------------------------------
// Start process if not already started
bool Process::start()
{
  // Only do this once - may be entered many times
  if (started) return true;

  Log::Streams log;

  // Check for reentry because of dependency loop
  if (mark)
  {
    log.error << "Dependency loop involving process '" << id 
	      << "' - failing entire chain\n";
    return false;
  }
  mark = true;

  log.detail << "Checking dependencies for process '" << id << "'\n";

  if (dependencies.empty())
    log.detail << " - '" << id << "' is independent\n";

  // Check all dependencies and start these first - recursively, this
  // ensures processes are run in dependency order
  else for(list<Process *>::iterator p=dependencies.begin(); 
	   p!=dependencies.end(); ++p)
  {
    Process *dep = *p;
    log.detail << " - '" << id << "' depends on '" << dep->id << "'\n";
    if (!dep->start())
    {
      log.error << "Can't start process '" << id << "' because '"
		<< dep->id << "' failed\n";
      started = true;
      return false;
    }
  }

  log.summary << "Starting process '" << id << "'\n";
  started = true;

  // !!! Do process stuff
  return true;
}


}} // namespaces





//==========================================================================
// ObTools::Daemon: process.cc
//
// Implementation of daemon process
//
// Copyright (c) 2009 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-daemon.h"
#include "ot-log.h"
#include "ot-misc.h"
#include "ot-init.h"
#include "ot-file.h"
#include <execinfo.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fstream>

#define DEFAULT_TIMESTAMP "%a %d %b %H:%M:%*S [%*L]: "

namespace ObTools { namespace Daemon {

//--------------------------------------------------------------------------
// Signal handlers for both processes
static Process *the_process = 0;

// SIGTERM:  Clean shutdown
void sigterm(int)
{
  if (the_process) the_process->shutdown();
  signal(SIGTERM, SIG_IGN);
}

// SIGHUP:  Reload config
void sighup(int)
{
  if (the_process) the_process->reload();
  signal(SIGHUP, sighup);
}

// Various bad things!
void sigevil(int sig)
{
  if (the_process) the_process->log_evil(sig);
  signal(sig, SIG_DFL);
  raise(sig);
}

//--------------------------------------------------------------------------
// Start process, with arguments
// Returns process exit code
int Process::start(int argc, char **argv)
{
  // Run initialisation sequence (auto-registration of modules etc.)
  Init::Sequence::run();

  // Grab config filename if specified
  string cf = default_config_file;
  if (argc > 1) cf = argv[argc-1];  // Last arg, leaves room for options

  // Read config
  config.add_file(cf);
  if (!config.read(config_element))
  {
    cerr << argv[0] << ": Can't read config file " << cf << endl;
    return 2;
  }

#if defined(DEBUG)
  Log::StreamChannel chan_out(cout);
#else
  string logfile = config.get_value("log/@file", default_log_file);
  ofstream logstream(logfile.c_str(),ios::app);
  if (!logstream)
  {
    cerr << argv[0] << ": Unable to open logfile " << logfile << endl;
    return 2;
  }
  Log::StreamChannel chan_out(logstream);
#endif

  Log::TimestampFilter tsfilter(config.get_value("log/@timestamp", 
						 DEFAULT_TIMESTAMP), chan_out);
  int log_level = config.get_value_int("log/@level", Log::LEVEL_SUMMARY);
  Log::LevelFilter level_out((Log::Level)log_level, tsfilter);
  Log::logger.connect(level_out);
  Log::Streams log;
  log.summary << name << " version " << version << " starting\n";

#if !defined(DEBUG)
  // Full background daemon 
  if (daemon(0, 0))
    log.error << "Can't become daemon: " << strerror(errno) << endl;

  // Create pid file
  ofstream pidfile(pid_file.c_str());
  pidfile << getpid() << endl;
  pidfile.close();
#endif

  // Register signal handlers - same for both master and slave
  the_process = this;
  signal(SIGTERM, sigterm);
  signal(SIGHUP,  sighup);
  signal(SIGSEGV, sigevil);
  signal(SIGILL,  sigevil);
  signal(SIGFPE,  sigevil);
  signal(SIGABRT, sigevil);

  // Watchdog? Master/slave processes...
  if (enable_watchdog)
  {
    // Now loop forever restarting a child process, in case it fails
    bool first = true;
    while (!shut_down)
    {
      if (!first) log.error << "*** RESTARTING SLAVE ***\n";
      first = false;

      log.summary << "Forking slave process\n";
      slave_pid = fork();

      if (slave_pid < 0)
      {
	log.error << "Can't fork slave process: " << strerror(errno) << endl;
	continue;
      }

      if (slave_pid)
      {
	// PARENT PROCESS
	log.detail << "Slave process pid " << slave_pid << " forked\n";

	// Wait for it to exit
	int status;
	int died = waitpid(slave_pid, &status, 0);

	// Check for fatal failure
	if (died && !WIFEXITED(status))
	{
	  log.error << "*** Slave process " << slave_pid << " died ***\n"; 
	}
	else
	{
	  int rc = WEXITSTATUS(status);
	  if (shut_down && !rc)
	    log.summary << "Slave process exited OK\n";  // Expected
	  else
	    log.error << "*** Slave process " << slave_pid 
		      << " exited with code " << rc << " ***\n";
	}
      }
      else
      {
	// SLAVE PROCESS
	// Run subclass prerun before dropping priviledges
	int rc = run_priv();
	if (rc) return rc;

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
		exit(2);
	      }
	    }
	    else 
	    {
	      log.error << "Can't find group " << groupname << "\n";
	      exit(2);
	    }
	  }

	  if (!username.empty())
	  {
	    int uid = File::Path::user_name_to_id(username);

	    if (uid >= 0)
	    {
	      log.summary << "Changing to user " << username 
			  << " (" << uid << ")\n";
	      if (setuid((uid_t)uid))
	      {
		log.error << "Can't change user: " << strerror(errno) << endl;
		exit(2);
	      }
	    }
	    else 
	    {
	      log.error << "Can't find user " << username << "\n";
	      exit(2);
	    }
	  }
	}

	// Run subclass full startup
	return run();
      }
    }

    log.summary << "Master process exiting\n";
    return 0;
  }
  else
  {
    // Just run directly
    int rc = run_priv();
    if (rc) return rc;
    return run();
  }
}

//--------------------------------------------------------------------------
// Signal to shut down - called from SIGTERM handler first in master and then
// (because this passes it down) in slave
void Process::shutdown() 
{
  // Stop our restart loop (master) and any loop in the slave
  shut_down=true; 

  Log::Streams log;

  // Tell the slave to stop, if we're the master
  if (slave_pid)
  {
    log.summary << "SIGTERM received in master process\n";
    kill(slave_pid, SIGTERM);
  }
  else
  {
    log.summary << "SIGTERM received in slave process\n";
  }
}

//--------------------------------------------------------------------------
// Signal to reload config - called from SIGHUP handler first in master and then
// (because this passes it down) in slave
void Process::reload() 
{
  Log::Streams log;

  // Tell the slave to reload, if we're the master
  if (slave_pid)
  {
    log.summary << "SIGHUP received in master process\n";
    kill(slave_pid, SIGHUP);
  }
  else
  {
    log.summary << "SIGHUP received in slave process\n";
    reconfigure();
  }
}

//--------------------------------------------------------------------------
// Handle a failure signal
void Process::log_evil(int sig)
{
  string what;
  switch (sig)
  {
    case SIGSEGV: what = "segment violation"; break;
    case SIGILL:  what = "illegal instruction"; break;
    case SIGFPE:  what = "floating point exception"; break;
    case SIGABRT: what = "aborted"; break;
    default:      what = "unknown";
  }

  Log::Streams log;
  log.error << "*** Signal received in " << (slave_pid?"master":"slave")
	    << ": " << what << " (" << sig << ") ***\n";

  // Do a backtrace
#define MAXTRACE 100
  void *buffer[MAXTRACE];
  int n = backtrace(buffer, MAXTRACE);
  char **strings = backtrace_symbols(buffer, n);
  if (strings)
  {
    log.error << "--- backtrace:\n";
    for(int i=0; i<n; i++)
      log.error << "- " << strings[i] << endl;
    log.error << "---\n";
    free(strings);
  }
}

}} // namespaces





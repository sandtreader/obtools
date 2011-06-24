//==========================================================================
// ObTools::Daemon: shell.cc
//
// Implementation of daemon shell
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
#define FIRST_WATCHDOG_SLEEP_TIME 1
#define MAX_WATCHDOG_SLEEP_TIME 60

namespace ObTools { namespace Daemon {

//--------------------------------------------------------------------------
// Signal handlers for both processes
static Shell *the_shell = 0;

// SIGTERM:  Clean shutdown
void sigterm(int)
{
  if (the_shell) the_shell->shutdown();
  signal(SIGTERM, SIG_IGN);
}

// SIGHUP:  Reload config
void sighup(int)
{
  if (the_shell) the_shell->reload();
  signal(SIGHUP, sighup);
}

// Various bad things!
void sigevil(int sig)
{
  if (the_shell) the_shell->log_evil(sig);
  signal(sig, SIG_DFL);
  raise(sig);
}

//--------------------------------------------------------------------------
// Main run function
// Delegate entirely to the application
int Shell::run()
{
  int result = application.pre_run();
  if (result)
    return result;

  while (!shut_down)
  {
    result = application.tick();
    if (result)
      return result;
    MT::Thread::usleep(1);
  }
  return 0;
}

//--------------------------------------------------------------------------
// Start process, with arguments
// Returns process exit code
int Shell::start(int argc, char **argv)
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
  bool go_daemon = false;
#else
  bool go_daemon = config.get_value_bool("background/@daemon", true);
#endif

  // Create log stream if daemon
  ostream *sout = &cout;
  if (go_daemon)
  {
    string logfile = config.get_value("log/@file", default_log_file);
    sout = new ofstream(logfile.c_str(), ios::app);
    if (!*sout)
    {
      cerr << argv[0] << ": Unable to open logfile " << logfile << endl;
      return 2;
    }
  }

  Log::StreamChannel chan_out(*sout);
  Log::TimestampFilter tsfilter(config.get_value("log/@timestamp", 
						 DEFAULT_TIMESTAMP), chan_out);
  int log_level = config.get_value_int("log/@level", Log::LEVEL_SUMMARY);
  Log::LevelFilter level_out((Log::Level)log_level, tsfilter);
  Log::logger.connect(level_out);
  Log::Streams log;
  log.summary << name << " version " << version << " starting\n";

  // Call preconfigure before we go daemon - e.g. asking for SSL passphrase
  int rc = application.preconfigure();
  if (rc)
  {
    log.error << "Preconfigure failed: " << rc << endl;
    return rc;
  }
  // Tell application to read config settings
  application.read_config(config);


  // Full background daemon 
  if (go_daemon)
  {
    if (daemon(0, 0))
      log.error << "Can't become daemon: " << strerror(errno) << endl;

    // Create pid file
    string pid_file = config.get_value("daemon/pid/@file", default_pid_file);
    ofstream pidfile(pid_file.c_str());
    pidfile << getpid() << endl;
    pidfile.close();
  }

  // Register signal handlers - same for both master and slave
  the_shell = this;
  signal(SIGTERM, sigterm);
  signal(SIGHUP,  sighup);
  signal(SIGSEGV, sigevil);
  signal(SIGILL,  sigevil);
  signal(SIGFPE,  sigevil);
  signal(SIGABRT, sigevil);

  // Ignore SIGPIPE from closed sockets etc.
  signal(SIGPIPE, SIG_IGN);

  // Watchdog? Master/slave processes...
  bool enable_watchdog = config.get_value_bool("watchdog/@restart", true);
  int sleep_time = FIRST_WATCHDOG_SLEEP_TIME;
  if (go_daemon && enable_watchdog)
  {
    // Now loop forever restarting a child process, in case it fails
    bool first = true;
    while (!shut_down)
    {
      if (!first)
      {
	log.detail << "Waiting for " << sleep_time << "s\n";
	MT::Thread::sleep(sleep_time);

	// Exponential backoff, up to a max
	sleep_time *= 2;
	if (sleep_time > MAX_WATCHDOG_SLEEP_TIME) 
	  sleep_time = MAX_WATCHDOG_SLEEP_TIME;

	log.error << "*** RESTARTING SLAVE ***\n";
      }
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
	int rc = application.run_priv();
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
	rc = run();
	application.cleanup();
	return rc;
      }
    }

    log.summary << "Master process exiting\n";
    return 0;
  }
  else
  {
    // Just run directly
    rc = application.run_priv();
    if (rc) return rc;
    rc = run();
    application.cleanup();
    return rc;
  }
}

//--------------------------------------------------------------------------
// Signal to shut down - called from SIGTERM handler first in master and then
// (because this passes it down) in slave
void Shell::shutdown() 
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
void Shell::reload() 
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
    if (config.read(config_element))
      application.read_config(config);
    else
      log.error << "Failed to re-read config, using existing" << endl;
    application.reconfigure();
  }
}

//--------------------------------------------------------------------------
// Handle a failure signal
void Shell::log_evil(int sig)
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




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
#ifndef __WIN32__
#include <unistd.h>
#endif

#define DEFAULT_TIMESTAMP "%a %d %b %H:%M:%*S [%*L]: "
#define FIRST_WATCHDOG_SLEEP_TIME 1
#define MAX_WATCHDOG_SLEEP_TIME 60

namespace ObTools { namespace Daemon {

//--------------------------------------------------------------------------
// Signal handlers for both processes
static Shell *the_shell = 0;

const sighandler_t sig_ign(SIG_IGN);
const sighandler_t sig_dfl(SIG_DFL);

// SIGTERM:  Clean shutdown
void sigterm(int)
{
  if (the_shell) the_shell->signal_shutdown();
  signal(SIGTERM, sig_ign);
}

// SIGQUIT:  Quit from keyboard
void sigquit(int)
{
  if (the_shell) the_shell->signal_shutdown();
  signal(SIGQUIT, sig_ign);
}

// SIGHUP:  Reload config
void sighup(int)
{
  if (the_shell) the_shell->signal_reload();
  signal(SIGHUP, sighup);
}

// Various bad things!
void sigevil(int sig)
{
  if (the_shell) the_shell->log_evil(sig);
  signal(sig, sig_dfl);
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
    int wait = application.tick_wait();
    if (wait)
      this_thread::sleep_for(chrono::microseconds{wait});
    if (trigger_shutdown)
    {
      shutdown();
    }
    else if (trigger_reload)
    {
      reload();
      trigger_reload = false;
    }
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

  // Process <include> in config
  config.process_includes();

#if defined(DEBUG)
  bool go_daemon = config.get_value_bool("background/@daemon", false);
#else
  bool go_daemon = config.get_value_bool("background/@daemon", true);
#endif

  // Create log stream if daemon
  auto chan_out = static_cast<Log::Channel *>(nullptr);
  if (go_daemon)
  {
    if (config.get_value_bool("log/@syslog"))
    {
      chan_out = new Log::SyslogChannel;
    }
    else
    {
      string logfile = config.get_value("log/@file", default_log_file);
      auto sout = new ofstream(logfile.c_str(), ios::app);
      if (!*sout)
      {
        cerr << argv[0] << ": Unable to open logfile " << logfile << endl;
        return 2;
      }
      chan_out = new Log::OwnedStreamChannel{sout};
    }
  }
  else
  {
    chan_out = new Log::StreamChannel{&cout};
  }

  auto log_level = config.get_value_int("log/@level",
                                        static_cast<int>(Log::Level::summary));
  auto level_out = static_cast<Log::Level>(log_level);
  auto time_format = config.get_value("log/@timestamp", DEFAULT_TIMESTAMP);
  Log::logger.connect_full(chan_out, level_out, time_format);
  Log::Streams log;
  log.summary << name << " version " << version << " starting\n";

  // Tell application to read config settings
  application.read_config(config);

  // Call preconfigure before we go daemon - e.g. asking for SSL passphrase
  int rc = application.preconfigure();
  if (rc)
  {
    log.error << "Preconfigure failed: " << rc << endl;
    return rc;
  }

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
  if (!go_daemon) signal(SIGQUIT, sigquit); // quit from Ctrl-backslash
  signal(SIGHUP,  sighup);
  signal(SIGSEGV, sigevil);
  signal(SIGILL,  sigevil);
  signal(SIGFPE,  sigevil);
  signal(SIGABRT, sigevil);

  // Ignore SIGPIPE from closed sockets etc.
  signal(SIGPIPE, sig_ign);

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
        this_thread::sleep_for(chrono::seconds(sleep_time));

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
          if (rc)
          {
            log.error << "*** Slave process " << slave_pid
                      << " exited with code " << rc << " ***\n";
          }
          else
          {
            log.summary << "Slave process exited OK\n";  // Expected
            shut_down = true;
          }
        }
      }
      else
      {
        // SLAVE PROCESS
        // Run subclass prerun before dropping privileges
        int rc = application.run_priv();
        if (rc) return rc;

        rc = drop_privileges();
        if (rc) return rc;

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
    rc = drop_privileges();
    if (rc) return rc;
    rc = run();
    application.cleanup();
    return rc;
  }
}

//--------------------------------------------------------------------------
// Drop privileges if required
// Returns 0 on success, rc if not
int Shell::drop_privileges()
{
  // Drop privileges if root
  if (!getuid())
  {
    Log::Streams log;
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
        if (setgid(static_cast<gid_t>(gid)))
        {
          log.error << "Can't change group: " << strerror(errno) << endl;
          return 2;
        }
      }
      else
      {
        log.error << "Can't find group " << groupname << "\n";
        return 2;
      }
    }

    if (!username.empty())
    {
      int uid = File::Path::user_name_to_id(username);

      if (uid >= 0)
      {
        log.summary << "Changing to user " << username
                    << " (" << uid << ")\n";
        if (setuid(static_cast<uid_t>(uid)))
        {
          log.error << "Can't change user: " << strerror(errno) << endl;
          return 2;
        }
      }
      else
      {
        log.error << "Can't find user " << username << "\n";
        return 2;
      }
    }
  }

  return 0;
}

//--------------------------------------------------------------------------
// Shut down - indirectly called from SIGTERM handler first in master and
// then (because this passes it down) in slave
void Shell::shutdown()
{
  // Stop our restart loop (master) and any loop in the slave
  shut_down=true;

  // Tell the slave to stop, if we're the master
  if (slave_pid) kill(slave_pid, SIGTERM);
}

//--------------------------------------------------------------------------
// Reload config - indirectly called from SIGHUP handler first in master and
// then (because this passes it down) in slave
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





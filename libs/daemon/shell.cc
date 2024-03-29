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
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <fstream>
#if !defined(PLATFORM_WINDOWS)
#include <execinfo.h>
#include <sys/wait.h>
#include <unistd.h>
#else
#include <dbghelp.h>
typedef __p_sig_fn_t sighandler_t;
#endif

#define DEFAULT_TIMESTAMP "%a %d %b %H:%M:%*S [%*L]: "
#define DEFAULT_HOLD_TIME "1 min"
#define FIRST_WATCHDOG_SLEEP_TIME 1
#define MAX_WATCHDOG_SLEEP_TIME 60

namespace ObTools { namespace Daemon {

//--------------------------------------------------------------------------
// Signal handlers for both processes
static Shell *the_shell = 0;

const sighandler_t sig_ign(SIG_IGN);
const sighandler_t sig_dfl(SIG_DFL);

// Clean shutdown signals
void sigshutdown(int)
{
  if (the_shell) the_shell->signal_shutdown();
  signal(SIGTERM, sig_ign);
  signal(SIGINT, sig_ign);
#if !defined(PLATFORM_WINDOWS)
  signal(SIGQUIT, sig_ign);
#endif
}

#if !defined(PLATFORM_WINDOWS)
// SIGHUP:  Reload config
void sighup(int)
{
  if (the_shell) the_shell->signal_reload();
  signal(SIGHUP, sighup);
}
#endif

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
#if !defined(PLATFORM_WINDOWS)
    if (config.get_value_bool("log/@syslog"))
    {
      chan_out = new Log::SyslogChannel;
    }
    else
    {
#endif
      const auto log_conf = File::Path{config.get_value("log/@file",
                                                        default_log_file)};
      const auto log_exp = log_conf.expand();
      const auto logfile = File::Path{cf}.resolve(log_exp);
      const auto log_dir = logfile.dir();
      if (!log_dir.ensure(true))
      {
        cerr << argv[0] << ": Logfile directory can not be created: "
             << log_dir << endl;
        return 2;
      }
      auto sout = new ofstream(logfile.c_str(), ios::app);
      if (!*sout)
      {
        cerr << argv[0] << ": Unable to open logfile " << logfile << endl;
        return 2;
      }
      chan_out = new Log::OwnedStreamChannel{sout};
#if !defined(PLATFORM_WINDOWS)
    }
#endif
  }
  else
  {
    chan_out = new Log::StreamChannel{&cout};
  }

  auto log_level = config.get_value_int("log/@level",
                                        static_cast<int>(Log::Level::summary));
  auto level_out = static_cast<Log::Level>(log_level);
  auto time_format = config.get_value("log/@timestamp", DEFAULT_TIMESTAMP);
  auto hold_time = config.get_value("log/@hold-time", DEFAULT_HOLD_TIME);
  Log::logger.connect_full(chan_out, level_out, time_format, hold_time);
  Log::Streams log;
  log.summary << name << " version " << version << " starting\n";

  // Tell application to read config settings
  application.read_config(config, cf);

  // Call preconfigure before we go daemon - e.g. asking for SSL passphrase
  int rc = application.preconfigure();
  if (rc)
  {
    log.error << "Preconfigure failed: " << rc << endl;
    return rc;
  }

#if !defined(PLATFORM_WINDOWS)
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
#endif

  // Register signal handlers - same for both parent and child
  the_shell = this;
  signal(SIGTERM, sigshutdown);
  signal(SIGINT,  sigshutdown); // quit from Ctrl-C
#if !defined(PLATFORM_WINDOWS)
  signal(SIGQUIT, sigshutdown); // quit from Ctrl-backslash
  signal(SIGHUP,  sighup);
  // Ignore SIGPIPE from closed sockets etc.
  signal(SIGPIPE, sig_ign);
#endif
  signal(SIGSEGV, sigevil);
  signal(SIGILL,  sigevil);
  signal(SIGFPE,  sigevil);
  signal(SIGABRT, sigevil);

#if !defined(PLATFORM_WINDOWS)
  // Watchdog? Parent/child processes...
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

        log.error << "*** RESTARTING CHILD ***\n";
      }
      first = false;

      log.summary << "Forking child process\n";
      child_pid = fork();

      if (child_pid < 0)
      {
        log.error << "Can't fork child process: " << strerror(errno) << endl;
        continue;
      }

      if (child_pid)
      {
        // PARENT PROCESS
        log.detail << "Child process pid " << child_pid << " forked\n";

        // Wait for it to exit
        int status;
        int died = waitpid(child_pid, &status, 0);

        // Check for fatal failure
        if (died && !WIFEXITED(status))
        {
          log.error << "*** Child process " << child_pid << " died ***\n";
        }
        else
        {
          int rc = WEXITSTATUS(status);
          if (rc)
          {
            log.error << "*** Child process " << child_pid
                      << " exited with code " << rc << " ***\n";
          }
          else
          {
            log.summary << "Child process exited OK\n";  // Expected
            shut_down = true;
          }
        }

        // However it exited, if shutdown is requested, stop
        if (trigger_shutdown) shut_down = true;
      }
      else
      {
        // CHILD PROCESS
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

    log.summary << "Parent process exiting\n";
    return 0;
  }
  else
  {
#endif
    // Just run directly
    rc = application.run_priv();
    if (rc) return rc;
#if !defined(PLATFORM_WINDOWS)
    rc = drop_privileges();
    if (rc) return rc;
#endif
    rc = run();
    application.cleanup();
    return rc;
#if !defined(PLATFORM_WINDOWS)
  }
#endif
}

//--------------------------------------------------------------------------
// Drop privileges if required
// Returns 0 on success, rc if not
int Shell::drop_privileges()
{
#if defined(PLATFORM_WINDOWS)
  return -99;
#else
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
#endif
}

//--------------------------------------------------------------------------
// Shut down - indirectly called from SIGTERM handler
void Shell::shutdown()
{
  // Stop our restart loop (parent) and any loop in the child
  shut_down=true;
}

//--------------------------------------------------------------------------
// Reload config - indirectly called from SIGHUP handler
void Shell::reload()
{
  Log::Streams log;
  log.summary << "SIGHUP received\n";
  if (config.read(config_element))
    application.read_config(config);
  else
    log.error << "Failed to re-read config, using existing" << endl;
  application.reconfigure();
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
  log.error << "*** Signal received in " << (child_pid?"parent":"child")
            << ": " << what << " (" << sig << ") ***\n";

  // Do a backtrace
#define MAXTRACE 100
#define MAXNAMELEN 1024
  void *buffer[MAXTRACE];
#if defined(PLATFORM_WINDOWS)
  const auto frames = CaptureStackBackTrace(1, MAXTRACE, buffer, nullptr);
  if (frames)
  {
    log.error << "--- backtrace:\n";
    for (auto i = 0; i < frames; ++i)
    {
      auto sumbuff = vector<uint64_t>(sizeof(SYMBOL_INFO) + MAXNAMELEN
                                      + sizeof(uint64_t) - 1);
      auto *info = reinterpret_cast<SYMBOL_INFO *>(&sumbuff[0]);
      info->SizeOfStruct = sizeof(SYMBOL_INFO);
      info->MaxNameLen = 1024;

      auto displacement = uint64_t{};
      if (SymFromAddr(GetCurrentProcess(), (DWORD64)buffer[i], &displacement,
                      info))
      {
        log.error << "- " << string(info->Name, info->NameLen) << endl;
      }
    }
    log.error << "---\n";
  }
#else
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
#endif
}

}} // namespaces





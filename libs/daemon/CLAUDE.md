# CLAUDE.md - ObTools::Daemon Library

## Overview

`ObTools::Daemon` is a Unix daemon framework with process forking, monitoring, signal handling, and XML configuration. Lives under `namespace ObTools::Daemon`.

**Header:** `ot-daemon.h`
**Dependencies:** `ot-net`, `ot-log`, `ot-xml`, `ot-misc`, `ot-init`
**Platforms:** posix

## Key Classes

| Class | Purpose |
|-------|---------|
| `Application` | Abstract base for daemon logic (override tick/lifecycle methods) |
| `Shell` | Manages daemon process, forking, monitoring, signals, logging |
| `Process` | Convenience: combines Shell + Application |

## Application (override these)

```cpp
virtual void read_config(const XML::Configuration& config,
                         const string& config_filename);
virtual int preconfigure();         // 0 to continue, exit code to abort
virtual int run_priv();             // before privileges dropped
virtual void reconfigure();         // on SIGHUP
virtual int pre_run();              // before main loop
virtual int tick_wait();            // microseconds between ticks (default 10000)
virtual int tick();                 // main iteration, 0 to continue
virtual void cleanup();
```

## Shell

```cpp
Shell(Application& app, const string& name, const string& version,
      const string& default_config_file, const string& config_element,
      const string& default_log_file, const string& default_pid_file);
int start(int argc, char **argv);
void shutdown();
void reload();
void signal_shutdown();
void signal_reload();
void log_evil(int sig);

XML::Configuration config;  // public access to daemon config
```

## Process (combined Shell + Application)

```cpp
Process(const string& name, const string& version,
        const string& default_config_file, const string& config_element,
        const string& default_log_file, const string& default_pid_file);
```

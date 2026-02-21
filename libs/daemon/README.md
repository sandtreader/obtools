# ObTools::Daemon

Unix daemon framework with process forking, monitoring, signal handling, and XML-based configuration.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-daemon.h"
using namespace ObTools;

class MyDaemon: public Daemon::Process
{
public:
  MyDaemon(): Daemon::Process("mydaemon", "1.0",
    "/etc/mydaemon.cfg.xml", "mydaemon",
    "/var/log/mydaemon.log", "/var/run/mydaemon.pid") {}

  void read_config(const XML::Configuration& config,
                   const string& config_filename) override
  {
    // Read your configuration from XML
  }

  int pre_run() override
  {
    // Initialize before main loop
    return 0;
  }

  int tick() override
  {
    // Called repeatedly — do your work here
    return 0;  // return non-zero to exit
  }

  void cleanup() override
  {
    // Clean up on shutdown
  }
};

int main(int argc, char **argv)
{
  MyDaemon daemon;
  return daemon.start(argc, argv);
}
```

### Lifecycle

1. `read_config()` — parse XML configuration
2. `preconfigure()` — early setup
3. `run_priv()` — before privilege drop
4. `pre_run()` — final setup before loop
5. `tick()` — called every `tick_wait()` microseconds
6. `reconfigure()` — called on SIGHUP
7. `cleanup()` — called on shutdown

## Build

```
NAME    = ot-daemon
TYPE    = lib
DEPENDS = ot-net ot-log ot-xml ot-misc ot-init
```

## License

Copyright (c) 2004 Paul Clark. MIT License.

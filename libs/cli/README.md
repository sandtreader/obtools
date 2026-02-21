# ObTools::CLI

Command-line interface framework with hierarchical command groups, interactive prompts, and telnet integration.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-cli.h"
using namespace ObTools;

class MyApp
{
public:
  void show_status(string args, istream& sin, ostream& sout)
  {
    sout << "Status: OK" << endl;
  }

  void set_name(string args, istream& sin, ostream& sout)
  {
    sout << "Name set to: " << args << endl;
  }
};

int main()
{
  MyApp app;
  CLI::Registry registry;

  CLI::MemberHandler<MyApp> status_h(app, &MyApp::show_status);
  CLI::MemberHandler<MyApp> name_h(app, &MyApp::set_name);

  registry.add("show status", &status_h, "Display current status");
  registry.add("set name", &name_h, "Set the name", "<name>");

  CLI::CommandLine cli(registry, cin, cout, "myapp>");
  cli.run();
}
```

Session:

```
myapp> help
  show status   Display current status
  set name      Set the name
myapp> show status
Status: OK
myapp> set name Alice
Name set to: Alice
```

## Build

```
NAME    = ot-cli
TYPE    = lib
DEPENDS = ot-net ot-text
```

## License

Copyright (c) 2005 Paul Clark. MIT License.

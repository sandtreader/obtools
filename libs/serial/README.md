# ObTools::Serial

TTY (serial port) device control with termios configuration. Linux-only.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-serial.h"
using namespace ObTools;

Serial::TTY tty;
if (tty.open("/dev/ttyUSB0"))
{
  Serial::Parameters params;
  tty.get_parameters(params);

  params.in_speed = 38400;
  params.out_speed = 38400;
  params.char_flags = Serial::CharFlags::char_size_8
                    | Serial::CharFlags::enable_receiver;
  params.in_flags = Serial::InputFlags::none;
  params.out_flags = Serial::OutputFlags::none;
  params.local_flags = Serial::LocalFlags::none;

  tty.set_parameters(params);

  tty.write_line("AT");

  string response;
  auto result = tty.get_line(response, chrono::microseconds(1000000));
  if (result == Serial::TTY::GetLineResult::ok)
    cout << "Got: " << response << endl;

  tty.close();
}
```

## Build

```
NAME    = ot-serial
TYPE    = lib
DEPENDS = ot-gen
```

## License

Copyright (c) 2017 Paul Clark. MIT License.

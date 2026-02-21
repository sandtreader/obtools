# ObTools::Control

Classical feedback control systems — PID (proportional-integral-derivative) control loop.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-control.h"
using namespace ObTools;

// Create PID controller with gains
Control::PIDLoop pid(1.0, 0.1, 0.05);  // Kp=1.0, Ki=0.1, Kd=0.05
pid.set_set_point(100.0);               // target value

// Simulation loop
double process_value = 0.0;
for (double t = 0.0; t < 10.0; t += 0.01)
{
  double control = pid.tick(process_value, t);
  process_value += control * 0.01;  // simple plant model
}
```

### Proportional-Only Control

```cpp
Control::PIDLoop p_only(2.0);  // Kp only
p_only.set_set_point(50.0);
double output = p_only.tick(30.0, 1.0);  // error = 20, output = 40
```

## Build

```
NAME    = ot-control
TYPE    = lib
```

## License

Copyright (c) 2018 Paul Clark. MIT License.

# ObTools::Alarm

Wall-clock alarm system that triggers events at specified times using an observer pattern.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-alarm.h"
using namespace ObTools;

class MyObserver: public Alarm::Clock::Observer
{
public:
  void receive_alarm(const Time::Stamp& alarm_time) override
  {
    cout << "Alarm fired at " << alarm_time << endl;
  }
};

int main()
{
  Alarm::Clock clock(Time::Duration(0.020));  // 20ms resolution

  MyObserver observer;
  Time::Stamp when = Time::Stamp::now() + Time::Duration(5.0);
  clock.add_alarm(when, &observer);

  // Observer will be notified in ~5 seconds
  this_thread::sleep_for(chrono::seconds(6));
}
```

## Build

```
NAME    = ot-alarm
TYPE    = lib
DEPENDS = ot-mt ot-time
```

## License

Copyright (c) 2010 Paul Clark. MIT License.

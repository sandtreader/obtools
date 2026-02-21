# ObTools::Ring

Lock-free, fixed-size ring buffer for single-writer, single-reader communication. Header-only.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-ring.h"
using namespace ObTools;

Ring::Buffer<int> buffer(1024);  // 1024-element ring buffer

// Writer thread
buffer.put(42);
buffer.put(43);

// Reader thread
int value;
while (buffer.get(value))
  cout << "Got: " << value << endl;

// Check status
cout << "Used: " << buffer.used() << "/" << buffer.size() << endl;
```

### Producer-Consumer Pattern

```cpp
Ring::Buffer<Message> queue(256);

// Producer thread
void producer()
{
  while (running)
  {
    Message msg = create_message();
    while (!queue.put(msg))
      this_thread::yield();  // buffer full, wait
  }
}

// Consumer thread
void consumer()
{
  Message msg;
  while (running)
  {
    if (queue.get(msg))
      process(msg);
    else
      this_thread::yield();  // buffer empty, wait
  }
}
```

## Build

```
NAME    = ot-ring
TYPE    = headers
```

## License

Copyright (c) 2017 Paul Clark. MIT License.

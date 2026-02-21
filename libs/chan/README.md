# ObTools::Channel

Abstract binary channel protocol for structured read/write with network byte order, bit-level I/O, and multiple backends (streams, sockets, memory buffers, queues).

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-chan.h"
using namespace ObTools;

// Write to memory buffer
Channel::BlockWriter writer;
writer.write_nbo_32(0x12345678);
writer.write("hello", 5);
writer.write_byte(0xFF);

// Read from memory buffer
Channel::BlockReader reader(writer.get_data(), writer.get_length());
uint32_t val = reader.read_nbo_32();     // 0x12345678
char buf[5]; reader.read(buf, 5);        // "hello"
uint8_t b = reader.read_byte();          // 0xFF

// Stream-based I/O
Channel::StreamWriter sw(my_ostream);
Channel::StreamReader sr(my_istream);

// TCP socket I/O
Channel::TCPSocketWriter tw(socket);
Channel::TCPSocketReader tr(socket);

// Bit-level I/O
Channel::BitWriter bw(writer);
bw.write_bits(0x1F, 5);
bw.write_bool(true);
bw.flush();

Channel::BitReader br(reader);
uint64_t bits = br.read_bits(5);
bool flag = br.read_bool();
```

## Build

```
NAME    = ot-chan
TYPE    = lib
DEPENDS = ot-net
```

## License

Copyright (c) 2005 Paul Clark. MIT License.

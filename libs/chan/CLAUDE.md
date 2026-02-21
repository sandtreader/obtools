# CLAUDE.md - ObTools::Channel Library

## Overview

`ObTools::Channel` provides abstract binary channel read/write with network byte order, bit-level I/O, and multiple backends (streams, sockets, memory buffers, queues). Lives under `namespace ObTools::Channel`.

**Header:** `ot-chan.h`
**Dependencies:** `ot-net`

## Key Classes

| Class | Purpose |
|-------|---------|
| `Reader` / `Writer` | Abstract base with offset tracking |
| `StreamReader/Writer` | istream/ostream wrappers |
| `TCPSocketReader/Writer` | TCP socket I/O |
| `FDReader/Writer` | Raw file descriptor I/O |
| `BlockReader/Writer` | In-memory buffer I/O |
| `StringReader/Writer` | String buffer I/O |
| `DataQueueReader/Writer` | MT::DataQueue I/O |
| `BitReader/Writer` | Bit-level I/O (MSB-first) |
| `BitEGReader` | Exp-Golomb coded bitstream |
| `LimitedReader` | Decorator limiting read bytes |

## Reader Methods

```cpp
bool try_read(void *buf, size_t count);   // non-throwing
void read(void *buf, size_t count);       // throws Error
uint8_t read_byte();
uint16_t read_nbo_16();  uint32_t read_nbo_24();
uint32_t read_nbo_32();  uint64_t read_nbo_64();
uint16_t read_le_16();   uint32_t read_le_32();  uint64_t read_le_64();
double read_nbo_double();
double read_nbo_fixed_point(int bits_before, int bits_after);
string read_to_eof();
void skip(size_t count);  void align(int n);  void rewind();
uint64_t offset;  // current position
```

## Writer Methods

```cpp
void write(const void *buf, size_t count);
void write(const string& s);
void write_byte(uint8_t b);
void write_nbo_16/24/32/64(value);
void write_le_16/32/64(value);
void write_nbo_double(double d);
void write_nbo_fixed_point(double d, int before, int after);
void skip(size_t count);  void align(int n);  void rewind();
```

## BitReader / BitWriter

```cpp
// BitReader
bool read_bit();  bool read_bool();
uint64_t read_bits(int n);
uint64_t read_exp_golomb();

// BitWriter
void write_bit(bool b);  void write_bool(bool b);
void write_bits(uint64_t v, int n);
void flush();
```

## Error

```cpp
struct Error { int error; string text; };
```

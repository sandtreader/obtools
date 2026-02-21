# CLAUDE.md - ObTools::Serial Library

## Overview

`ObTools::Serial` provides TTY (serial port) device control with termios configuration. Linux-only. Lives under `namespace ObTools::Serial`.

**Header:** `ot-serial.h`
**Dependencies:** `ot-gen`
**Platforms:** Linux only

## Key Classes

| Class | Purpose |
|-------|---------|
| `TTY` | Main serial port controller |
| `Parameters` | Terminal configuration parameters |
| `SpecialChars` | Special terminal control characters |

## Flag Enums (all `enum class`, bitwise combinable)

| Enum | Purpose |
|------|---------|
| `InputFlags` | Terminal input mode (ignore_break, parity_check, nl_to_cr, xon, etc.) |
| `OutputFlags` | Terminal output mode (post_processing, nl_to_cr_nl, tab_delay, etc.) |
| `CharFlags` | Character mode (char_size_6/7/8, two_stop_bits, parity_gen, enable_rts_cts) |
| `LocalFlags` | Local mode (generate_signals, canonical_mode, echo, etc.) |

Operators: `operator&`, `operator|`, `has_flags()`

## Parameters

```cpp
struct Parameters {
  int in_speed, out_speed;              // baud rates (-1 = not set)
  InputFlags in_flags;
  OutputFlags out_flags;
  CharFlags char_flags;
  LocalFlags local_flags;
  SpecialChars special_chars;
  uint8_t min_chars_for_non_canon_read; // default 1
  chrono::milliseconds non_canon_read_timeout;
};
```

## TTY

```cpp
bool open(const string& device);
void close();
bool get_parameters(Parameters& params);
bool set_parameters(const Parameters& params);
enum class GetLineResult { ok, fail, timeout, interrupt };
GetLineResult get_line(string& line, const chrono::microseconds& timeout = 0);
bool write_line(const string& line);       // appends \r
explicit operator bool() const;
```

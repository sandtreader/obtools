# CLAUDE.md - ObTools::Log Library

## Overview

`ObTools::Log` is a hierarchical logging framework with multiple channels, filters, and severity levels. Lives under `namespace ObTools::Log`.

**Header:** `ot-log.h`
**Dependencies:** `ot-mt`, `ot-time`

## Log Levels

```cpp
enum class Level { none, error, summary, detail, debug, dump };
```

## Key Classes

| Class | Purpose |
|-------|---------|
| `Channel` | Abstract logging destination |
| `StreamChannel` | Log to ostream |
| `OwnedStreamChannel` | Log to owned ostream |
| `SyslogChannel` | Log to syslog (Unix) |
| `Distributor` | Fan-out hub (also a Channel) |
| `Filter` | Abstract filter chaining to another channel |
| `LevelFilter` | Filter by max level |
| `PatternFilter` | Filter by glob pattern |
| `TimestampFilter` | Prepend timestamps |
| `RepeatedMessageFilter` | Suppress repeats |
| `Stream` | ostream-style logging interface |

## Distributor (singleton: `Log::logger`)

```cpp
void connect(Channel *channel);
void connect_full(Channel *channel, Level level, const string& pattern="");
void log(const Message& msg);
```

## Convenience Classes

```cpp
Log::Error log;   log << "error" << endl;     // Level::error
Log::Summary log; log << "info" << endl;      // Level::summary
Log::Detail log;  log << "detail" << endl;    // Level::detail
Log::Debug log;   log << "debug" << endl;     // Level::debug
Log::Dump log;    log << "dump" << endl;      // Level::dump
```

## Streams Struct

```cpp
Log::Streams streams;   // holds Error, Summary, Detail, Debug, Dump members
```

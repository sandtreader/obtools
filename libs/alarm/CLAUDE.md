# CLAUDE.md - ObTools::Alarm Library

## Overview

`ObTools::Alarm` triggers events at specified wall-clock times using an observer pattern. Lives under `namespace ObTools::Alarm`.

**Header:** `ot-alarm.h`
**Dependencies:** `ot-mt`, `ot-time`

## Key Classes

| Class | Purpose |
|-------|---------|
| `Clock` | Alarm clock with observer registration |
| `Clock::Observer` | Abstract interface for alarm callbacks |

## Observer

```cpp
class Clock::Observer {
  virtual void receive_alarm(const Time::Stamp& alarm_time) = 0;
};
```

## Clock

```cpp
Clock(const Time::Duration& resolution);   // smallest check period
bool add_alarm(const Time::Stamp& time, Observer *observer);
bool remove_alarm(const Time::Stamp& time, Observer *observer);
```

## Design

- Background timer thread checks for due alarms every `resolution` period
- Multiple observers can register for the same alarm time
- Observer callbacks receive the requested alarm time
- Thread-safe with mutex protection

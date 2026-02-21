# CLAUDE.md - ObTools::Action Library

## Overview

`ObTools::Action` provides generic asynchronous action management with typed action queues and handler dispatch. Header-only. Lives under `namespace ObTools::Action`.

**Header:** `ot-action.h`
**Dependencies:** `ot-gen`, `ot-mt`
**Type:** headers

## Key Classes

| Class | Purpose |
|-------|---------|
| `Action<T>` | Abstract action base (T = enum for action types) |
| `Manager<T>` | Action queue with handler dispatch |
| `Manager<T>::Handler` | Abstract handler for actions |

## Action<T>

```cpp
template<typename T>
class Action {
  virtual T get_type() const = 0;
};
```

## Manager<T>

```cpp
template<typename T>
class Manager {
  class Handler {
    virtual void handle(const Action<T>& action) = 0;
  };

  void add_handler(T type, Handler& handler);
  enum QueueResult { ok, replaced_old };
  QueueResult queue(Action<T> *action);
  void set_queue_limit(int n);
  auto get_queue_length() const;
  map<T, vector<Handler *>> get_config();
};
```

## Design

- Actions are handled sequentially per handler but multiple handlers process in parallel
- Dispatcher thread distributes queued actions to handler threads
- Queue limit drops oldest actions when exceeded
- Thread-safe with condition-based synchronization

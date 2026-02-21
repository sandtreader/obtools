# ObTools::MT

A multithreading library for C++17, wrapping standard threads with higher-level primitives: conditions, semaphores, reader/writer mutexes, thread-safe queues, thread pools, and task management.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Features

- **Synchronisation primitives**: SpinMutex, Condition (boolean condition variable), Semaphore
- **Reader/writer mutex** with recursive write locking and writer priority
- **Thread-safe queues**: generic `Queue<T>` and binary `DataQueue`
- **Thread base class** with cancellation, priority, and timed sleep
- **Thread pool** with configurable min spares / max threads
- **Task system** for managed work units with clean shutdown
- **Function pool** for running `std::function<void()>` on pooled threads

## Dependencies

- `ot-gen` - ObTools generation utilities
- `ext-pthread` - POSIX threads

## Quick Start

```cpp
#include "ot-mt.h"
using namespace ObTools;
```

### Creating a Thread

Subclass `MT::Thread` and implement `run()`:

```cpp
class Worker: public MT::Thread
{
  void run() override
  {
    while (is_running())
    {
      // Do work here
      sleep_for(chrono::seconds(1));  // respects cancel()
    }
  }
};

Worker worker;
worker.start();       // start the thread
// ...
worker.cancel();      // ask thread to stop
worker.join();        // wait for it to finish
```

### Thread Priority

```cpp
Worker worker;
worker.start();
worker.set_priority(10);             // normal scheduling
worker.set_priority(50, true);       // realtime (SCHED_RR)
```

### Condition Variables

A thread-safe boolean flag with wait/signal semantics:

```cpp
MT::Condition ready(false);

// Thread 1: wait for condition
ready.wait();                               // blocks until true
ready.wait_for(chrono::seconds(5));         // timed wait
ready.wait_until(deadline);                 // deadline wait
ready.wait(false);                          // wait for false

// Thread 2: signal
ready.signal();           // wake one waiter, set true
ready.broadcast(true);    // wake all waiters, set true
ready.clear();            // set false without signalling

// Check without waiting
if (ready) { /* condition is true */ }
```

### Semaphores

Classical counting semaphore (Dijkstra P/V):

```cpp
MT::Semaphore sem;          // count = 0
MT::Semaphore sem(5);       // count = 5 (pre-seeded)

// Producer
sem.signal();       // V: increment count
sem.signal(3);      // V(3): increment by 3

// Consumer
sem.wait();         // P: block until count > 0, then decrement
```

RAII holder:

```cpp
MT::Semaphore slots(10);

{
  MT::SemaphoreHolder holder(slots);  // waits on construction
  // ... use the slot ...
}  // signals on destruction
```

Named semaphore registry:

```cpp
MT::SemaphoreMap smap;
MT::Semaphore& db_sem = smap.get_semaphore("database", 5);
```

### Spin Mutex

Lightweight spinning mutex for very short critical sections:

```cpp
MT::SpinMutex smtx;

{
  MT::SpinLock lock(smtx);  // spins until acquired
  // ... very short critical section ...
}  // released on destruction
```

### Reader/Writer Mutex

Multiple concurrent readers, exclusive writer access. Supports recursive locking with writer priority.

Recursion rules:
- Read within read: OK
- Write within write: OK
- Read within write: OK
- Write within read: **NOT OK** (lock for write first)

```cpp
MT::RWMutex rwm;

// Read access (multiple threads can hold simultaneously)
{
  MT::RWReadLock lock(rwm);
  // ... read shared data ...
}

// Write access (exclusive)
{
  MT::RWWriteLock lock(rwm);
  // ... modify shared data ...
  // can also read safely here (read-in-write is OK)
}
```

### Thread-Safe Queue

Generic message queue with blocking receive:

```cpp
MT::Queue<string> messages;

// Producer (non-blocking)
messages.send("hello");
messages.emplace("world");

// Consumer (blocking)
string msg = messages.wait();

// Non-blocking check
if (messages.poll())
  string msg = messages.wait();  // guaranteed not to block

// Queue management
messages.limit(100);       // cap at 100, drop oldest
messages.flush();          // clear all
auto n = messages.waiting(); // current length
```

### Data Queue

Specialised queue for streaming binary data:

```cpp
MT::DataQueue dq;

// Writer thread
const char *data = "hello";
dq.write(reinterpret_cast<const MT::DataBlock::data_t *>(data), 5);
dq.close();  // send EOF

// Reader thread
MT::DataBlock::data_t buf[1024];
MT::DataBlock::size_t n;
while ((n = dq.read(buf, sizeof(buf))) > 0)
{
  // process n bytes in buf
}
// n == 0 means EOF
```

### Thread Pool

Manage a pool of reusable threads with configurable bounds:

```cpp
// Define a poolable thread
class MyPoolThread: public MT::PoolThread
{
public:
  string work_item;

  MyPoolThread(MT::IPoolReplacer& r): PoolThread(r) {}

  void run() override
  {
    // process work_item
  }
};

// Create pool: min 2 spares, max 10 threads
MT::ThreadPool<MyPoolThread> pool(2, 10);

// Use a thread from the pool
MyPoolThread *t = pool.remove();  // non-blocking, may return nullptr
if (t)
{
  t->work_item = "task data";
  t->kick();   // activate the thread
}

// Or wait for one (blocking)
MyPoolThread *t = pool.wait();
t->work_item = "task data";
t->kick();

// Thread returns itself to pool when run() completes

// Check pool state
if (pool.available()) { /* spares exist */ }
if (pool.active()) { /* threads are working */ }

// Shutdown
pool.shutdown();  // terminates all threads
```

### Function Pool

Simpler pool for running functions:

```cpp
MT::FunctionPool pool(2, 8);

// Fire and forget
pool.run([]{ do_expensive_work(); });

// Run multiple functions and wait for all
vector<function<void()>> tasks = {
  []{ step_a(); },
  []{ step_b(); },
  []{ step_c(); },
};
pool.run_and_wait(tasks);
```

### Task System

For managed work units with clean shutdown:

```cpp
class MyTask: public MT::Task
{
public:
  int result = 0;

  void run() override
  {
    while (is_running())
    {
      // do work
      result++;
      sleep_for(chrono::milliseconds(100));
    }
  }

  void shutdown() override
  {
    // custom cleanup
    MT::Task::shutdown();  // sets running = false
  }
};

// TaskThread takes ownership and starts immediately
MT::TaskThread<MyTask> tt(new MyTask);

// Access task
int r = tt->result;

// Destructor calls shutdown(), sends shutdown_signal(), joins thread
```

## API Reference

### Synchronisation Primitives

| Class | Methods |
|-------|---------|
| `SpinMutex` | `lock()`, `unlock()` |
| `SpinLock` | Constructor acquires, destructor releases |
| `Condition` | `wait(desired)`, `wait_for(duration, desired)`, `wait_until(tp, desired)`, `signal(value)`, `broadcast(value)`, `clear()`, `operator bool()` |
| `Semaphore` | `signal()`, `signal(n)`, `wait()` |
| `SemaphoreHolder` | RAII: `wait()` on construction, `signal()` on destruction |
| `SemaphoreMap` | `get_semaphore(name, initial_count)` |

### Thread Classes

| Class | Key Methods |
|-------|-------------|
| `Thread` | `start()`, `cancel()`, `join()`, `detach()`, `kill(sig)`, `set_priority(pri, rt)` |
| `RWMutex` | `lock_reader()`, `unlock_reader()`, `lock_writer()`, `unlock_writer()` |
| `RWReadLock` | RAII read lock |
| `RWWriteLock` | RAII write lock |

### Queue Classes

| Class | Key Methods |
|-------|-------------|
| `Queue<T>` | `send(msg)`, `emplace(args...)`, `wait()`, `poll()`, `limit(n)`, `flush()`, `waiting()` |
| `DataQueue` | `write(data, len)`, `read(buf, len, block)`, `close()` |

### Pool Classes

| Class | Key Methods |
|-------|-------------|
| `PoolThread` | `kick()`, `die(wait)` |
| `ThreadPool<T>` | `remove()`, `wait()`, `replace(t)`, `available()`, `active()`, `get_actives()`, `shutdown()`, `get/set_min_spares()`, `get/set_max_threads()` |
| `FunctionPool` | `run(f)`, `run_and_wait(vf)` |

### Task Classes

| Class | Key Methods |
|-------|-------------|
| `Task` | `run()`, `is_running()`, `sleep_for()`, `sleep_until()`, `shutdown()`, `shutdown_signal()` |
| `TaskThread<T>` | `operator->()`, `kill(sig)` |

## Build

```
NAME    = ot-mt
TYPE    = lib
DEPENDS = ot-gen ext-pthread
```

## License

Copyright (c) 2003 Paul Clark. MIT License.

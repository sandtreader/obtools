# CLAUDE.md - ObTools::MT Library

## Overview

`ObTools::MT` is a multithreading library wrapping C++ standard threads with higher-level primitives: conditions, semaphores, reader/writer mutexes, thread-safe queues, thread pools, and task management. Lives under `namespace ObTools::MT`.

**Header:** `ot-mt.h`
**Dependencies:** `ot-gen`, `ext-pthread`

## Type Aliases

```cpp
using Mutex = mutex;
using RMutex = recursive_mutex;
using Lock = unique_lock<mutex>;
using RLock = unique_lock<recursive_mutex>;
using BasicCondVar = condition_variable;
```

## Key Classes

| Class | Purpose |
|-------|---------|
| `SpinMutex` / `SpinLock` | Lightweight spinning mutex with RAII lock |
| `Condition` | Thread-safe boolean condition variable with wait/signal |
| `Semaphore` | Classical counting semaphore (Dijkstra P/V) |
| `SemaphoreHolder` | RAII wrapper for Semaphore |
| `SemaphoreMap` | Named semaphore registry |
| `Thread` | Abstract base class for threads (subclass and implement `run()`) |
| `RWMutex` | Multiple-reader/single-writer mutex with recursive write support |
| `RWReadLock` / `RWWriteLock` | RAII wrappers for RWMutex |
| `Queue<T>` | Thread-safe generic message queue |
| `DataQueue` | Queue specialised for binary data blocks |
| `PoolThread` | Poolable thread base class |
| `ThreadPool<T>` | Generic thread pool with min spares/max threads |
| `Task` | Abstract base class for managed tasks |
| `TaskThread<T>` | Wrapper that runs a Task on a thread |
| `FunctionPoolThread` / `FunctionPool` | Thread pool for `std::function<void()>` |

## Condition

```cpp
Condition(bool initial = false);
void wait(bool desired = true);                       // block until flag == desired
void wait_for(duration, bool desired = true);         // timed wait
void wait_until(time_point, bool desired = true);     // deadline wait
void signal(bool value = true);                       // signal one waiter
void broadcast(bool value = true);                    // signal all waiters
void clear();                                         // set false without signal
explicit operator bool() const;                       // read flag
```

## Semaphore

```cpp
Semaphore();
Semaphore(int initial_count);
void signal();              // V: increment count
void signal(int n);         // V(n): increment by n
void wait();                // P: block until count>0, then decrement
```

`SemaphoreHolder` - RAII: waits on construction, signals on destruction.
`SemaphoreMap` - `get_semaphore(name, initial_count)` returns named semaphore.

## Thread

Subclass and implement `run()`. Call `start()` to begin.

```cpp
virtual void run() = 0;             // implement this
virtual bool start();               // start thread
void join();                        // wait for thread to end
void detach();                      // let thread die without join
virtual void cancel();              // ask thread to stop
void kill(int signal = SIGTERM);    // send signal
bool set_priority(int, bool realtime = false);
```

Protected helpers: `is_running()`, `sleep_for(duration)`, `sleep_until(time_point)` - all respect `cancel()`.

## RWMutex

Reader/writer mutex. Recursion rules: read-in-read OK, write-in-write OK, read-in-write OK, write-in-read NOT OK.

```cpp
void lock_reader();    void unlock_reader();
void lock_writer();    void unlock_writer();
```

`RWReadLock(mutex)` / `RWWriteLock(mutex)` - RAII wrappers (non-copyable).

## Queue<T>

```cpp
void send(const T msg);                      // enqueue (non-blocking)
void emplace(Args... args);                  // construct-in-place
T wait();                                    // dequeue (blocking)
bool poll();                                 // non-blocking check
bool limit(size_type n);                     // cap queue size
void flush();                                // clear queue
size_type waiting() const;                   // current length
```

## DataQueue

Inherits `Queue<DataBlock>`. For streaming binary data.

```cpp
void write(const data_t *data, size_t length);
size_t read(data_t *data, size_t length, bool block = true);
void close();   // send EOF
```

## ThreadPool<T>

```cpp
ThreadPool(unsigned min_spares=1, unsigned max_threads=10, bool realtime=false);
T *remove();                  // get spare thread (nullptr if none)
T *wait();                    // blocking get
void replace(T *t);           // return thread to pool
bool available();             // any spares?
bool active();                // any actives?
list<T*>& get_actives();
void shutdown();
```

## Task / TaskThread<T>

```cpp
// Task: subclass and implement run()
virtual void run() = 0;
bool is_running() const;
void sleep_for(duration);
void sleep_until(time_point);
virtual void shutdown();
virtual int shutdown_signal();

// TaskThread: owns a Task, runs it on a thread
TaskThread(T *task);      // takes ownership, starts immediately
T *operator->();          // access task
// destructor: shutdown + join
```

## FunctionPool

```cpp
FunctionPool(unsigned min_spares=1, unsigned max_threads=10, bool realtime=false);
bool run(function<void()> f);                    // run async
void run_and_wait(vector<function<void()>>& vf); // run all, wait for completion
```

## File Layout

```
ot-mt.h            - Public header (all classes)
thread.cc          - Thread implementation
dqueue.cc          - DataQueue implementation
pool.cc            - ThreadPool/PoolThread implementation
test-thread.cc     - Thread tests (gtest)
test-rwmutex.cc    - RWMutex tests
test-semaphore.cc  - Semaphore tests
test-dqueue.cc     - DataQueue tests
test-pool.cc       - ThreadPool tests
test-queue.cc      - Queue tests
Tupfile            - Build configuration
```

## Common Patterns

```cpp
// Simple thread
class MyWorker: public MT::Thread
{
  void run() override
  {
    while (is_running())
    {
      // do work
      sleep_for(chrono::seconds(1));
    }
  }
};

MyWorker worker;
worker.start();
// ... later ...
worker.cancel();
worker.join();

// Thread-safe queue
MT::Queue<string> q;
q.send("hello");
string msg = q.wait();  // blocks

// Function pool
MT::FunctionPool pool(2, 8);
pool.run([]{ /* async work */ });

// Reader/writer lock
MT::RWMutex rwm;
{ MT::RWReadLock lock(rwm); /* read safely */ }
{ MT::RWWriteLock lock(rwm); /* write safely */ }
```

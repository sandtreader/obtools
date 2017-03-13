//==========================================================================
// ObTools::MT: ot-mt.h
//
// Public definitions for ObTools::MT
// C++ wrapping of standard thread library and useful multithreading bits -
// queues, etc.
//
// Copyright (c) 2003-2016 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_MT_H
#define __OBTOOLS_MT_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <list>
#include <memory>
#include <signal.h>
#include <stdint.h>
#include <ot-gen.h>

namespace ObTools { namespace MT {

// Make our lives easier without polluting anyone else
using namespace std;

using Mutex = mutex;
using RMutex = recursive_mutex;
using Lock = unique_lock<mutex>;
using RLock = unique_lock<recursive_mutex>;
using BasicCondVar = condition_variable;

//==========================================================================
// Condition variable class (boolean condition)
// Implements safe signalling on a simple boolean variable
class Condition
{
private:
  volatile bool flag;
  condition_variable cv;
  mutable mutex m;

public:
  //------------------------------------------------------------------------
  // Default Constructor - initialises condvar and mutex
  Condition(bool initial = false):
    flag{initial}
  {}

  //------------------------------------------------------------------------
  // Get the current value
  operator bool() const
  {
    unique_lock<mutex> lock{m};
    return flag;
  }

  //------------------------------------------------------------------------
  // Wait on the condition to become as desired
  void wait(bool desired = true)
  {
    unique_lock<mutex> lock{m};
    while (flag != desired)
      cv.wait(lock);
  }

  //------------------------------------------------------------------------
  // Wait on the condition to become as desired, or for given period
  template <class Rep, class Period>
    void wait_for(const std::chrono::duration<Rep, Period>& time,
                  bool desired = true)
  {
    unique_lock<mutex> lock{m};
    if (flag != desired)
      cv.wait_for(lock, time);
  }

  //------------------------------------------------------------------------
  // Wait on the condition to become as desired, or until given time
  template <class Clock, class Duration>
    void wait_until(const std::chrono::time_point<Clock, Duration>& time,
                  bool desired = true)
  {
    unique_lock<mutex> lock{m};
    if (flag != desired)
      cv.wait_until(lock, time);
  }

  //------------------------------------------------------------------------
  // Signal the condition to become as stated
  void signal(bool value = true)
  {
    unique_lock<mutex> lock{m};
    if (flag != value)
    {
      flag = value;
      cv.notify_one();
    }
  }

  //------------------------------------------------------------------------
  // Broadcast the condition to become as stated (multiple waiters)
  void broadcast(bool value = true)
  {
    unique_lock<mutex> lock{m};
    if (flag != value)
    {
      flag = value;
      cv.notify_all();
    }
  }

  //------------------------------------------------------------------------
  // Clear the flag without signalling.
  // This is used with the default 'true' parameters of wait/signal/broadcast
  // to implement 'rising-edge-only' synchronisation.  If you want both edges
  // synchronised, manually wait/signal/broadcast for 'false' too
  void clear() { flag = false; }
};

//==========================================================================
// Abstract Thread class
class Thread
{
  friend class PoolThread;
private:
  mutable Condition running;    // whether running or not
  unique_ptr<thread> mythread;  // thread

  //------------------------------------------------------------------------
  // Virtual run routine - called once thread has started by start()
  virtual void run() = 0;

protected:
  //------------------------------------------------------------------------
  // Test if running
  bool is_running() const
  { return running; }

  //------------------------------------------------------------------------
  // Sleep for a given period, or until thread told to stop
  template <class Rep, class Period>
    void sleep_for(const std::chrono::duration<Rep,Period>& time)
  {
    running.wait_for(time, false);
  }

  //------------------------------------------------------------------------
  // Sleep until a given time, or until thread told to stop
  template <class Clock, class Duration>
    void sleep_until(const std::chrono::time_point<Clock, Duration>& time)
  {
    running.wait_until(time, false);
  }

public:

  //------------------------------------------------------------------------
  // Start - note separate from default constructor to allow you time to create
  // parameters in subclass constructors
  // Returns whether successful
  virtual bool start();

  //------------------------------------------------------------------------
  // Set priority - higher numbers have higher priority
  // realtime sets SCHED_RR if set
  // Whether successful (may fail if realtime requested when not root)
  bool set_priority(int priority, bool realtime = false);

  //------------------------------------------------------------------------
  // Join - caller waits for this thread to end
  void join()
  {
    if (mythread && mythread->joinable())
      mythread->join();
  }

  //------------------------------------------------------------------------
  // Detach - let it die silently when it ends, we aren't going to join with it
  void detach()
  {
    if (mythread)
      mythread->detach();
  }

  //------------------------------------------------------------------------
  // Cancel - ask it to stop
  virtual void cancel();

  //------------------------------------------------------------------------
  // Kill a thread (or send another signal)
  void kill(int signal = SIGTERM)
  {
    if (mythread)
      pthread_kill(mythread->native_handle(), signal);
  }

  //------------------------------------------------------------------------
  // Test if it has stopped
  bool operator!() const { return !is_running(); }

  //------------------------------------------------------------------------
  // Destructor - ask it to cancel if started
  virtual ~Thread();
};

//==========================================================================
// Multiple readers/single writer Mutex class
// Like pthread_rwlock, but provides ability for recursive use by writer
// that would otherwise deadlock
// Implements writer priority

// The mutex can be used recursively as follows:
//    A read within another read is OK (obviously)
//    A write within another write is OK
//    A read within a write is OK
// but
//    A write within a read is NOT OK

// Hence if mixed-mode recursion is required, lock for write first

class RWMutex
{
private:
  // Internal mutex, only locked transiently
  mutex mymutex;

  // Count and condition for readers
  volatile unsigned readers_active;
  condition_variable no_readers;

  // Count and condition for writers
  volatile unsigned writers_waiting;
  volatile bool writer_active;
  condition_variable no_writer;

  // Support for recursive locks
  volatile unsigned count;
  thread::id writer;

public:
  //------------------------------------------------------------------------
  // Default Constructor - initialises mutex
  RWMutex():
    readers_active{0}, writers_waiting{0}, writer_active{false}
  {}

  //------------------------------------------------------------------------
  // Lock the mutex for read
  void lock_reader()
  {
    unique_lock<mutex> lock{mymutex};

    // Check if we already own this as a writer
    if (!writer_active || writer != this_thread::get_id())
    {
      // If I'm the first reader in, wait until there are no writers,
      // either queued or active - after that, my presence will protect
      // the rest (including any that recurse inside me)
      // NB:  Without this check, a recursed read which happens after a
      // writer has started waiting for the lock will deadlock with it
      if (!readers_active)
      {
        while (writers_waiting || writer_active) no_writer.wait(lock);
      }

      // Claim it and block writers
      ++readers_active;
    }
  }

  //------------------------------------------------------------------------
  // Unlock the mutex for read
  void unlock_reader()
  {
    unique_lock<mutex> lock{mymutex};

    // Check if we already own this as a writer
    if (!writer_active || writer != this_thread::get_id())
    {
      // Wake up all writers so they can proceed through the no-readers gate
      if (!--readers_active) no_readers.notify_all();
    }
  }

  //------------------------------------------------------------------------
  // Lock the mutex for write
  void lock_writer()
  {
    unique_lock<mutex> lock{mymutex};
    const auto self = this_thread::get_id();

    // Check if we already own this
    if (writer_active && writer == self)
    {
      // Accumulate another one, but don't lock again
      count++;
    }
    else
    {
      // Show there are writers waiting, to ensure priority
      writers_waiting++;

      // Wait until there are no readers
      while (readers_active) no_readers.wait(lock);

      // Wait until there are no other writers using it
      while (writer_active) no_writer.wait(lock);

      // We're not waiting any more, we're active
      writers_waiting--;
      writer_active = true;

      // Show we now own this
      writer = self;
      count = 1;
    }
  }

  //------------------------------------------------------------------------
  // Unlock the mutex for write
  void unlock_writer()
  {
    unique_lock<mutex> lock{mymutex};

    // Check we've bottomed out any recursion
    if (!--count)
    {
      writer_active = false;                  // Let other writers go

      // Wake up all writers and readers - note reader may be ahead of a
      // writer in the queue, and we must ensure the writer gets woken up
      no_writer.notify_all();
    }
  }
};

//==========================================================================
// Reader/Writer Lock class - as Lock, but claiming reader side of an RWMutex
class RWReadLock
{
private:
  RWMutex& mutex;
  RWReadLock(const RWReadLock& x) = delete;
  void operator=(const RWReadLock&) = delete;

public:
  RWReadLock(RWMutex &m): mutex{m} { mutex.lock_reader(); }
  ~RWReadLock() { mutex.unlock_reader(); }
};

//==========================================================================
// Ditto, but for writer side
class RWWriteLock
{
private:
  RWMutex& mutex;
  RWWriteLock(const RWWriteLock& x) = delete;
  void operator=(const RWWriteLock&) = delete;

public:
  RWWriteLock(RWMutex &m): mutex{m} { mutex.lock_writer(); }
  ~RWWriteLock() { mutex.unlock_writer(); }
};

//==========================================================================
// Queue template - send any message between threads safety
// Uses the non-emptiness of the queue as the condition variable, and
// signals on every send, to guarantee any waiters are woken
template<class T> class Queue
{
private:
  queue<T> q;
  mutex mymutex;
  condition_variable available;

public:
  //------------------------------------------------------------------------
  // Constructor
  Queue() {}

  //------------------------------------------------------------------------
  // Get current length
  typename queue<T>::size_type waiting()
  {
    unique_lock<mutex> lock{mymutex};
    return q.size();
  }

  //------------------------------------------------------------------------
  // Send a message (never blocks)
  void send(const T msg)
  {
    unique_lock<mutex> lock{mymutex};
    q.push(msg);
    available.notify_one();
  }

  //------------------------------------------------------------------------
  // See if any message is available before potentially blocking on wait()
  bool poll()
  {
    unique_lock<mutex> lock{mymutex};
    return (!q.empty());
  }

  //------------------------------------------------------------------------
  // Wait to receive a message (blocking)
  T wait()
  {
    // Can loop if someone else gets it
    for(;;)
    {
      unique_lock<mutex> lock{mymutex};

      // If nothing there, wait for availability signal
      if (q.empty())
        available.wait(lock);

      // Check again in case someone else got there first
      if (!q.empty())
      {
        T msg = q.front();
        q.pop();

        return msg;
      }
    }
  }

  //------------------------------------------------------------------------
  // Get count of messages left in queue
  typename queue<T>::size_type count()
  {
    unique_lock<mutex> lock{mymutex};
    return q.size();
  }

  //------------------------------------------------------------------------
  // Limit the number of messages in the queue, popping off the oldest
  // if too many.  Returns true if the limit was applied.
  bool limit(typename queue<T>::size_type n)
  {
    unique_lock<mutex> lock{mymutex};
    bool result = false;
    while (q.size() > n)
    {
      q.pop();
      result = true;
    }
    return result;
  }

  //------------------------------------------------------------------------
  // Flush the queue
  void flush()
  {
    unique_lock<mutex> lock{mymutex};
    q = queue<T>();
  }
};

//==========================================================================
// Data queue class (dqueue.cc)
// Specific Queue for data blocks, with read/write support
// Provides a generic cross-thread data buffer with blocking on the read
// side - see also Channel::DataQueueReader/Writer
struct DataBlock
{
  typedef unsigned char data_t;
  typedef unsigned long size_t;

  data_t *data;        // nullptr = EOF marker
  size_t length;
  DataBlock(): data{nullptr}, length{0} {}
  DataBlock(data_t *_data, size_t _length):
    data(_data), length(_length) {}
};

class DataQueue: public Queue<DataBlock>
{
  DataBlock working_block;
  DataBlock::size_t working_block_used;
  bool eof;

public:
  //------------------------------------------------------------------------
  // Constructor
  DataQueue(): working_block_used{0}, eof{false} {}

  //------------------------------------------------------------------------
  // Write a block to the queue
  void write(const DataBlock::data_t *data, DataBlock::size_t length);

  //------------------------------------------------------------------------
  // Set EOF
  void close() { send(DataBlock()); }

  //------------------------------------------------------------------------
  // Read data from the queue - blocking, or non-blocking
  // Reads data to the amount requested, or to EOF (blocking) or whatever
  // is available (non-blocking)
  // If data is 0, just skips it
  // Returns amount of data read
  DataBlock::size_t read(DataBlock::data_t *data, DataBlock::size_t length,
                         bool block = true);

  //------------------------------------------------------------------------
  // Destructor
  ~DataQueue();
};

//==========================================================================
// Poolable thread class - use in Thread Pool below
// (you still have to subclass this to implement run(), though)

template<class T> class ThreadPool;  //forward
class PoolThread;                    //forward

// Interface allowing replacement into the pool without knowing how the
// pool template is instantiated (see below)
class IPoolReplacer
{
public:
  virtual void replace(PoolThread *t) = 0;
  virtual ~IPoolReplacer() {}  // Keeps compiler happy
};

class PoolThread: public Thread
{
public:
  IPoolReplacer& replacer;
  Condition in_use;
  bool dying;

  //------------------------------------------------------------------------
  // Constructor - automatically starts thread in !in_use state
  PoolThread(IPoolReplacer& _replacer);

  //------------------------------------------------------------------------
  // Start - overriden for pooling
  bool start() override;

  //------------------------------------------------------------------------
  // Kick thread after being given parameters (members of subclasses)
  void kick();

  //------------------------------------------------------------------------
  // Request it to die.  If 'wait' is set, waits for thread to exit
  virtual void die(bool wait = false);
};

//==========================================================================
// Template for pool replacers
// This piece of evil allows a 'PoolThread' to replace itself in the pool
// knowing only that it is a subclass of PoolThread, not what the
// pool template is instantiated to (the actual subclass)
template<class T> class PoolReplacer: public IPoolReplacer
{
private:
  ThreadPool<T>& pool;

public:
  PoolReplacer(ThreadPool<T>& _pool): pool(_pool) {}
  virtual void replace(PoolThread *t) { pool.replace(dynamic_cast<T *>(t)); }
};

//==========================================================================
// Thread Pool template
template<class T> class ThreadPool
{
private:
  mutex mymutex;
  unsigned int min_spares;
  unsigned int max_threads;
  bool realtime;
  list<T *> spares;   // Spare waiting threads
  list<T *> actives;  // Active threads
  PoolReplacer<T> replacer;
  bool shutting_down;

  // Add another spare thread to the pool
  void add_spare()
  {
    auto t = new T(replacer);
    // Set realtime if requested
    if (realtime)
      t->set_priority(10, true);
    spares.push_back(move(t));
  }

  // Fill pool to provide 'min' spares, but not more than 'max' in all
  // unless max is 0
  // Always creates at least one spare
  void fill()
  {
    while (spares.size() < (min_spares?min_spares:1)
           && (!max_threads || spares.size()+actives.size() < max_threads))
      add_spare();
  }

public:
  //------------------------------------------------------------------------
  // Constructor
  // min-max is range of number of threads the pool will keep alive
  ThreadPool(unsigned int _min_spares = 1, unsigned int _max_threads = 10,
             bool _realtime = false):
    min_spares{_min_spares}, max_threads{_max_threads}, realtime{_realtime},
    replacer{*this}, shutting_down{false}
  {
    fill();
  }

  //------------------------------------------------------------------------
  // Check if any threads available
  bool available()
  {
    unique_lock<mutex> lock(mymutex);
    return !spares.empty();
  }

  //------------------------------------------------------------------------
  // Check if any threads active
  bool active()
  {
    unique_lock<mutex> lock(mymutex);
    return !actives.empty();
  }

  //------------------------------------------------------------------------
  // Get active threads
  list<T *>& get_actives()
  {
    return actives;
  }

  //------------------------------------------------------------------------
  // Remove a thread from the pool
  // nullptr if none available
  T *remove()
  {
    if (shutting_down)
      return nullptr;

    unique_lock<mutex> lock(mymutex);
    fill();  // Try and make spares
    if (!spares.size())
      return nullptr;

    auto t = spares.front();
    spares.pop_front();
    actives.push_back(t);
    return t;
  }

  //------------------------------------------------------------------------
  // Wait for a thread from the pool
  // Guarantees to return one eventually
  T *wait()
  {
    // Busy wait if nothing available
    // !!! Should provide a CondVar to allow us to block here
    for(;;)
    {
      auto t = remove();
      if (t)
        return t;
      this_thread::sleep_for(chrono::milliseconds{10});
    }
  }

  //------------------------------------------------------------------------
  // Replace a thread in the pool
  void replace(T *t)
  {
    if (shutting_down) return;  // Just lose it
    unique_lock<mutex> lock(mymutex);

    // Remove from actives
    actives.remove(t);

    // Empty spares above minimum amount, allowing for the one we're about
    // to replace
    while (spares.size() && spares.size() > min_spares-1)
    {
      auto ts = spares.back();  // Kill last one entered
      spares.pop_back();
      ts->die(true);  // Wait for it to die before deletion
      delete ts;
    }

    // Add this back to spares after deletion - can't delete the one we're
    // running in!  (Note that this means that there will always be one
    // spare even if min_spares=0)
    spares.push_back(t);
  }

  //------------------------------------------------------------------------
  // Shut down pool - cancel and stop all threads
  void shutdown()
  {
    if (!shutting_down)
    {
      shutting_down = true;
      unique_lock<mutex> lock(mymutex);

      // Kill actives
      for (auto t : actives)
      {
        // Ask it nicely
        t->die();

        // Wait for a bit while it's still running
        for (auto i = 0; i < 5; ++i)
        {
          if (!*t) break;
          this_thread::sleep_for(chrono::milliseconds{10});
        }

        // Then kill it with cancel if it hasn't already died
        delete t;
      }
      actives.clear();

      // Kill spares - same again
      for (auto t : spares)
      {
        t->die();
        for (auto i = 0; i < 5; ++i)
        {
          if (!*t) break;
          this_thread::sleep_for(chrono::milliseconds{10});
        }
        delete t;
      }

      spares.clear();
    }
  }

  //------------------------------------------------------------------------
  // Destructor
  // Kill all threads
  ~ThreadPool()
  {
    shutdown();
  }
};

//==========================================================================
// Task
// A function object that can be run on a thread in a managed way by a
// TaskThread
class Task
{
private:
  Condition running;

public:
  //------------------------------------------------------------------------
  // Constructor
  Task():
    running{true}
  {}

  //------------------------------------------------------------------------
  // Pure virtual run method - called when the TaskThread is constructed
  // Implementations of this should frequently check is_running() to see
  // if the thread has been asked to shutdown
  virtual void run() = 0;

  //------------------------------------------------------------------------
  // Check that thread hasn't been asked to shutdown
  bool is_running() const
  {
    return running;
  }

  //------------------------------------------------------------------------
  // Sleep for a given period, or until thread told to stop
  template< class Rep, class Period >
    void sleep_for(const std::chrono::duration<Rep,Period>& time)
  {
    running.wait_for(time, false);
  }

  //------------------------------------------------------------------------
  // Sleep until a given time, or until thread told to stop
  template <class Clock, class Duration>
    void sleep_until(const std::chrono::time_point<Clock, Duration>& time)
  {
    running.wait_until(time, false);
  }

  //------------------------------------------------------------------------
  // Virtual shutdown method
  // Override if anything special is needed to unblock something in the run()
  // method (e.g. closing a socket to unblock a recv)
  virtual void shutdown()
  {
    running.signal(false);
  }

  //------------------------------------------------------------------------
  // Get a signal to send at shutdown
  virtual int shutdown_signal() { return 0; }

  //------------------------------------------------------------------------
  // Virtual destructor
  virtual ~Task() {}
};

//==========================================================================
// Task Thread
// A class that runs a task in a thread
// !Note: takes ownership of task
template<class T>
class TaskThread
{
private:
  unique_ptr<T> task;

  class Thread: public MT::Thread
  {
  private:
    Task *task;

  public:
    Thread (Task *_task):
      task{_task}
    {
    }
    void run() override
    {
      task->run();
    }
  } thread;

public:
  //------------------------------------------------------------------------
  // Constructor
  // Takes ownership of passed in Task
  TaskThread(T *_task):
    task{_task}, thread{_task}
  {
    thread.start();
  }

  //------------------------------------------------------------------------
  // Arrow operator to get to the task
  T *operator->()
  {
    return task.get();
  }

  //------------------------------------------------------------------------
  // Kill thread (or send another signal)
  void kill(int signal = SIGTERM)
  {
    thread.kill(signal);
  }

  //------------------------------------------------------------------------
  // Destructor
  ~TaskThread()
  {
    task->shutdown();
    if (int signal = task->shutdown_signal())
      thread.kill(signal);
    thread.join();
  }
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_MT_H

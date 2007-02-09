//==========================================================================
// ObTools::MT: ot-mt.h
//
// Public definitions for ObTools::MT
// C++ wrapping of pthreads and useful multithreading bits - queues, etc.
// 
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#ifndef __OBTOOLS_MT_H
#define __OBTOOLS_MT_H

#include <pthread.h>
#include <queue>
#include <list>

namespace ObTools { namespace MT { 

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Abstract Thread class
class Thread
{
public:
  bool running;          // True if thread created and thread handle valid
  pthread_t thread;      // thread handle - not necessarily an integer type!
  Thread *self;          // Used as start argument; must outlive thread

  //--------------------------------------------------------------------------
  // Default Constructor - does nothing
  Thread(): running(false) {}

  //--------------------------------------------------------------------------
  // Virtual run routine - called once thread has started by start()
  // Do not call directly!
  virtual void run() = 0;

  //--------------------------------------------------------------------------
  // Start - note separate from default constructor to allow you time to create
  // parameters in subclass constructors
  // Returns whether successful
  bool start();

  //--------------------------------------------------------------------------
  // Set priority - higher numbers have higher priority
  // realtime sets SCHED_RR if set
  // Whether successful (may fail if realtime requested when not root)
  bool set_priority(int priority, bool realtime=false);

  //--------------------------------------------------------------------------
  // Join - caller waits for this thread to end
  void join() { if (running) pthread_join(thread, NULL); }

  //--------------------------------------------------------------------------
  // Detach - let it die silently when it ends, we aren't going to join with it
  void detach() { if (running) pthread_detach(thread); }

  //--------------------------------------------------------------------------
  // Cancel - ask it to stop
  void cancel();

  //--------------------------------------------------------------------------
  // Allow Cancel - allow it to stop if asked
  void allow_cancel() { pthread_testcancel(); }

  //--------------------------------------------------------------------------
  // Test if it has stopped
  bool operator!() { return !running; }

  //--------------------------------------------------------------------------
  // Destructor - ask it to cancel if started
  virtual ~Thread();

  //--------------------------------------------------------------------------
  // Portable sleep()/usleep() functions - this is a convenient place to
  // put these, since thread implementations often need to do this

  // Sleep for given number of seconds
  static void sleep(int secs);

  // Sleep for given number of microseconds
  static void usleep(int usecs);
};

//==========================================================================
// Mutex class
// Note:  These mutexes are NOT recursive - PTHREAD_MUTEX_RECURSIVE is not
// portable.  Use RecursiveMutex if you need to lock it multiple times within 
// the same thread 
class Mutex
{
private:
  pthread_mutex_t mutex;
  friend class BasicCondVar;
  friend class Condition;   // So they can use my mutex for cond_wait

public:
  //--------------------------------------------------------------------------
  // Default Constructor - initialises mutex 
  Mutex() { pthread_mutex_init(&mutex, NULL); }
  
  //--------------------------------------------------------------------------
  // Destructor - destroys mutex
  ~Mutex() { pthread_mutex_destroy(&mutex); }

  //--------------------------------------------------------------------------
  // Lock the mutex, block if locked
  void lock() { pthread_mutex_lock(&mutex); }

  //--------------------------------------------------------------------------
  // Try to lock the mutex, return false if it fails
  bool trylock() { return pthread_mutex_trylock(&mutex)==0; }

  //--------------------------------------------------------------------------
  // Unlock the mutex
  void unlock() { pthread_mutex_unlock(&mutex); }
};

//==========================================================================
// Lock class - use to hold mutexes while in scope, thereby giving
// correct exception behaviour
// e.g.
// {
//   Lock lock(mutex);
//     ... do critical section stuff
// } // lock goes out of scope and unlocks automatically

class Lock
{
private:
  Mutex& mutex;

  //--------------------------------------------------------------------------
  // Block copying and assignment;  that way lies insanity
  // These are here only to be private, and hence inaccessible
  // Initialisation of mutex is just to keep the compiler happy
  Lock(const Lock& x): mutex(x.mutex) { }
  void operator=(const Lock&) { }

public:
  //--------------------------------------------------------------------------
  // Constructor - locks mutex
  Lock(Mutex &m): mutex(m) { mutex.lock(); }
  
  //--------------------------------------------------------------------------
  // Destructor - unlocks mutex
  ~Lock() { mutex.unlock(); }
};

//==========================================================================
// BasicCondVar class - basic condition variable like pthread_condvar
// Uses caller's mutex to allow mutex to extend beyond the wait() call

// NB All wait(), signal() and broadcast() calls must be made with 
// mutex locked

// Cancellation handler to unlock a mutex
extern "C" void _unlock_mutex(void *m);

class BasicCondVar
{
private:
  pthread_cond_t cv;

public:
  //--------------------------------------------------------------------------
  // Default Constructor - initialises condvar
  BasicCondVar()
  { 
    pthread_cond_init(&cv, NULL); 
  }
  
  //--------------------------------------------------------------------------
  // Destructor - destroys condvar 
  ~BasicCondVar() 
  { 
    pthread_cond_destroy(&cv); 
  }

  //--------------------------------------------------------------------------
  // Wait on the condvar using caller's mutex
  // Note mutex should already be locked, will be unlocked during wait
  // and then relocked again
  void wait(Mutex& mutex)
  { 
    // Handle unlock of mutex should cond_wait be cancelled
    pthread_cleanup_push(_unlock_mutex, &mutex.mutex);
    pthread_cond_wait(&cv, &mutex.mutex);
    pthread_cleanup_pop(0);
  }

  //--------------------------------------------------------------------------
  // Signal the condition 
  void signal()
  { 
    pthread_cond_signal(&cv);
  }

  //--------------------------------------------------------------------------
  // Broadcast the condition to multiple waiters
  void broadcast()
  { 
    pthread_cond_broadcast(&cv);
  }
};

//==========================================================================
// Condition variable class (boolean condition)
// Implements safe signalling on a simple boolean variable
class Condition
{
private:
  volatile bool flag;
  pthread_cond_t cv;
  Mutex mutex;

public:
  //--------------------------------------------------------------------------
  // Default Constructor - initialises condvar and mutex
  Condition(bool initial=false): flag(initial)
  { 
    pthread_cond_init(&cv, NULL); 
  }
  
  //--------------------------------------------------------------------------
  // Destructor - destroys condvar and mutex
  ~Condition() 
  { 
    pthread_cond_destroy(&cv); 
  }

  //--------------------------------------------------------------------------
  // Wait on the condition to become as desired
  void wait(bool desired=true)
  { 
    Lock lock(mutex);       // Use locks to ensure exception safety

    // But also, because C++ exceptions and thread cancellation isn't 
    // integrated yet (grr), make sure that if the cond_wait is cancelled
    // the mutex gets unlocked again
    pthread_cleanup_push(_unlock_mutex, &mutex.mutex);

    while (flag!=desired) 
      pthread_cond_wait(&cv, &mutex.mutex);

    pthread_cleanup_pop(0);
  }

  //--------------------------------------------------------------------------
  // Signal the condition to become as stated
  void signal(bool value=true)
  { 
    Lock lock(mutex);
    if (flag != value)
    {
      flag = value;
      pthread_cond_signal(&cv);
    }
  }

  //--------------------------------------------------------------------------
  // Broadcast the condition to become as stated (multiple waiters)
  void broadcast(bool value=true)
  { 
    Lock lock(mutex);
    if (flag != value)
    {
      flag = value;
      pthread_cond_broadcast(&cv);
    }
  }

  //--------------------------------------------------------------------------
  // Clear the flag without signalling.
  // This is used with the default 'true' parameters of wait/signal/broadcast
  // to implement 'rising-edge-only' synchronisation.  If you want both edges
  // synchronised, manually wait/signal/broadcast for 'false' too
  void clear() { flag = false; }
};

//==========================================================================
// Recursive Mutex class
// You can use these recursively - i.e. hold them locked in one function 
// while you call another one which locks it again
class RMutex
{
private:
  Mutex mutex;
  Condition available;
  volatile int count;
  volatile bool owned;
  pthread_t owner;

public:
  //--------------------------------------------------------------------------
  // Default Constructor - initialises mutex 
  RMutex(): mutex(), available(true), count(0), owned(false) {}
  
  //--------------------------------------------------------------------------
  // Lock the mutex, block if locked
  void lock() 
  {
    pthread_t self = pthread_self();
    for(;;) // May loop if we lose the race after wait()
    {
      // Wait for it to be available
      if (owned && !pthread_equal(owner, self)) available.wait();

      // Take it
      Lock lock(mutex);

      // Make sure we're still OK
      if (!count || (owned && pthread_equal(owner,self)))  
      {
	++count;
	owned=true;
	owner=self;
	available.clear();
	return;
      }
    }
  }

  //--------------------------------------------------------------------------
  // Unlock the mutex
  void unlock()
  { 
    Lock lock(mutex);

    if (!--count) 
    {
      owned = false;
      available.signal();    // Wake up waiters
    }
  }
};

//==========================================================================
// Recursive Lock class - as Lock, but usable recursively (see Lock for detail)
class RLock
{
private:
  RMutex& mutex;
  RLock(const RLock& x): mutex(x.mutex) { }
  void operator=(const RLock&) { }

public:
  RLock(RMutex &m): mutex(m) { mutex.lock(); }
  ~RLock() { mutex.unlock(); }
};

//==========================================================================
// Multiple readers/single writer Mutex class
// Like pthread_rwlock, but provides ability for recursive use by writer
// that would otherwise deadlock
// Implements writer priority
class RWMutex
{
private:
  // Internal mutex, only locked transiently
  Mutex mutex;

  // Count and condition for readers
  volatile int readers_active;
  BasicCondVar no_readers;

  // Count and condition for writers
  volatile int writers_waiting;
  volatile bool writer_active;
  BasicCondVar no_writer;

public:
  //--------------------------------------------------------------------------
  // Default Constructor - initialises mutex 
  RWMutex(): 
    mutex(), readers_active(0), no_readers(),
    writers_waiting(0), writer_active(false), no_writer()
    {}
  
  //--------------------------------------------------------------------------
  // Lock the mutex for read
  void lock_reader() 
  {
    Lock lock(mutex);

    // Wait until there are no writers, either queued or active
    while (writers_waiting || writer_active) no_writer.wait(mutex);

    // Claim it and block writers
    readers_active++;
  }

  //--------------------------------------------------------------------------
  // Unlock the mutex for read
  void unlock_reader()
  { 
    Lock lock(mutex);

    // Wake up all writers so they can proceed through the no-readers gate
    if (!--readers_active) no_readers.broadcast();  
  }

  //--------------------------------------------------------------------------
  // Lock the mutex for write
  void lock_writer() 
  {
    Lock lock(mutex);
    
    // Show there are writers waiting, to ensure priority
    writers_waiting++;

    // Wait until there are no readers
    while (readers_active) no_readers.wait(mutex);

    // Wait until there are no other writers using it
    while (writer_active) no_writer.wait(mutex);
    
    // We're not waiting any more, we're active
    writers_waiting--;
    writer_active = true;
  }  

  //--------------------------------------------------------------------------
  // Unlock the mutex for write
  void unlock_writer()
  { 
    Lock lock(mutex);
    writer_active = false;                  // Let other writers go

    // Wake up all writers and readers - note reader may be ahead of a
    // writer in the queue, and we must ensure the writer gets woken up
    no_writer.broadcast();
  }
};

//==========================================================================
// Reader/Writer Lock class - as Lock, but claiming reader side of an RWMutex
class RWReadLock
{
private:
  RWMutex& mutex;
  RWReadLock(const RWReadLock& x): mutex(x.mutex) { }
  void operator=(const RWReadLock&) { }

public:
  RWReadLock(RWMutex &m): mutex(m) { mutex.lock_reader(); }
  ~RWReadLock() { mutex.unlock_reader(); }
};

//==========================================================================
// Ditto, but for writer side
class RWWriteLock
{
private:
  RWMutex& mutex;
  RWWriteLock(const RWWriteLock& x): mutex(x.mutex) { }
  void operator=(const RWWriteLock&) { }

public:
  RWWriteLock(RWMutex &m): mutex(m) { mutex.lock_writer(); }
  ~RWWriteLock() { mutex.unlock_writer(); }
};

//==========================================================================
// Queue template - send any message between threads safety
template<class T> class Queue
{
private:
  queue<T> q;
  Mutex mutex;
  Condition available;

public:
  //--------------------------------------------------------------------------
  // Constructor 
  Queue() {}

  //--------------------------------------------------------------------------
  // Send a message (never blocks)
  void send(T msg)
  {
    Lock lock(mutex);
    q.push(msg);
    available.signal(); 
  }

  //--------------------------------------------------------------------------
  // See if any message is available before potentially blocking on wait()
  bool poll() { return (!q.empty()); }

  //--------------------------------------------------------------------------
  // Wait to receive a message (blocking)
  T wait()
  {
    // Can loop if someone else gets it
    for(;;)
    {
      // Wait for availability signal
      available.wait();

      // May be something if we get there first - lock it and check again
      Lock lock(mutex);
      if (!q.empty()) 
      {
	T msg = q.front();
	q.pop();

	// Clear flag if now empty
	if (q.empty()) available.clear();

	return msg;
      }
    }
  }
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
  virtual void replace(PoolThread *t)=0;
};

class PoolThread: public Thread
{
public:
  IPoolReplacer& replacer;
  Condition in_use;
  bool dying;

  //--------------------------------------------------------------------------
  // Constructor - automatically starts thread in !in_use state
  PoolThread(IPoolReplacer& _replacer);

  //--------------------------------------------------------------------------
  // Kick thread after being given parameters (members of subclasses)
  void kick();

  //--------------------------------------------------------------------------
  // Request it to die.  If 'wait' is set, waits for thread to exit
  void die(bool wait=false);

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
  Mutex mutex;
  unsigned int min_spares;
  unsigned int max_threads;
  list<T *> threads;  // All of them
  list<T *> spares;   // Just the spare ones
  PoolReplacer<T> replacer;
  bool shutting_down;

  // Add another spare thread to the pool
  void add_spare()
  {
    T *t = new T(replacer);
    threads.push_back(t);
    spares.push_back(t);
  }

  // Fill pool to provide 'min' spares, but not more than 'max' in all
  void fill() 
  { 
    while (spares.size() < min_spares && threads.size() < max_threads) 
      add_spare(); 
  }

public:
  //--------------------------------------------------------------------------
  // Constructor
  // min-max is range of number of threads the pool will keep alive
  // min must be at least 1
  ThreadPool(unsigned int min=1, unsigned int max=10): 
    min_spares(min), max_threads(max), replacer(*this), shutting_down(false)
  { fill(); }

  //--------------------------------------------------------------------------
  // Remove a thread from the pool
  // 0 if none available
  T *remove()
  {
    if (shutting_down) return 0;
    Lock lock(mutex);
    fill();  // Try and make spares
    if (!spares.size()) return 0; 

    T *t = spares.front();
    spares.pop_front();
    return t;
  }

  //--------------------------------------------------------------------------
  // Replace a thread in the pool
  void replace(T *t)
  {
    if (shutting_down) return;  // Just lose it
    Lock lock(mutex);

    // Empty spares above minimum amount, allowing for the one we're about
    // to replace
    while (spares.size() && spares.size() > min_spares-1)
    {
      T *ts = spares.back();  // Kill last one entered
      spares.pop_back();
      threads.remove(ts);
      ts->die(true);  // Wait for it to die before deletion
      delete ts;
    }

    // Add this back to spares after deletion - can't delete the one we're
    // running in!  (Note that this means that there will always be one
    // spare even if min_spares=0)
    spares.push_back(t);
  }

  //--------------------------------------------------------------------------
  // Shut down pool - cancel and stop all threads
  void shutdown()
  {
    if (!shutting_down)
    {
      shutting_down = true;
      Lock lock(mutex);
      for(typename list<T *>::iterator p=threads.begin(); p!=threads.end();p++)
      {
	T* t = *p;
	// Ask it nicely
	t->die();

	// Wait for a bit while it's still running
	for(int i=0; i<5; i++)
	{
	  if (!*t) break;
	  Thread::usleep(10000);
	}

	// Then kill it with cancel if it hasn't already died
	delete *p;
      }
      threads.clear();
      spares.clear();
    }
  }

  //--------------------------------------------------------------------------
  // Destructor
  // Kill all threads 
  ~ThreadPool()
  {
    shutdown();
  }
};


//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_MT_H




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
  pthread_t thread;
  Thread *self;          // Used as start argument; must outlive thread

  //--------------------------------------------------------------------------
  // Default Constructor - does nothing
  Thread(): thread(0) {}

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
  void join() { if (thread) pthread_join(thread, NULL); }

  //--------------------------------------------------------------------------
  // Detach - let it die silently when it ends, we aren't going to join with it
  void detach() { if (thread) pthread_detach(thread); }

  //--------------------------------------------------------------------------
  // Cancel - ask it to stop
  void cancel();

  //--------------------------------------------------------------------------
  // Allow Cancel - allow it to stop if asked
  void allow_cancel() { pthread_testcancel(); }

  //--------------------------------------------------------------------------
  // Test if it has stopped
  bool operator!() { return !thread; }

  //--------------------------------------------------------------------------
  // Destructor - ask it to cancel if started
  virtual ~Thread();
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
  friend class Condition;   // So it can use my mutex for cond_wait

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
  void operator=(const Lock& x) { }

public:
  //--------------------------------------------------------------------------
  // Constructor - locks mutex
  Lock(Mutex &m): mutex(m) { mutex.lock(); }
  
  //--------------------------------------------------------------------------
  // Destructor - unlocks mutex
  ~Lock() { mutex.unlock(); }
};

//==========================================================================
// Condition variable class (boolean condition)

// Cancellation handler to unlock a mutex
extern "C" void _unlock_mutex(void *m);

class Condition
{
private:
  bool flag;
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
  int count;
  pthread_t owner;

public:
  //--------------------------------------------------------------------------
  // Default Constructor - initialises mutex 
  RMutex(): mutex(), available(true), count(0), owner(0) {}
  
  //--------------------------------------------------------------------------
  // Lock the mutex, block if locked
  void lock() 
  {
    pthread_t self = pthread_self();
    for(;;) // May loop if we lose the race after wait()
    {
      // Wait for it to be available
      if (owner!=self) available.wait();

      // Take it
      Lock lock(mutex);

      if (!count || owner==self)  // Make sure we're still OK
      {
	++count;
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
      owner = 0;
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
  void operator=(const RLock& x) { }

public:
  RLock(RMutex &m): mutex(m) { mutex.lock(); }
  ~RLock() { mutex.unlock(); }
};

//==========================================================================
// Claimable class
// A claimable object can be Claimed - see below
// Use as a mixin
class Claimable
{
private:
  Mutex mutex;
  int claims;

public:
  //--------------------------------------------------------------------------
  // Default Constructor 
  Claimable(): mutex(), claims(0) {}

  //--------------------------------------------------------------------------
  // Stake a claim
  void claim() { Lock lock(mutex); claims++; }

  //--------------------------------------------------------------------------
  // Release a claim
  void release() { Lock lock(mutex); claims--; }

  //--------------------------------------------------------------------------
  // Check whether claimed
  // Note:  Locking here ensures we can't get in while another thread is
  // inside a claim or release method
  bool is_claimed() { Lock lock(mutex); return claims!=0; }
};

//==========================================================================
// Claim class
// Temporary claim on a Claimable object
// Note that a 'claim' is not a 'lock' - it is not exclusive.  The idea
// here is to state that you are interested in the claimed object, and 
// would prefer it didn't get deleted under you.  Use this where multiple
// threads need to use an object which might get deleted - each user stakes
// a claim while it needs the object, and the reaper avoids deleting any
// which are claimed.
class Claim
{
private:
  Claimable &target;

public:
  //--------------------------------------------------------------------------
  // Constructor - claim the target
  Claim(Claimable& _target): target(_target) { target.claim(); }

  //--------------------------------------------------------------------------
  // Destructor - release the target
  ~Claim() { target.release(); }
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

  //--------------------------------------------------------------------------
  // Constructor - automatically starts thread in !in_use state
  PoolThread(IPoolReplacer& _replacer);

  //--------------------------------------------------------------------------
  // Kick thread after being given parameters (members of subclasses)
  void kick();
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
    min_spares(min), max_threads(max), replacer(*this)
  { fill(); }

  //--------------------------------------------------------------------------
  // Remove a thread from the pool
  // 0 if none available
  T *remove()
  {
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
    Lock lock(mutex);
    spares.push_back(t);

    // Empty spares above minimum amount
    while (spares.size() > min_spares)
    {
      T *t = spares.back();  // Kill last one entered
      spares.pop_back();

      // Remove from arbitrary position in threads list
      typename list<T *>::iterator p = find(threads.begin(), threads.end(), t);
      if (p!=threads.end()) threads.erase(p);
      
      delete t;
    }
  }

  //--------------------------------------------------------------------------
  // Destructor
  // Kill all threads 
  ~ThreadPool()
  {
    Lock lock(mutex);
    for(typename list<T *>::iterator p=threads.begin(); p!=threads.end(); p++)
      delete *p;
  }
};


//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_MT_H




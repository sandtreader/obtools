//==========================================================================
// ObTools::Action: ot-gen.h
//
// Public definitions for ObTools::Action
// Generic action management classes
// Handles actions asynchronously but sequentially for potentially multiple
// handlers
//
// Copyright (c) 2013 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_ACTION_H
#define __OBTOOLS_ACTION_H

#include <map>
#include <vector>

#include "ot-gen.h"
#include "ot-mt.h"

namespace ObTools { namespace Action {

using namespace std;

//==========================================================================
// Action
// T is an enumeration specifying a list of action types for this
template<class T>
class Action
{
public:
  //------------------------------------------------------------------------
  // Type of the action
  virtual T get_type() const = 0;

  //------------------------------------------------------------------------
  // Virtual destructor
  virtual ~Action() {}
};

//==========================================================================
// Action Manager
// Manages queue for a list of action types
// T is an enumeration listing the action types
template<class T>
class Manager
{
public:
  //------------------------------------------------------------------------
  // Abstract Handler class for action
  class Handler
  {
  public:
    //----------------------------------------------------------------------
    // Handle the action
    virtual void handle(const Action<T>& action) = 0;

    //----------------------------------------------------------------------
    // Virtual destructor
    virtual ~Handler() {}
  };

private:
  map<T, vector<Handler *> > handlers;
  MT::Mutex handlers_mutex;

  MT::MQueue<Gen::SharedPointer<Action<T> > > actions;

  class ActionThread: public MT::Thread
  {
  private:
    MT::Condition condition;
    Gen::SharedPointer<Action<T> > action;
    Handler *handler;
    MT::Mutex mutex;

  public:
    //----------------------------------------------------------------------
    // Constructor
    ActionThread():
      handler(0)
    {
    }

    //----------------------------------------------------------------------
    // Run routine
    virtual void run()
    {
      while (running)
      {
        condition.wait(false);

        Gen::SharedPointer<Action<T> > a;
        Handler *h;
        {
          MT::Lock lock(mutex);
          a = action;
          h = handler;
        }
        if (a.get())
        {
          h->handle(*a);
          {
            MT::Lock lock(mutex);
            action.reset();
            handler = 0;
          }
          condition.signal(true);
        }
      }
    }

    //---------------------------------------------------------------------
    // Set the action to work upon
    void set_action(Gen::SharedPointer<Action<T> > new_action,
                    Handler *new_handler)
    {
      {
        MT::Lock lock(mutex);
        action = new_action;
        handler = new_handler;
      }
      condition.signal(false);
    }

    //---------------------------------------------------------------------
    // Wait on action being used
    void wait()
    {
      condition.wait(true);
    }
  };

  vector<Gen::SharedPointer<ActionThread> > threads;

  class WorkerThread: public MT::Thread
  {
  private:
    Manager& manager;

  public:
    //----------------------------------------------------------------------
    // Constructor
    WorkerThread(Manager& _manager):
      manager(_manager)
    {
    }

    //----------------------------------------------------------------------
    // Run routine
    virtual void run()
    {
      while (running)
        manager.next_action();
    }
  } worker_thread;

  friend class WorkerThread;

  //------------------------------------------------------------------------
  // Handle next action on queue
  // Returns whether more to process
  void next_action()
  {
    Gen::SharedPointer<Action<T> > action = actions.wait();

    if (!action.get())
      return;

    vector<Gen::SharedPointer<ActionThread > > active;
    {
      MT::Lock lock(handlers_mutex);
      typename map<T, vector<Handler *> >::iterator
               h = handlers.find(action->get_type());
      if (h != handlers.end())
      {
        unsigned i(0);
        for (typename vector<Handler *>::iterator
             it = h->second.begin(); it != h->second.end(); ++it)
        {
          if (i++ >= threads.size())
          {
            Gen::SharedPointer<ActionThread> at(new ActionThread());
            at->start();
            threads.push_back(at);
          }
          threads.back()->set_action(action, *it);
          active.push_back(threads.back());
        }
      }
    }

    // Wait for all active threads to finish
    for (typename std::vector<Gen::SharedPointer<ActionThread > >::iterator
         it = active.begin(); it != active.end(); ++it)
    {
      (*it)->wait();
    }
  }

public:
  //------------------------------------------------------------------------
  // Constructor
  Manager():
    worker_thread(*this)
  {
    worker_thread.start();
  }

  //------------------------------------------------------------------------
  // Register a handler
  void add_handler(T type, Handler &handler)
  {
    MT::Lock lock(handlers_mutex);
    handlers[type].push_back(&handler);
  }

  //------------------------------------------------------------------------
  // Queue an action
  void queue(const Gen::SharedPointer<Action<T> >& action)
  {
    actions.send(action);
  }

  //------------------------------------------------------------------------
  // Get configuration
  map<T, vector<Handler *> > get_config()
  {
    MT::Lock lock(handlers_mutex);
    return handlers;
  }

  //------------------------------------------------------------------------
  // Destructor
  ~Manager()
  {
    // Politely ask worker thread to stop
    worker_thread.running = false;
    actions.send(Gen::SharedPointer<Action<T> >(0));
    worker_thread.join();
    // Politely ask action threads to stop
    // threads is normally only accessed by worker_thread, but that's gone now
    // so we are safe to use without a lock
    for (typename vector<Gen::SharedPointer<ActionThread> >::iterator
         it = threads.begin(); it != threads.end(); ++it)
    {
      (*it)->running = false;
      (*it)->set_action(0, 0);
      (*it)->join();
    }
  }
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_ACTION_H

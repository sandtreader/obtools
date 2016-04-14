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

  MT::MQueue<Action<T> *> actions;

  class ActionTask: public MT::Task
  {
  private:
    MT::Condition condition;
    Action<T> *action;
    Handler *handler;
    MT::Mutex mutex;

  public:
    //----------------------------------------------------------------------
    // Constructor
    ActionTask():
      action(0), handler(0)
    {
    }

    //----------------------------------------------------------------------
    // Run routine
    virtual void run()
    {
      while (is_running())
      {
        condition.wait(false);

        Action<T> *a(0);
        Handler *h(0);
        {
          MT::Lock lock(mutex);
          a = action;
          h = handler;
        }
        if (a)
        {
          h->handle(*a);
          {
            MT::Lock lock(mutex);
            action = 0;
            handler = 0;
          }
          condition.signal(true);
        }
      }
    }

    //---------------------------------------------------------------------
    // Virtual shutdown
    void shutdown()
    {
      MT::Task::shutdown();
      // Call with empty action to wake
      set_action(0, 0);
    }

    //---------------------------------------------------------------------
    // Set the action to work upon
    void set_action(Action<T> *new_action, Handler *new_handler)
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

  vector<Gen::SharedPointer<MT::TaskThread<ActionTask> > > threads;

  class WorkerTask: public MT::Task
  {
  private:
    Manager& manager;

  public:
    //----------------------------------------------------------------------
    // Constructor
    WorkerTask(Manager& _manager):
      manager(_manager)
    {
    }

    //----------------------------------------------------------------------
    // Run routine
    virtual void run()
    {
      while (is_running())
        manager.next_action();
    }

    //----------------------------------------------------------------------
    // Virtual shutdown
    virtual void shutdown()
    {
      MT::Task::shutdown();
      // Wake up thread with empty action
      manager.queue(0);
    }
  };

  MT::TaskThread<WorkerTask> worker_thread;

  friend class WorkerThread;

  //------------------------------------------------------------------------
  // Handle next action on queue
  // Returns whether more to process
  void next_action()
  {
    auto_ptr<Action<T> > action(actions.wait());

    if (!action.get())
      return;

    vector<Gen::SharedPointer<MT::TaskThread<ActionTask > > > active;
    {
      MT::Lock lock(handlers_mutex);
      typename map<T, vector<Handler *> >::iterator
               h = handlers.find(action->get_type());
      if (h != handlers.end())
      {
        unsigned i(0);
        for (typename vector<Handler *>::iterator
             it = h->second.begin(); it != h->second.end(); ++it, ++i)
        {
          if (i >= threads.size())
          {
            threads.push_back(new MT::TaskThread<ActionTask>(new ActionTask()));
          }
          (*threads[i])->set_action(action.get(), *it);
          active.push_back(threads[i]);
        }
      }
    }

    // Wait for all active threads to finish
    for (typename std::vector<Gen::SharedPointer<
                              MT::TaskThread<ActionTask > > >::iterator
         it = active.begin(); it != active.end(); ++it)
    {
      (**it)->wait();
    }
  }

public:
  //------------------------------------------------------------------------
  // Constructor
  Manager():
    worker_thread(new WorkerTask(*this))
  {
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
  void queue(Action<T> *action)
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
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_ACTION_H

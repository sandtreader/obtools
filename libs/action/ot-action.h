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
  // Compare actions for duplication test
  // By default just compares type
  virtual bool operator==(const Action<T>& o) const
  { return get_type() == o.get_type(); }

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

  MT::Queue<Action<T> *> actions;
  int queue_limit = 0;
  bool dedup = false;

  class ActionTask: public MT::Task
  {
  private:
    MT::Condition condition;
    Action<T> *action = nullptr;
    Handler *handler = nullptr;

  public:
    //----------------------------------------------------------------------
    // Run routine
    void run() override
    {
      auto quit = false;
      while (!quit)
      {
        condition.wait(true);

        quit = !(handler && action);
        if (!quit)
          handler->handle(*action);

        condition.signal(false);
      }
    }

    //---------------------------------------------------------------------
    // Set the action to work upon
    void set_action(Action<T> *new_action, Handler *new_handler)
    {
      action = new_action;
      handler = new_handler;
      condition.signal(true);
    }

    //---------------------------------------------------------------------
    // Wait on action being used
    void wait()
    {
      condition.wait(false);
    }
  };

  vector<shared_ptr<MT::TaskThread<ActionTask> > > threads;

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
    void run() override
    {
      while (is_running() && manager.next_action())
        ;

      // Trigger threads shut down
      for (auto t: manager.threads)
      {
        (*t)->set_action(nullptr, nullptr);
        (*t)->wait();
      }
    }
  };

  MT::TaskThread<WorkerTask> worker_thread;

  friend class WorkerThread;

  //------------------------------------------------------------------------
  // Handle next action on queue
  // Returns whether more to process
  bool next_action()
  {
    auto action = unique_ptr<Action<T>>{actions.wait()};

    if (!action.get())
      return false;

    vector<shared_ptr<MT::TaskThread<ActionTask>>> active;
    {
      MT::Lock lock{handlers_mutex};
      auto h = handlers.find(action->get_type());
      if (h != handlers.end())
      {
        auto i = 0u;
        for (auto handler: h->second)
        {
          if (i >= threads.size())
          {
            const auto p = make_shared<MT::TaskThread<ActionTask>>(
                new ActionTask{});
            threads.push_back(p);
          }
          (*threads[i])->set_action(action.get(), handler);
          active.push_back(threads[i]);
          ++i;
        }
      }
    }

    // Wait for all active threads to finish
    for (auto t: active)
    {
      (*t)->wait();
    }

    return true;
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
  // Set limit on queue length
  void set_queue_limit(int n) { queue_limit = n; }

  //------------------------------------------------------------------------
  // Enable de-duplication of queued actions
  void enable_dedup() { dedup = true; }

  //------------------------------------------------------------------------
  // Queue Result
  enum class QueueResult
  {
    ok,
    replaced_old,
    duplicate
  };

  //------------------------------------------------------------------------
  // Queue an action
  // Returns whether any limit was applied
  QueueResult queue(Action<T> *action)
  {
    if (dedup && actions.contains_ptr(action))
      return QueueResult::duplicate;

    auto limited = false;
    if (queue_limit)
      limited = actions.limit(queue_limit-1);
    actions.send(action);
    return limited ? QueueResult::replaced_old : QueueResult::ok;
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
    // Wake up thread with empty action
    queue(nullptr);
  }
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_ACTION_H

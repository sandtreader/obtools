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
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include "ot-gen.h"

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
  map<T, vector<Handler *>> handlers;
  mutex handlers_mutex;

  queue<unique_ptr<Action<T>>> actions;
  mutex queue_mutex;
  condition_variable queue_condition;

  class ActionTask
  {
  private:
    mutex condition_mutex;
    condition_variable condition;
    Action<T> *action = nullptr;
    Handler *handler = nullptr;

  public:
    //----------------------------------------------------------------------
    // Constructor
    ActionTask():
      action(0), handler(0)
    {
    }

    //----------------------------------------------------------------------
    // Run routine
    void operator()()
    {
      auto quit = false;
      while (!quit)
      {
        cout << "a run wait" << endl;
        unique_lock<mutex> lock{condition_mutex};
        condition.wait(lock);

        quit = !(handler && action);
        if (!quit)
          handler->handle(*action);

        cout << "a run done" << endl;
        condition.notify_one();
      }
    }

    //---------------------------------------------------------------------
    // Set the action to work upon
    void set_action(Action<T> *new_action, Handler *new_handler)
    {
      unique_lock<mutex> lock{condition_mutex};
      action = new_action;
      handler = new_handler;
      cout << "a set" << endl;
      condition.notify_one();
    }

    //---------------------------------------------------------------------
    // Wait on action being used
    void wait()
    {
      cout << "a wait" << endl;
      unique_lock<mutex> lock{condition_mutex};
      condition.wait(lock);
    }
  };

  vector<thread> threads;
  vector<unique_ptr<ActionTask>> tasks;

  class WorkerTask
  {
  private:
    Manager& manager;
    bool quit;

  public:
    //----------------------------------------------------------------------
    // Constructor
    WorkerTask(Manager& _manager):
      manager(_manager), quit(false)
    {
    }

    //----------------------------------------------------------------------
    // Run routine
    void operator()()
    {
      while (!quit && manager.next_action())
        ;
    }

    //----------------------------------------------------------------------
    // Signal task to stop
    void stop()
    {
      quit = true;
    }
  } worker_task;

  thread worker_thread;

  friend class WorkerThread;

  //------------------------------------------------------------------------
  // Handle next action on queue
  // Returns whether more to process
  bool next_action()
  {
    unique_lock<mutex> lock{queue_mutex};
    queue_condition.wait(lock);

    auto& action = actions.front();
    if (!action)
      return false;

    cout << "action time: " << action.get() << endl;

    vector<ActionTask *> active;
    {
      unique_lock<mutex> lock{handlers_mutex};
      auto h = handlers.find(action->get_type());
      if (h != handlers.end())
      {
        auto i = 0u;
        for (auto it = h->second.begin(); it != h->second.end(); ++it, ++i)
        {
          if (i >= threads.size())
          {
            tasks.push_back(make_unique<ActionTask>());
            threads.push_back(thread{ref(*tasks.back())});
          }
          tasks[i]->set_action(action.get(), *it);
          active.push_back(tasks[i].get());
        }
      }
    }

    // Wait for all active threads to finish
    for (auto& it : active)
    {
      it->wait();
    }
    cout << "done action time" << endl;

    return true;
  }

public:
  //------------------------------------------------------------------------
  // Constructor
  Manager():
    worker_task(*this), worker_thread(ref(worker_task))
  {
  }

  //------------------------------------------------------------------------
  // Register a handler
  void add_handler(T type, Handler &handler)
  {
    unique_lock<mutex> lock{handlers_mutex};
    handlers[type].push_back(&handler);
  }

  //------------------------------------------------------------------------
  // Queue an action
  void push(Action<T> *action)
  {
    unique_lock<mutex> lock{queue_mutex};
    actions.push(unique_ptr<Action<T>>(action));
    queue_condition.notify_one();
  }

  //------------------------------------------------------------------------
  // Get configuration
  map<T, vector<Handler *> > get_config()
  {
    unique_lock<mutex> lock{queue_mutex};
    return handlers;
  }

  //------------------------------------------------------------------------
  // Virtual destructor
  virtual ~Manager()
  {
    // Wake up thread with empty action
    cout << "push null" << endl;
    push(nullptr);

    // Trigger threads shut down
    for (auto& it : tasks)
    {
      it->set_action(0, 0);
      it->wait();
    }

    for (auto& t : threads)
      t.join();
    worker_task.stop();
    worker_thread.join();
  }
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_ACTION_H

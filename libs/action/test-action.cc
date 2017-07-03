//==========================================================================
// ObTools::Action: test-actions.cc
//
// Test harness for action library
//
// Copyright (c) 2013 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-action.h"
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Action type enum
enum ActionType
{
  one,
  two,
};

//--------------------------------------------------------------------------
// ActionOne test action type
class ActionOne: public Action::Action<ActionType>
{
public:
  int num;

  //------------------------------------------------------------------------
  // Constructor
  ActionOne(int _num):
    num(_num)
  {}

  //------------------------------------------------------------------------
  // Get type of action
  virtual ActionType get_type() const
  {
    return one;
  }
};

//--------------------------------------------------------------------------
// Handler for ActionOne
class Handler: public Action::Manager<ActionType>::Handler
{
public:
  vector<int> nums;

  //------------------------------------------------------------------------
  // Handle action
  void handle(const Action::Action<ActionType>& action)
  {
    const ActionOne *p = dynamic_cast<const ActionOne *>(&action);
    if (!p)
      return;

    const ActionOne& one(*p);
    nums.push_back(one.num);
  }
};

//--------------------------------------------------------------------------
// Tests
TEST(ActionTest, TestRegistration)
{
  Handler handler;

  {
    Action::Manager<ActionType> manager;
    manager.add_handler(one, handler);
    manager.queue(new ActionOne(1));
    manager.queue(new ActionOne(2));
    manager.queue(new ActionOne(3));
    // Allow actions to be handled
    this_thread::sleep_for(chrono::milliseconds{100});
  }

  vector<int> expected;
  expected.push_back(1);
  expected.push_back(2);
  expected.push_back(3);
  ASSERT_EQ(expected, handler.nums);
}

TEST(ActionTest, TestQueueLimit)
{
  Handler handler;
  vector<int> expected;

  {
    vector<int> tmp;
    Action::Manager<ActionType> manager;
    manager.add_handler(one, handler);
    manager.set_queue_limit(2);
    ASSERT_EQ(Action::Manager<ActionType>::QueueResult::ok,
              manager.queue(new ActionOne(1)));
    tmp.push_back(1);
    ASSERT_EQ(Action::Manager<ActionType>::QueueResult::ok,
              manager.queue(new ActionOne(2)));
    tmp.push_back(2);
    for (auto i = 3; i < 11; ++i)
    {
      switch (manager.queue(new ActionOne(i)))
      {
        case Action::Manager<ActionType>::QueueResult::ok:
          expected.push_back(tmp.front());
        case Action::Manager<ActionType>::QueueResult::replaced_old:
          tmp.erase(tmp.begin());
          tmp.push_back(i);
          break;
      }
    }
    expected.insert(expected.end(), tmp.begin(), tmp.end());
    // Allow actions to be handled
    this_thread::sleep_for(chrono::milliseconds{100});
  }

  ASSERT_TRUE(expected.size());
  ASSERT_EQ(expected, handler.nums);
}

TEST(ActionTest, TestMultipleHandlers)
{
  Handler handler1;
  Handler handler2;

  {
    Action::Manager<ActionType> manager;
    manager.add_handler(one, handler1);
    manager.add_handler(one, handler2);
    manager.queue(new ActionOne(1));
    manager.queue(new ActionOne(2));
    manager.queue(new ActionOne(3));
    // Allow actions to be handled
    this_thread::sleep_for(chrono::milliseconds{100});
  }

  vector<int> expected;
  expected.push_back(1);
  expected.push_back(2);
  expected.push_back(3);
  EXPECT_EQ(expected, handler1.nums);
  EXPECT_EQ(expected, handler2.nums);
}


} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

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
class HandlerOne: public Action::Manager<ActionType>::Handler
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
  HandlerOne handler;

  {
    Action::Manager<ActionType> manager;
    manager.add_handler(one, handler);
    manager.queue(new ActionOne(1));
    manager.queue(new ActionOne(2));
    manager.queue(new ActionOne(3));
    // Allow actions to be handled
    MT::Thread::usleep(100000);
  }

  vector<int> expected;
  expected.push_back(1);
  expected.push_back(2);
  expected.push_back(3);
  ASSERT_EQ(expected, handler.nums);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

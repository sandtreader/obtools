//==========================================================================
// ObTools::Init: test-init.cc
//
// Test harness for auto-initialisation and factory library
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-init.h"
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

//==========================================================================
// Test helpers

// Simple base class for testing factory/registry
class Animal
{
public:
  string name;
  virtual string speak() = 0;
  virtual ~Animal() {}
};

class Dog: public Animal
{
public:
  Dog(const string& n) { name = n; }
  string speak() override { return "Woof"; }
};

class Cat: public Animal
{
public:
  Cat(const string& n) { name = n; }
  string speak() override { return "Meow"; }
};

//==========================================================================
// Sequence / Action tests
// Note: Sequence uses a global static list that persists across tests.
// Actions must be heap-allocated to avoid dangling pointers.

static vector<int> action_log;

class TestAction: public Init::Action
{
  int id;
public:
  TestAction(int _id, int rank = 0): Init::Action(rank), id(_id) {}
  void initialise() override { action_log.push_back(id); }
};

TEST(SequenceTest, TestAddRunAndRankOrdering)
{
  action_log.clear();

  // Heap-allocate to avoid dangling pointers in global static list
  auto *high = new TestAction(10, 2);
  auto *mid = new TestAction(20, 1);
  auto *low = new TestAction(30, 0);

  Init::Sequence::add(*high);
  Init::Sequence::add(*mid);
  Init::Sequence::add(*low);
  Init::Sequence::run();

  // Find positions of 10, 20, 30 in the log
  int pos10 = -1, pos20 = -1, pos30 = -1;
  for (size_t i = 0; i < action_log.size(); i++)
  {
    if (action_log[i] == 10) pos10 = i;
    if (action_log[i] == 20) pos20 = i;
    if (action_log[i] == 30) pos30 = i;
  }

  ASSERT_NE(-1, pos10);
  ASSERT_NE(-1, pos20);
  ASSERT_NE(-1, pos30);
  // rank 0 (id=30) before rank 1 (id=20) before rank 2 (id=10)
  EXPECT_LT(pos30, pos20);
  EXPECT_LT(pos20, pos10);

  // Intentionally leak - global static list holds pointers we can't clear
}

//==========================================================================
// AutoAction tests

TEST(AutoActionTest, TestAutoRegistration)
{
  action_log.clear();

  // Heap-allocate to survive in global static list
  static int counter = 0;
  counter = 0;

  class CountingAction: public Init::AutoAction
  {
    int &cnt;
  public:
    CountingAction(int &c): Init::AutoAction(99), cnt(c) {}
    void initialise() override { cnt++; }
  };

  new CountingAction(counter);  // Intentional leak, auto-registers
  Init::Sequence::run();
  EXPECT_GT(counter, 0);
}

//==========================================================================
// Registry / Factory tests

class DogFactory: public Init::Factory<Animal, const string&>
{
public:
  Animal *create(const string& name) override { return new Dog(name); }
};

class CatFactory: public Init::Factory<Animal, const string&>
{
public:
  Animal *create(const string& name) override { return new Cat(name); }
};

TEST(RegistryTest, TestRegisterAndCreate)
{
  Init::Registry<Animal, const string&> registry;
  DogFactory dog_factory;
  CatFactory cat_factory;

  registry.add("dog", dog_factory);
  registry.add("cat", cat_factory);

  auto *d = registry.create("dog", "Rex");
  ASSERT_NE(nullptr, d);
  EXPECT_EQ("Woof", d->speak());
  EXPECT_EQ("Rex", d->name);
  delete d;

  auto *c = registry.create("cat", "Whiskers");
  ASSERT_NE(nullptr, c);
  EXPECT_EQ("Meow", c->speak());
  EXPECT_EQ("Whiskers", c->name);
  delete c;
}

TEST(RegistryTest, TestCreateUnknown)
{
  Init::Registry<Animal, const string&> registry;
  auto *result = registry.create("fish", "Nemo");
  EXPECT_EQ(nullptr, result);
}

//==========================================================================
// NewFactory tests

class Widget
{
public:
  string config;
  Widget(const string& c): config(c) {}
  virtual ~Widget() {}
};

class Button: public Widget
{
public:
  Button(const string& c): Widget(c) {}
};

TEST(NewFactoryTest, TestNewFactoryCreates)
{
  Init::NewFactory<Widget, Button, const string&> factory;
  Widget *w = factory.create("click-me");
  ASSERT_NE(nullptr, w);
  EXPECT_EQ("click-me", w->config);
  delete w;
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

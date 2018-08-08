//==========================================================================
// ObTools::Alarm: test-alarm.cc
//
// Test harness for alarm library
//
// Copyright (c) 2013 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-alarm.h"
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Alarm observer
class Observer: public Alarm::Clock::Observer
{
private:
  MT::Mutex mutex;
  bool alarmed;
  Time::Stamp reported_alarm_time;
  Time::Stamp actual_alarm_time;

public:
  //------------------------------------------------------------------------
  // Constructor
  Observer():
    alarmed(false)
  {}

  //------------------------------------------------------------------------
  // Receive alarm from clock
  void receive_alarm(const Time::Stamp& alarm_time)
  {
    MT::Lock lock(mutex);
    alarmed = true;
    actual_alarm_time = Time::Stamp::now();
    reported_alarm_time = alarm_time;
  }

  //------------------------------------------------------------------------
  // Did we receive an alarm?
  bool got_alarm()
  {
    MT::Lock lock(mutex);
    return alarmed;
  }

  //------------------------------------------------------------------------
  // Time we actually received alarm
  Time::Stamp get_actual_alarm_time()
  {
    MT::Lock lock(mutex);
    return actual_alarm_time;
  }

  //------------------------------------------------------------------------
  // Reported time we received alarm
  Time::Stamp get_reported_alarm_time()
  {
    MT::Lock lock(mutex);
    return reported_alarm_time;
  }
};

//--------------------------------------------------------------------------
// Tests
TEST(AlarmTest, TestAlarm)
{
  const Time::Duration resolution(0.02);
  const Time::Duration max_expected_drift(1.1 * resolution.seconds());
  Observer observer;
  Alarm::Clock clock(resolution);
  Time::Stamp alarm_time = Time::Stamp::now();
  alarm_time += Time::Duration(0.2);
  ASSERT_TRUE(clock.add_alarm(alarm_time, &observer));
  this_thread::sleep_for(chrono::milliseconds{250});
  ASSERT_TRUE(observer.got_alarm());
  Time::Stamp actual_time(observer.get_actual_alarm_time());
  Time::Stamp reported_time(observer.get_reported_alarm_time());
  ASSERT_EQ(alarm_time, reported_time);
  ASSERT_GE(actual_time, reported_time);
  ASSERT_LE(actual_time - reported_time, max_expected_drift);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

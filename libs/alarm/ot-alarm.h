//==========================================================================
// ObTools::Alarm: ot-alarm.h
//
// Public definitions for ObTools::Alarm
// Triggers events at given wall clock time
//
// Copyright (c) 2013 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_ALARM_H
#define __OBTOOLS_ALARM_H

#include "ot-mt.h"
#include "ot-time.h"
#include <set>

namespace ObTools { namespace Alarm {

using namespace std;

//==========================================================================
// Alarm clock class
class Clock
{
public:
  //------------------------------------------------------------------------
  // Observer for receiving alarm event
  class Observer
  {
  public:
    //----------------------------------------------------------------------
    // Receive alarm from clock
    virtual void receive_alarm(const Time::Stamp& alarm_time) = 0;

    //----------------------------------------------------------------------
    // Virtual destructor
    virtual ~Observer() {}
  };

private:
  //------------------------------------------------------------------------
  // Runs until time is up
  class TimerTask: public MT::Task
  {
  private:
    MT::Mutex mutex;
    Time::Stamp alarm_time;
    Clock& clock;
    const Time::Duration resolution;

  public:
    //----------------------------------------------------------------------
    // Constructor
    TimerTask(Clock& clock, const Time::Duration& resolution);

    //----------------------------------------------------------------------
    // Run method
    void run();

    //----------------------------------------------------------------------
    // Set alarm time
    void set_alarm_time(const Time::Stamp& time);
  };

  MT::Mutex mutex;
  map<Time::Stamp, set<Observer *> > observers;
  MT::TaskThread<TimerTask> timer_thread;
  MT::BasicCondVar ready;

  //------------------------------------------------------------------------
  // Receive alarm from thread and return next time
  Time::Stamp trigger_alarm(const Time::Stamp& time);

public:
  //------------------------------------------------------------------------
  // Constructor
  // Resolution is the smallest period between alarm checks. Note that the
  // thread will wake every resolution interval
  Clock(const Time::Duration& resolution);

  //------------------------------------------------------------------------
  // Add an alarm
  bool add_alarm(const Time::Stamp& time, Observer *observer);

  //------------------------------------------------------------------------
  // Remove an alarm
  bool remove_alarm(const Time::Stamp& time, Observer *observer);
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_ALARM_H

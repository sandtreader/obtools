//==========================================================================
// ObTools::Alarm: alarm.cc
//
// Implementation of alarm library
//
// Copyright (c) 2013 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-alarm.h"

namespace ObTools { namespace Alarm {

using namespace std;

//--------------------------------------------------------------------------
// Constructor
// Resolution is the smallest period between alarm checks. Note that the
// thread will wake every resolution interval
Clock::Clock(const Time::Duration& resolution):
  timer_thread(new TimerTask(*this, resolution))
{
}

//--------------------------------------------------------------------------
// Add an alarm
bool Clock::add_alarm(const Time::Stamp& time, Observer *observer)
{
  MT::Lock lock(mutex);
  map<Time::Stamp, set<Observer *> >::iterator it = observers.find(time);
  if (it != observers.end())
  {
    it->second.insert(observer);
    return true;
  }
  pair<map<Time::Stamp, set<Observer *> >::iterator, bool>
    new_it = observers.insert(make_pair(time, set<Observer *>()));
  new_it.first->second.insert(observer);
  if (new_it.first == observers.begin())
  {
    timer_thread->set_alarm_time(time);
  }
  return true;
}

//--------------------------------------------------------------------------
// Remove an alarm
bool Clock::remove_alarm(const Time::Stamp& time, Observer *observer)
{
  MT::Lock lock(mutex);
  map<Time::Stamp, set<Observer *> >::iterator it = observers.find(time);
  if (it == observers.end())
    return false;

  if (it->second.size() > 1)
  {
    it->second.erase(observer);
    return true;
  }

  if (it == observers.begin())
  {
    map<Time::Stamp, set<Observer *> >::iterator next(it);
    ++next;
    if (next == observers.end())
      timer_thread->set_alarm_time(Time::Stamp());
    else
      timer_thread->set_alarm_time(next->first);
  }

  observers.erase(it);

  return true;
}

//--------------------------------------------------------------------------
// Receive alarm from thread and return next time
Time::Stamp Clock::trigger_alarm(const Time::Stamp& time)
{
  MT::Lock lock(mutex);

  map<Time::Stamp, set<Observer *> >::iterator it = observers.find(time);
  if (it != observers.end())
  {
    for (set<Observer *>::iterator
         ob = it->second.begin(); ob != it->second.end(); ++ob)
    {
      (*ob)->receive_alarm(time);
    }
    observers.erase(it);
  }

  if (observers.empty())
    return Time::Stamp();

  return observers.begin()->first;
}

//--------------------------------------------------------------------------
// Constructor
Clock::TimerTask::TimerTask(Clock& _clock, const Time::Duration& _resolution):
  clock(_clock), resolution(_resolution)
{}

//--------------------------------------------------------------------------
// Run method
void Clock::TimerTask::run()
{
  while (is_running())
  {
    MT::Thread::nanosleep(resolution.seconds() * 1000000000);
    MT::Lock lock(mutex);
    if (alarm_time.valid() && alarm_time < Time::Stamp::now())
      alarm_time = clock.trigger_alarm(alarm_time);
  }
}

//--------------------------------------------------------------------------
// Set alarm time
void Clock::TimerTask::set_alarm_time(const Time::Stamp& time)
{
  MT::Lock lock(mutex);
  alarm_time = time;
}

}} // namespaces

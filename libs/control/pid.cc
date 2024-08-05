//==========================================================================
// ObTools::Control: pid.cc
//
// PID loop implementation
//
// Copyright (c) 2024 Paul Clark
//==========================================================================

#include <gtest/gtest.h>
#include "ot-control.h"

namespace ObTools { namespace Control {

// Tick, and generate the next control variable u(t) from the measured
// process variable y(t), at time t
double PIDLoop::tick(double pv, double t)
{
  auto d_t = t - previous_t;
  auto e_t = set_point - pv;
  auto d_e = e_t - previous_e;

  integral += e_t * d_t;
  auto derivative = (d_t > 0) ? (d_e/d_t) : 0.0;

  auto cv = k_p * e_t         // P
          + k_i * integral    // I
          + k_d * derivative; // D

  previous_t = t;
  previous_e = e_t;
  return cv;
}

}}

//==========================================================================
// ObTools::Control: ot-control.h
//
// Public definitions for ObTools::Control
//
// Control theory classes
//
// Copyright (c) 2024 Paul Clark.
//==========================================================================

#ifndef __OBTOOLS_CONTROL_H
#define __OBTOOLS_CONTROL_H

#include <memory>
#include <vector>
#include <queue>
#include <functional>

namespace ObTools { namespace Control {

// Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// PID loop
class PIDLoop
{
  // Parameters
  double k_p{0.0};
  double k_i{0.0};
  double k_d{0.0};

  double set_point{0.0};

  // State
  double previous_t{0.0};
  double previous_e{0.0};
  double integral{0.0};

public:
  // Construct
  PIDLoop() {}
  PIDLoop(double _k_p, double _k_i = 0.0, double _k_d = 0.0):
    k_p(_k_p), k_i(_k_i), k_d(_k_d) {}

  // Get/set set point
  double get_set_point() { return set_point; }
  void set_set_point(double _set_point) { set_point = _set_point; }

  // Get parameters
  double get_k_p() { return k_p; }
  double get_k_i() { return k_i; }
  double get_k_d() { return k_d; }

  // Set parameters
  void set_parameters(double _k_p, double _k_i = 0.0, double _k_d = 0.0)
  { k_p = _k_p; k_i = _k_i; k_d = _k_d; }
  void set_k_p(double _k_p) { k_p = _k_p; }
  void set_k_i(double _k_i) { k_i = _k_i; }
  void set_k_d(double _k_d) { k_d = _k_d; }

  // Reset at time t
  void reset(double t = 0.0) { previous_t = t; previous_e = integral = 0.0; }

  // Tick, and generate the next control variable u(t) from the measured
  // process variable y(t), at time t
  double tick(double pv, double t);
};

//==========================================================================
}} // namespaces

#endif // !__OBTOOLS_CONTROL_H

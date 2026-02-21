# CLAUDE.md - ObTools::Control Library

## Overview

`ObTools::Control` provides classical feedback control systems, specifically a PID (proportional-integral-derivative) control loop. Lives under `namespace ObTools::Control`.

**Header:** `ot-control.h`
**Dependencies:** none

## Key Classes

| Class | Purpose |
|-------|---------|
| `PIDLoop` | PID feedback controller |

## PIDLoop

```cpp
PIDLoop();
PIDLoop(double k_p, double k_i = 0.0, double k_d = 0.0);

// Set point
double get_set_point() const;
void set_set_point(double set_point);

// Gain parameters
void set_parameters(double k_p, double k_i = 0.0, double k_d = 0.0);
void set_k_p(double k_p);
void set_k_i(double k_i);
void set_k_d(double k_d);
double get_k_p() const;
double get_k_i() const;
double get_k_d() const;

// Control
void reset(double t = 0.0);
double tick(double pv, double t);  // returns control variable
```

## PID Formula

`u(t) = k_p * e(t) + k_i * integral(e) + k_d * de/dt`

Where `e(t) = set_point - pv` (process variable).

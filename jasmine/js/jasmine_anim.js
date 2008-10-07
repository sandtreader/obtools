// Jasmine animation library for Javascript
// Animation support
// (c) xMill Consulting Limited 2008.  All rights reserved.

//============================================================================
// Signal class - generates a 0-1 function over time
//  Target: Object with a set_scalar(value 0..1) function
//  F:      Single-value function providing waveform
//  Period: Cycle/time period in ms
//  Cycles: Number of cycles, 0=forever (default 0)
Jasmine.Signal = function(target, f, period, cycles)
{
  this.target = target;
  this.f      = f;
  this.period = period;
  this.cycles = cycles || 0;
  this.start = 0;
}

//-------------------------------------------------
// Signal ticker
Jasmine.Signal.prototype.tick = function(now)
{
  if (!this.start) this.start = now;

  var dt = now - this.start;
  var c = dt/this.period;

  // Check for finish
  if (this.cycles && c >= this.cycles)
  {
    // Force to end point
    this.target.set_scalar(this.f(1.0));
    return false;
  }

  // Modulus to 0..1
  var v = c - Math.floor(c);

  // Pass via wave function
  v = this.f(v);

  // Call to target
  this.target.set_scalar(v);

  return true;
}

//============================================================================
// Simple linear signal, 0..1 over period and stop
Jasmine.LinearSignal = function(target, period)
{
  this.parent.constructor.call(this, target, 
			       function(v) { return v; },
			       period, 1);
}

Jasmine.LinearSignal.inherits(Jasmine.Signal);

//============================================================================
// Sawtooth signal, repeatedly 0..1
Jasmine.SawtoothSignal = function(target, period, cycles)
{
  this.parent.constructor.call(this, target, 
			       function(v) { return v; },
			       period, cycles);
}

Jasmine.SawtoothSignal.inherits(Jasmine.Signal);

//============================================================================
// Triangle signal, repeatedly 0..1..0
Jasmine.TriangleSignal = function(target, period, cycles)
{
  this.parent.constructor.call(this, target, 
			       function(v) { return v<0.5?2*v:2-2*v; },
			       period, cycles);
}

Jasmine.TriangleSignal.inherits(Jasmine.Signal);

//============================================================================
// Sine signal, repeatedly 0..1..0 sine wave, with optional phase
Jasmine.SineSignal = function(target, period, phase, cycles)
{
  this.phase = phase || 0;

  var f = function (v)
  { return Math.sin(2*Math.PI*(v-0.25)+this.phase)/2+0.5; };

  // Sin function phase shifted to start at 0
  this.parent.constructor.call(this, target, f, period, cycles);
}

Jasmine.SineSignal.inherits(Jasmine.Signal);

//============================================================================
// Random signal, jumps beween 0..1 at start of every cycle
// (when v jumps backwards)
Jasmine.RandomSignal = function(target, period, cycles)
{
  this.current = 0;
  this.last = 1.0;
  this.parent.constructor.call(this, target, 
			       function(v) 
			       { if (v < this.last)
				   this.current = Math.random();
                                 this.last = v;
                                 return this.current; },
			       period, cycles);
}

Jasmine.RandomSignal.inherits(Jasmine.Signal);

//============================================================================
// Signal splitter - sends the same scalar input signal to 
// multiple outputs, array of targets
Jasmine.SignalSplitter = function(targets)
{
  this.targets = targets || [];
}

//--------------------------------------------------------------------------
// Splitter set method - passes v on to targets
Jasmine.SignalSplitter.prototype.set_scalar = function(v)
{
  for (t in this.targets) this.targets[t].set_scalar(v);
}

//--------------------------------------------------------------------------
// Add a target to a splitter
Jasmine.SignalSplitter.prototype.add = function(t)
{
  this.targets.push(t);
}

//--------------------------------------------------------------------------
// Remove a target from a splitter 
Jasmine.SignalSplitter.prototype.remove = function(t)
{
  for(var i=0; i<this.targets.length; i++)
    if (this.targets[i] == t)
      this.targets.splice(i--, 1);
}

//============================================================================
// ScalarInterpolator - generates value between two other values
// according to input signal
// Sends value to target with set_scalar(v)
// Note unusual order of end, start - start is optional and
// defaults to 0
Jasmine.ScalarInterpolator = function(target, end, start)
{
  this.target = target;
  this.start  = start || 0;
  this.end    = end;
}

//--------------------------------------------------------------------------
// Interpolator set method - takes scalar 'v' and 
// passes it on to target as interpolated value
Jasmine.ScalarInterpolator.prototype.set_scalar = function(v)
{
  var u = 1-v;
  var i = u*this.start + v*this.end;
  this.target.set_scalar(i);
}

//============================================================================
// VectorInterpolator - generates Vector between two other Vectors
// according to input signal
// Sends vector to target with set_vector(vector)
// Note unusual order of end, start - start is optional and
// defaults to (0,0,0)
Jasmine.VectorInterpolator = function(target, end, start)
{
  this.target = target;
  this.start  = start || new Jasmine.Vector(0,0,0);
  this.end    = end;
}

//--------------------------------------------------------------------------
// Interpolator set method - takes scalar 'v' and 
// passes it on to target as interpolated Vector
Jasmine.VectorInterpolator.prototype.set_scalar = function(v)
{
  var u = 1-v;
  var i = new Jasmine.Vector(
    u*this.start.x + v*this.end.x,
    u*this.start.y + v*this.end.y,
    u*this.start.z + v*this.end.z);
    
  this.target.set_vector(i);
}

//============================================================================
// PositionBinder - acts as an VectorInterpolator target which sets
// the position of the target Thing with move_by (note, relative)
Jasmine.PositionBinder = function(target)
{
  this.target = target;
  this.last = new Jasmine.Vector(0,0,0);
}

//--------------------------------------------------------------------------
Jasmine.PositionBinder.prototype.set_vector = function(vector)
{
  // Work out delta from last time
  var delta = vector.minus(this.last);
  this.target.move_by(delta);
  this.last = vector;
}

//============================================================================
// SizeBinder - acts as an VectorInterpolator target which sets
// the size of the target Thing with resize_by (note, relative)
Jasmine.SizeBinder = function(target)
{
  this.target = target;
  this.last = new Jasmine.Vector(0,0,0);
}

//--------------------------------------------------------------------------
Jasmine.SizeBinder.prototype.set_vector = function(vector)
{
  // Work out delta from last time
  var delta = vector.minus(this.last);
  this.target.resize_by(delta);
  this.last = vector;
}

//============================================================================
// BackgroundColorBinder - acts as an VectorInterpolator target which sets
// the background colour of the target Thing 
// Note: Colour binders are not additive
Jasmine.BackgroundColorBinder = function(target)
{
  this.target = target;
}

//--------------------------------------------------------------------------
Jasmine.BackgroundColorBinder.prototype.set_vector = function(vector)
{
  this.target.set_background_color(vector);
}

//============================================================================
// AlphaBinder - acts as an ScalarInterpolator target which sets
// the opacity of the target Thing 
// Note: Alpha binders are not additive
Jasmine.AlphaBinder = function(target)
{
  this.target = target;
}

//--------------------------------------------------------------------------
Jasmine.AlphaBinder.prototype.set_scalar = function(alpha)
{
  this.target.set_alpha(alpha);
}

//============================================================================
// VisiblityBinder - acts as an ScalarInterpolator target which sets
// the visibility of the target Thing to visible if value >= threshold
// Threshold defaults to 0.5
// Note: Visibility binders are not additive
Jasmine.VisibilityBinder = function(target, threshold)
{
  this.target = target;
  this.threshold = threshold || 0.5;
}

//--------------------------------------------------------------------------
Jasmine.VisibilityBinder.prototype.set_scalar = function(vis)
{
  this.target.set_visibility(vis >= this.threshold);
}

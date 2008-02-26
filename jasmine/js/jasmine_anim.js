// Jasmine animation library for Javascript
// Animation support
// (c) xMill Consulting Limited 2008.  All rights reserved.

//=============================================================
// Signal class - generates a 0-1 function over time
//  Target: Object with a set(value 0..1) function
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
    this.target.set(this.f(1.0));
    return false;
  }

  // Modulus to 0..1
  var v = c - Math.floor(c);

  // Pass via wave function
  v = this.f(v);

  // Call to target
  this.target.set(v);

  return true;
}

//=============================================================
// Simple linear signal, 0..1 over period and stop
Jasmine.LinearSignal = function(target, period)
{
  this.parent.constructor.call(this, target, 
			       function(v) { return v; },
			       period, 1);
}

Jasmine.LinearSignal.inherits(Jasmine.Signal);

//=============================================================
// Sawtooth signal, repeatedly 0..1
Jasmine.SawtoothSignal = function(target, period, cycles)
{
  this.parent.constructor.call(this, target, 
			       function(v) { return v; },
			       period, cycles);
}

Jasmine.SawtoothSignal.inherits(Jasmine.Signal);

//=============================================================
// Triangle signal, repeatedly 0..1..0
Jasmine.TriangleSignal = function(target, period, cycles)
{
  this.parent.constructor.call(this, target, 
			       function(v) { return v<0.5?2*v:2-2*v; },
			       period, cycles);
}

Jasmine.TriangleSignal.inherits(Jasmine.Signal);

//=============================================================
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

//=============================================================
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

//=============================================================
// Interpolator - generates Point between two other Points
// according to input signal
// Sends point to target with set(point)
// Note unusual order of end, start - start is optional and
// defaults to (0,0,0)
Jasmine.Interpolator = function(target, end, start)
{
  this.target = target;
  this.start  = start || new Jasmine.Point(0,0,0);
  this.end    = end;
}

//-------------------------------------------------
// Interpolator set method - takes scalar 'v' and 
// passes it on to target as interpolated Point
Jasmine.Interpolator.prototype.set = function(v)
{
  var u = 1-v;
  var point = new Jasmine.Point(
    u*this.start.x + v*this.end.x,
    u*this.start.y + v*this.end.y,
    u*this.start.z + v*this.end.z);
    
  this.target.set(point);
}

//=============================================================
// PositionBinder - acts as an Interpolator target which sets
// the position of the target Thing with move_by (note, relative)
Jasmine.PositionBinder = function(target)
{
  this.target = target;
  this.last = new Jasmine.Point(0,0,0);
}

//-------------------------------------------------
Jasmine.PositionBinder.prototype.set = function(point)
{
  // Work out delta from last time
  var delta = point.minus(this.last);
  this.target.move_by(delta);
  this.last = point;
}

//=============================================================
// SizeBinder - acts as an Interpolator target which sets
// the size of the target Thing with move_by (note, relative)
Jasmine.SizeBinder = function(target)
{
  this.target = target;
  this.last = new Jasmine.Point(0,0,0);
}

//-------------------------------------------------
Jasmine.SizeBinder.prototype.set = function(point)
{
  // Work out delta from last time
  var delta = point.minus(this.last);
  this.target.resize_by(delta);
  this.last = point;
}


// Jasmine animation library for Javascript
// (c) xMill Consulting Limited 2008.  All rights reserved.

// Main object, which provides the overall namespace
// Use 
//   with (Jasmine) { ... }
// to avoid typing it all the time!
var Jasmine = {};

//============================================================================
// Object support functions
// Inheritance creator - inserts an intermediate prototype to
// avoid derived methods moving into the base.  Also defines
// 'parent' as the base prototype for super calls (super is reserved)
Function.prototype.inherits = function(base)
{
  function Intermediate() {}
  Intermediate.prototype = base.prototype;  
  this.prototype = new Intermediate();
  this.prototype.constructor = this;
  this.prototype.parent = base.prototype;
}

//============================================================================
// Universe object (singleton class)
Jasmine.Universe = new Object();

// List of tickers
Jasmine.Universe.tickers = [];

//--------------------------------------------------------------------------
// Start method
Jasmine.Universe.start = function()
{
  // Current time
  this.now = new Date().getTime();

  // Start tick at default interval
  setInterval("Jasmine.Universe.tick()", 40);
}

//--------------------------------------------------------------------------
// Register a ticker
// Tickers must provide a tick(now) function which returns true
// if it wants to continue
Jasmine.Universe.add_ticker = function(t)
{
  this.tickers.push(t);
}

//--------------------------------------------------------------------------
// Universe interval tick
Jasmine.Universe.tick = function()
{
  // Get new time
  this.now = new Date().getTime();

  // Call all tickers, removing any that return false
  for(var i=0; i<this.tickers.length; i++)
    if (!this.tickers[i].tick(this.now))
      this.tickers.splice(i--, 1);
}

//============================================================================
// Vector class - 2D or 3D point/size.  z is optional, default 0
Jasmine.Vector = function(x,y,z)
{
  this.x = x;
  this.y = y;
  this.z = z||0;
}

//--------------------------------------------------------------------------
// Convert to string
Jasmine.Vector.prototype.toString = function()
{
  return "<"+this.x+","+this.y+","+this.z+">";
}

//--------------------------------------------------------------------------
// Subtract vectors
Jasmine.Vector.prototype.minus = function(o)
{
  return new Jasmine.Vector(this.x-o.x, this.y-o.y, this.z-o.z);
}

//--------------------------------------------------------------------------
// Add vectors
Jasmine.Vector.prototype.plus = function(o)
{
  return new Jasmine.Vector(this.x+o.x, this.y+o.y, this.z+o.z);
}

//--------------------------------------------------------------------------
// Multiply vectors (vector multiply)
Jasmine.Vector.prototype.times = function(o)
{
  return new Jasmine.Vector(this.x*o.x, this.y*o.y, this.z*o.z);
}

//============================================================================
// Thing class - superclass of all display objects
Jasmine.Thing = function(position, size)
{
  this.position = position;
  this.size = size;
  this.container = null;   // May apply transform/projection
}

//--------------------------------------------------------------------------
// Move to a new position
Jasmine.Thing.prototype.move_to = function(position)
{
  this.position = position;
}

//--------------------------------------------------------------------------
// Move relatively
Jasmine.Thing.prototype.move_by = function(delta)
{
  this.position.x += delta.x;
  this.position.y += delta.y;
  this.position.z += delta.z;
}

//--------------------------------------------------------------------------
// Change to a new size
Jasmine.Thing.prototype.resize_to = function(size)
{
  this.size = size;
}

//--------------------------------------------------------------------------
// Size relatively
Jasmine.Thing.prototype.resize_by = function(delta)
{
  this.size.x += delta.x;
  this.size.y += delta.y;
  this.size.z += delta.z;
}

//--------------------------------------------------------------------------
// Get transformed/projected position for display
// Returns display Thing
Jasmine.Thing.prototype.get_display = function()
{
}

//--------------------------------------------------------------------------
// Convert to string
Jasmine.Thing.prototype.toString = function()
{
  return ""+this.position+"+"+this.size;
}

//============================================================================
// DOMThing class - binding of a thing to a real DOM element
// position and size are optional and will be set from the
// element if not specified
Jasmine.DOMThing = function(id, position, size)
{
  this.id = id;
  this.element = document.getElementById(id);
  if (!this.element) return;  // Leave pos/size undefined

  this.parent.constructor.call(this, position, size);
}

Jasmine.DOMThing.inherits(Jasmine.Thing);

//--------------------------------------------------------------------------
// Set style to (possibly transformed/projected) position/size
Jasmine.DOMThing.prototype.display = function()
{
  if (!this.element) return;

  // Get display position/size
  var loc = new Jasmine.Thing(this.position, this.size);
  if (!!this.container) this.container.transform(loc);

  this.element.style.left   = loc.position.x+"px";
  this.element.style.top    = loc.position.y+"px";
  this.element.style.width  = loc.size.x+"px";
  this.element.style.height = loc.size.y+"px";
}

//--------------------------------------------------------------------------
// Move permanently to a new position - also moves DOM element
Jasmine.DOMThing.prototype.move_to = function(position)
{
  this.parent.move_to.call(this, position);
  this.display();
}

//--------------------------------------------------------------------------
// Move relatively - also moves DOM element
Jasmine.DOMThing.prototype.move_by = function(delta)
{
  this.parent.move_by.call(this, delta);
  this.display();
}

//--------------------------------------------------------------------------
// Change to a new size - also resizes DOM element
Jasmine.DOMThing.prototype.resize_to = function(size)
{
  this.parent.resize_to.call(this, size);
  this.display();
}

//--------------------------------------------------------------------------
// Resize relatively - also resizes DOM element
Jasmine.DOMThing.prototype.resize_by = function(delta)
{
  this.parent.resize_by.call(this, delta);
  this.display();
}

//--------------------------------------------------------------------------
// Convert to string
Jasmine.DOMThing.prototype.toString = function()
{
  return "DOM["+this.id+"]@"+this.parent.toString.call(this);
}

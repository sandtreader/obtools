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
// Clone
Jasmine.Vector.prototype.clone = function()
{
  return new Jasmine.Vector(this.x, this.y, this.z);
}

//--------------------------------------------------------------------------
// Convert to string
Jasmine.Vector.prototype.toString = function()
{
  return "<"+this.x+","+this.y+","+this.z+">";
}

//--------------------------------------------------------------------------
// Subtract vectors, creating a new one
Jasmine.Vector.prototype.minus = function(o)
{
  return new Jasmine.Vector(this.x-o.x, this.y-o.y, this.z-o.z);
}

//--------------------------------------------------------------------------
// Subtract vectors, modifying this
Jasmine.Vector.prototype.subtract = function(o)
{
  this.x -= o.x; this.y -= o.y; this.z -= o.z;
}

//--------------------------------------------------------------------------
// Add vectors, creating a new one
Jasmine.Vector.prototype.plus = function(o)
{
  return new Jasmine.Vector(this.x+o.x, this.y+o.y, this.z+o.z);
}

//--------------------------------------------------------------------------
// Add vectors, modifying this
Jasmine.Vector.prototype.add = function(o)
{
  this.x += o.x; this.y += o.y; this.z += o.z;
}

//--------------------------------------------------------------------------
// Multiply vectors (dot product), creating a new one
Jasmine.Vector.prototype.times = function(o)
{
  return new Jasmine.Vector(this.x*o.x, this.y*o.y, this.z*o.z);
}

//--------------------------------------------------------------------------
// Multiply vectors (dot product), modifying this 
Jasmine.Vector.prototype.multiply_by = function(o)
{
  this.x *= o.x; this.y *= o.y; this.z *= o.z;
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
// Clone, leaving container null
Jasmine.Thing.prototype.clone = function()
{
  return new Jasmine.Thing(this.position.clone(), this.size.clone());
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
  this.position.add(delta);
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
  this.size.add(delta);
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
  var loc;
  if (!!this.container)
  {
    loc = this.clone();
    this.container.transform(loc);
  }
  else loc = this;

  this.element.style.left   = loc.position.x+"px";
  this.element.style.top    = loc.position.y+"px";
  this.element.style.width  = loc.size.x+"px";
  this.element.style.height = loc.size.y+"px";

  // Assume the Z axis has been scaled enough to make sense as integers
  // Note: In an RH co-ord system with Y downwards, Z is positive behind
  // which is reverse to CSS order (positive in front)
  // Also shift baseline outwards to avoid hiding behind zIndex=0 background
  this.element.style.zIndex = 1000000-Math.floor(loc.position.z);  
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
// Set background colour to the given RGB vector (0..1, 0..1, 0..1)
// Note: We use US spelling in code for consistency with CSS
Jasmine.DOMThing.prototype.set_background_color = function(vector)
{
  if (!this.element) return;
  this.element.style.backgroundColor = 
    "rgb("+Math.floor(vector.x*255)
      +","+Math.floor(vector.y*255)
      +","+Math.floor(vector.z*255)+")";
}

//--------------------------------------------------------------------------
// Set alpha to given value 0 = transparent .. 1 = opaque
Jasmine.DOMThing.prototype.set_alpha = function(alpha)
{
  if (!this.element) return;

  // Firefox/Mozilla
  this.element.style.opacity = alpha;

  // IE
  this.element.style.filter = "alpha(opacity="+Math.floor(alpha*100)+")";
}

//--------------------------------------------------------------------------
// Set visibility - true or false
Jasmine.DOMThing.prototype.set_visibility = function(on)
{
  if (!this.element) return;

  // Firefox/Mozilla
  this.element.style.visibility = on?"visible":"hidden";
}

//--------------------------------------------------------------------------
// Convert to string
Jasmine.DOMThing.prototype.toString = function()
{
  return "DOM["+this.id+"]@"+this.parent.toString.call(this);
}

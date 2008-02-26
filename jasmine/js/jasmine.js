// Jasmine animation library for Javascript
// (c) xMill Consulting Limited 2008.  All rights reserved.

// Main object, which provides the overall namespace
// Use 
//   with (Jasmine) { ... }
// to avoid typing it all the time!
var Jasmine = {};

//=============================================================
// Object support functions
// Inheritance creator - inserts an intermediate prototype to
// avoid derived methods moving into the base.  Also defines
// 'parent' as the base prototype for super calls (super is reserved)
Jasmine.inherit = function(derived, base)
{
  function Intermediate() {}
  Intermediate.prototype = base.prototype;  
  derived.prototype = new Intermediate();
  derived.prototype.constructor = derived;
  derived.prototype.parent = base.prototype;
}

//=============================================================
// Universe object (singleton class)
Jasmine.Universe = new Object();

// List of tickers
Jasmine.Universe.tickers = [];

//-------------------------------------------------
// Start method
Jasmine.Universe.start = function()
{
  // Current time
  this.now = new Date().getTime();

  // Start tick at default interval
  setInterval("Jasmine.Universe.tick()", 40);
}

//-------------------------------------------------
// Register a ticker
// Tickers must provide a tick(now) function
Jasmine.Universe.add_ticker = function(t)
{
  this.tickers.push(t);
}

//-------------------------------------------------
// Universe interval tick
Jasmine.Universe.tick = function()
{
  // Get new time
  this.now = new Date().getTime();

  // Call all tickers
  for (t in this.tickers) this.tickers[t].tick(this.now);
}

//=============================================================
// Point class - 2D or 3D point/size.  z is optional, default 0
Jasmine.Point = function(x,y,z)
{
  this.x = x;
  this.y = y;
  this.z = z||0;
}

//-------------------------------------------------
// Convert to string
Jasmine.Point.prototype.toString = function()
{
  return "<"+this.x+","+this.y+","+this.z+">";
}

//=============================================================
// Thing class - superclass of all display objects
Jasmine.Thing = function(position, size)
{
  this.position = position;
  this.size = size;
}

//-------------------------------------------------
// Move to a new position
Jasmine.Thing.prototype.move_to = function(position)
{
  this.position = position;
}

//-------------------------------------------------
// Move relatively
Jasmine.Thing.prototype.move_by = function(delta)
{
  this.position.x += delta.x;
  this.position.y += delta.y;
  this.position.z += delta.z;
}

//-------------------------------------------------
// Change to a new size
Jasmine.Thing.prototype.resize_to = function(size)
{
  this.size = size;
}

//-------------------------------------------------
// Size relatively
Jasmine.Thing.prototype.resize_by = function(delta)
{
  this.size.x += delta.x;
  this.size.y += delta.y;
  this.size.z += delta.z;
}

//-------------------------------------------------
// Convert to string
Jasmine.Thing.prototype.toString = function()
{
  return ""+this.position+"+"+this.size;
}

//=============================================================
// DOMThing class - binding of a thing to a real DOM element
// 2D co-ordinates and size are taken from CSS style
// z is optional, defaults to 0
Jasmine.DOMThing = function(id, z)
{
  this.id = id;
  this.element = document.getElementById(id);
  if (!this.element) return;  // Leave pos/size undefined

  // Force absolute positioning and clipping
  this.element.style.position = "absolute";
  this.element.style.overflow = "hidden";

  // Walk up offset tree to get real position
  var left = top = 0;
  for(var e=this.element; e && e.offsetParent; e=e.offsetParent)
  {
    left += e.offsetLeft;
    top += e.offsetTop;
  }

  // And get real width/height
  var width = this.element.offsetWidth || 0;
  var height = this.element.offsetHeight || 0;

  var pos = new Jasmine.Point(left, top, z);
  var size = new Jasmine.Point(width, height);

  this.parent.constructor.call(this, pos, size);
}

Jasmine.inherit(Jasmine.DOMThing, Jasmine.Thing);

//-------------------------------------------------
// Move to a new position - also moves DOM element
Jasmine.DOMThing.prototype.move_to = function(position)
{
  // Shift position in document relative to original
  var dx = position.x-this.position.x;
  this.element.style.left=(parseInt(this.element.style.left||"0", 10)+dx)+"px";

  var dy = position.y-this.position.y;
  this.element.style.top=(parseInt(this.element.style.top||"0", 10)+dy)+"px";

  // Fix stored position
  this.parent.move_to.call(this, position);
}

//-------------------------------------------------
// Move relatively - also moves DOM element
Jasmine.DOMThing.prototype.move_by = function(delta)
{
  // Shift position in document relative to original
  this.element.style.left=
    (parseInt(this.element.style.left||"0", 10)+delta.x)+"px";

  this.element.style.top=
    (parseInt(this.element.style.top||"0", 10)+delta.y)+"px";

  // Fix stored position
  this.parent.move_by.call(this, delta);
}

//-------------------------------------------------
// Change to a new size - also resizes DOM element
Jasmine.DOMThing.prototype.resize_to = function(size)
{
  // Set size absolutely
  this.element.style.width = size.x+"px";
  this.element.style.height = size.y+"px";

  // Fix stored size
  this.parent.resize_to.call(this, size);
}

//-------------------------------------------------
// Resize relatively - also resizes DOM element
Jasmine.DOMThing.prototype.resize_by = function(delta)
{
  // Fix stored size
  this.parent.resize_by.call(this, delta);

  // Reset DOM size
  this.element.style.width = this.size.x+"px";
  this.element.style.height = this.size.y+"px";
}

//-------------------------------------------------
// Convert to string
Jasmine.DOMThing.prototype.toString = function()
{
  return "DOM["+this.id+"]@"+this.parent.toString.call(this);
}

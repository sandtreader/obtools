// Jasmine animation library for Javascript
// 3D support (really 2.5D, since we're only displaying 2D objects)
// Copyright (c) Paul Clark 2008.  All rights reserved.
// This code comes with NO WARRANTY and is subject to licence agreement

//============================================================================
// 3D world - holds multiple Things (including sub-worlds)
// and provides transformations for them.
// Transformations are composable, and provide transform_position
// and transform_size methods
Jasmine.World = function(things, transformations)
{
  // Attach to things
  this.things = things || [];
  for(t in this.things) this.things[t].container = this;

  this.transformations = transformations || [];
  this.container = null;  // We can be in parent world, too
}

//--------------------------------------------------------------------------
// Add a Thing
Jasmine.World.prototype.add = function(t)
{
  this.things.push(t);

  // Set things' container to ourselves, so they ask us for
  // display transforms
  t.container = this;
}

//--------------------------------------------------------------------------
// Remove a Thing
Jasmine.World.prototype.remove = function(t)
{
  for(var i=0; i<this.things.length; i++)
    if (this.things[i] == t)
      this.things.splice(i--, 1);
}

//--------------------------------------------------------------------------
// Add a transformation
Jasmine.World.prototype.add_transformation = function(t)
{
  this.transformations.push(t);
}

//--------------------------------------------------------------------------
// Remove a transformation
Jasmine.World.prototype.remove_transformation = function(t)
{
  for(var i=0; i<this.transformations.length; i++)
    if (this.transformations[i] == t)
      this.transformations.splice(i--, 1);
}

//--------------------------------------------------------------------------
// Redisplay the world
Jasmine.World.prototype.tick = function(now)
{
  this.display();
  return true;
}

//--------------------------------------------------------------------------
// Redisplay all things
Jasmine.World.prototype.display = function(now)
{
  for (t in this.things) this.things[t].display();
}

//--------------------------------------------------------------------------
// Transform location in place
Jasmine.World.prototype.transform = function(loc)
{
  // Transform with all our transformations in order
  for(t in this.transformations)
    this.transformations[t].transform(loc);

  // Pass up to parent if we have one
  if (!!this.container) this.container.transform(loc);
}

//============================================================================
// Scale transformation class:  
//  scale:       Vector containing x,y,z scales
Jasmine.Scale = function(scale)
{
  this.set_vector(scale);
}

//--------------------------------------------------------------------------
// Set the scale - can be used as a VectorInterpolator target
Jasmine.Scale.prototype.set_vector = function(scale)
{
  this.scale = scale;
}

//--------------------------------------------------------------------------
// Set the scale with a scalar (all axes the same)
// - can be used as a VectorInterpolator target
Jasmine.Scale.prototype.set_scalar = function(scale)
{
  this.scale = new Jasmine.Vector(scale, scale, scale);
}

//--------------------------------------------------------------------------
// Move the scale - can be used as a SizeBinder target
Jasmine.Scale.prototype.resize_by = function(delta)
{
  this.set_vector(this.scale.plus(delta));
}

//--------------------------------------------------------------------------
// Transform a location in place
Jasmine.Scale.prototype.transform = function(loc)
{
  loc.position.multiply_by(this.scale);
  loc.size.multiply_by(this.scale);
}

//============================================================================
// Rotation transformation class:  
//  rotation:      Vector of x,y,z rotations (rad)
Jasmine.Rotation = function(rotation)
{
  this.set_vector(rotation);
}

//--------------------------------------------------------------------------
// Set the rotation - can be used as a VectorInterpolator target
Jasmine.Rotation.prototype.set_vector = function(rotation)
{
  this.rotation = rotation;

  // Precalculate some useful stuff
  this.sinx = Math.sin(rotation.x);
  this.cosx = Math.cos(rotation.x);
  this.siny = Math.sin(rotation.y);
  this.cosy = Math.cos(rotation.y);
  this.sinz = Math.sin(rotation.z);
  this.cosz = Math.cos(rotation.z);
}

//--------------------------------------------------------------------------
// Move the rotation - can be used as a PositionBinder target
Jasmine.Rotation.prototype.move_by = function(delta)
{
  this.set_vector(this.rotation.plus(delta));
}

//--------------------------------------------------------------------------
// Transform location in place
Jasmine.Rotation.prototype.transform = function(loc)
{
  var x = loc.position.x;
  var y = loc.position.y;
  var z = loc.position.z;

  // Standard 3D rotation matrix
  var xy = this.cosx*y  - this.sinx*z;
  var xz = this.sinx*y  + this.cosx*z;
  var yz = this.cosy*xz - this.siny*x;
  var yx = this.siny*xz + this.cosy*x;
  var zx = this.cosz*yx - this.sinz*xy;
  var zy = this.sinz*yx + this.cosz*xy;
    
  loc.position.x = zx;
  loc.position.y = zy;
  loc.position.z = yz;
}

//============================================================================
// Translation transformation class:  
//  translation:       x,y,z translation Vector
Jasmine.Translation = function(translation)
{
  this.set_vector(translation);
}

//--------------------------------------------------------------------------
// Set the translation - can be used as a VectorInterpolator target
Jasmine.Translation.prototype.set_vector = function(translation)
{
  this.translation = translation;
}

//--------------------------------------------------------------------------
// Move the translation - can be used as a PositionBinder target
Jasmine.Translation.prototype.move_by = function(delta)
{
  this.set_vector(this.translation.plus(delta));
}

//--------------------------------------------------------------------------
// Transform a location in place
Jasmine.Translation.prototype.transform = function(loc)
{
  loc.position.add(this.translation);
}

//============================================================================
// Projection transformation class:  
//  depth: Depth of field
Jasmine.Perspective = function(depth)
{
  this.depth = depth;
}

//--------------------------------------------------------------------------
// Transform a location (perspective projection to 2D)
Jasmine.Perspective.prototype.transform = function(loc)
{
  var d = this.depth;
  var f = d/(d+loc.position.z);

  loc.position.x *= f;
  loc.position.y *= f;
  // keep z for z-ordering

  loc.size.x *= f;
  loc.size.y *= f;
  loc.size.z  = 0;
}


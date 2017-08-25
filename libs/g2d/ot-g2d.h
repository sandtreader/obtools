//==========================================================================
// ObTools::G2D: ot-g2d.h
//
// Public definitions for ObTools 2D Graphics Library
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_G2D_H
#define __OBTOOLS_G2D_H

namespace ObTools { namespace G2D {

// Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Core types

// Co-ordinates
typedef double coord_t;

//==========================================================================
// 2D Point
struct Point
{
  coord_t x;
  coord_t y;

  Point(): x(0.0), y(0.0) {}
  Point(coord_t _x, coord_t _y): x(_x), y(_y) {}
};

//==========================================================================
}} // namespaces

#endif // !__OBTOOLS_G2D_H

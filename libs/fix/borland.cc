//==========================================================================
// ObTools::Fix: borland.cc
//
// Fixes for Borland C++Builder on Windows
//
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-fix.h"
#include <ctype.h>

#ifndef __BORLANDC__
#error This file is only for Borland
#endif

//--------------------------------------------------------------------------
// atoll - atol equivalent for long longs
uint64_t atoll(const char *s)
{
  uint64_t n = 0;
  for(; *s && isdigit(*s); s++)
    n = n*10 + *s - '0';
  return n;
}

//==========================================================================
// ObTools::Fix: ot-fix.h
//
// Public definitions for ObTools::Fix 
// Platform specific fixes to fix missing aspects of ISO-C-99 etc.
// 
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#ifndef __OBTOOLS_FIX_H
#define __OBTOOLS_FIX_H

#include <stdint.h>

//==========================================================================
// Missing long long functions for Borland

//--------------------------------------------------------------------------
// atol equivalent for long longs
#ifdef __BORLANDC__
uint64_t atoll(const char *s);
#endif


#endif // !__OBTOOLS_FIX_H

















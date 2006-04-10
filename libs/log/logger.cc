//==========================================================================
// ObTools::Log: logger.cc
//
// Global log handlers for simple use
//
// e.g.
//
//   ObTools::Log::Summary << "Transaction " << i << " started" << endl;
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-log.h"

namespace ObTools { namespace Log {

//==========================================================================
// Singleton Logger for simple Logs
Distributor logger;

//==========================================================================
// Simple global log streams
// Do not remove this restriction to single-threaded code without reading
// *** notes in ot-log.h!
#if defined(_SINGLE)
Stream Error  (logger, LEVEL_ERROR);
Stream Summary(logger, LEVEL_SUMMARY);
Stream Detail (logger, LEVEL_DETAIL);
#if OBTOOLS_LOG_MAX_LEVEL >= OBTOOLS_LOG_LEVEL_DEBUG
Stream Debug  (logger, LEVEL_DEBUG);
#endif
#if OBTOOLS_LOG_MAX_LEVEL >= OBTOOLS_LOG_LEVEL_DUMP
Stream Dump   (logger, LEVEL_DUMP);
#endif
#endif

}} // namespaces




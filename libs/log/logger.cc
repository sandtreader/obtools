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
//==========================================================================

#include "ot-log.h"

namespace ObTools { namespace Log {

//==========================================================================
// Singleton Logger for simple Logs
Distributor logger;

//==========================================================================
// Simple global log streams
LogStream Error  (logger, LEVEL_ERROR);
LogStream Summary(logger, LEVEL_SUMMARY);
LogStream Detail (logger, LEVEL_DETAIL);
LogStream Debug  (logger, LEVEL_DEBUG);
LogStream Dump   (logger, LEVEL_DUMP);

}} // namespaces




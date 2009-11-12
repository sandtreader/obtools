//==========================================================================
// ObTools::Log: test-log.cc
//
// Test harness for log library
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-log.h"
using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main

int main()
{
  Log::StreamChannel chan_out(cout);
  Log::StreamChannel chan_err(cerr);

  Log::PatternFilter hfilter("H*", chan_out);
  Log::LevelFilter   level_out(Log::LEVEL_SUMMARY, hfilter);

  Log::TimestampFilter tsfilter("%H:%M:%*S %a %d %b %Y [%*L]:", chan_err);
  Log::LevelFilter     level_err(Log::LEVEL_ERROR, tsfilter);

  Log::logger.connect(hfilter);
  Log::logger.connect(level_err);

#if defined(_SINGLE)
  // Use global log streams
  Log::Summary << "Hello, world\nThis is a test\n\n";
  Log::Summary << "You shouldn't see this\n";

  OBTOOLS_LOG_IF_DUMP(
    Log::Dump << "This is more than you ever wanted to know\n";
    )

  Log::Error << "Hey, both cout and cerr should see this\n";
  Log::Error << "But only cerr will see this\nand this\n";
#else
  Log::Streams log;
  log.summary << "Hello, world\nThis is a test\n\n";
  log.summary << "You shouldn't see this\n";

  OBTOOLS_LOG_IF_DUMP(
    log.dump << "This is more than you ever wanted to know\n";
    )

  log.error << "Hey, both cout and cerr should see this\n";
  log.error << "But only cerr will see this\nand this\n";
#endif

  return 0;  
}



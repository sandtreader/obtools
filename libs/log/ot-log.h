//==========================================================================
// ObTools::Log: ot-log.h
//
// Public definitions for ObTools::Log
// Logging handling
// 
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#ifndef __OBTOOLS_LOG_H
#define __OBTOOLS_LOG_H

#include "ot-mt.h"
#include <time.h>
#include <list>
#include <string>
#include <iostream>

namespace ObTools { namespace Log { 

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Standard log levels
enum Level
{
  LEVEL_ERROR   = 1,   // Errors the operator should know about
  LEVEL_SUMMARY = 2,   // Summary of things happening
  LEVEL_DETAIL  = 3,   // Detail of things happening
  LEVEL_DEBUG   = 4,   // Debugging
  LEVEL_DUMP    = 5    // Gory detail (packet dumps etc.)
};

//==========================================================================
// Maximum log level ever allowed
// Used to optimise out log calls above this level - see dump_ok etc. below
// Overrideable in Makefile
#if !defined OBTOOLS_LOG_MAX_LEVEL
#define OBTOOLS_LOG_MAX_LEVEL LEVEL_DETAIL
#endif

//==========================================================================
// Log message
class Message
{
public:
  Level level;
  time_t timestamp;
  string text;          // Line of text without EOL

  //--------------------------------------------------------------------------
  // Constructor
  Message(Level l, const string& t): level(l), text(t) 
  { ::time(&timestamp); }
};

//==========================================================================
// Log channel 
// Abstract virtual definition of a logging channel
class Channel
{
public:
  MT::Mutex mutex;  // Assume multi-threaded use

  //--------------------------------------------------------------------------
  // Constructor
  Channel() {}

  //--------------------------------------------------------------------------
  // Abstract virtual logging function
  virtual void log(Message& msg) = 0;

  //--------------------------------------------------------------------------
  // Virtual destructor
  virtual ~Channel() {}
};

//==========================================================================
// Abtract Filter channel
// Does something to the message and passes it on, or drops it
class Filter: public Channel
{
protected:
  Channel& next;

public:
  Filter(Channel& _next): next(_next) {}
};

//==========================================================================
// LevelFilter channel
// Filters messages by maximum log level
class LevelFilter: public Filter
{
private:
  Level level;

public:
  LevelFilter(Level _l, Channel& _next): level(_l), Filter(_next) {}

  void log(Message& msg);
};

//==========================================================================
// PatternFilter channel
// Filters messages by a pattern applies to the text
class PatternFilter: public Filter
{
private:
  string pattern;

public:
  // Constructor takes Text::pattern_match (glob) format
  PatternFilter(const string& _p, Channel& _next): 
    pattern(_p), Filter(_next) {}

  void log(Message& msg);
};

//==========================================================================
// TimestampFilter channel
// Adds timestamps to the front of each message
class TimestampFilter: public Filter
{
private:
  string format;

public:
  // Constructor takes strftime format
  TimestampFilter(const string& _format, Channel& _next): 
    format(_format), Filter(_next) {}

  void log(Message& msg);
};

//==========================================================================
// Channel to standard iostream
class StreamChannel: public Channel
{
private:
  ostream& stream;

public:
  //--------------------------------------------------------------------------
  // Constructor
  StreamChannel(ostream& s): stream(s) {}

  //--------------------------------------------------------------------------
  // Logging function
  void log(Message& msg);
};

//==========================================================================
// Log distribution point
// Also a channel, so they can be chained
class Distributor: public Channel
{
private:
  list<Channel *> channels;

public:
  //--------------------------------------------------------------------------
  // Constructor
  Distributor() {}
  
  //--------------------------------------------------------------------------
  // Connect a channel 
  void connect(Channel& chan);

  //--------------------------------------------------------------------------
  // Disconnect the given channel
  void disconnect(Channel& chan);

  //--------------------------------------------------------------------------
  // Log a message
  void log(Message& msg);
};

//==========================================================================
// Log stream for use with sugared objects below

//Logging streambuf
class LogStreamBuf: public streambuf
{
private:
  string buffer;
  bool closed;
  Channel& channel;
  Level level;

protected:
  // Streambuf overflow - handles characters
  int overflow(int); 
   
public:
  // Constructor
  LogStreamBuf(Channel& _channel, Level level);

  // Close stream
  void close();
};

//Log stream
class LogStream: public ostream
{
public:
  // Constructor - like ofstream
  LogStream(Channel &channel, Level level): 
    ostream(new LogStreamBuf(channel, level)) {}

  // Destructor
  ~LogStream() { delete rdbuf(); }

  // Close - pass on to LogStreamBuf to do
  void close() { static_cast<LogStreamBuf *>(rdbuf())->close(); }
};

//==========================================================================
// Singleton Distributor for simple Logs
extern Distributor logger;

//==========================================================================
// Simple log streams
// e.g.
//   ObTools::Log::Error << "The end of the world is nigh!" << endl;

extern LogStream Error;
extern LogStream Summary;
extern LogStream Detail;
extern LogStream Debug;
extern LogStream Dump;

//==========================================================================
// Useful constants for optimising out high log levels
//
// e.g.
//   if (ObTools::Log::dump_ok)
//     ObTools::Log::Dump << "The entire packet is: " << packet;

const bool error_ok   = (OBTOOLS_LOG_MAX_LEVEL >= LEVEL_ERROR);
const bool summary_ok = (OBTOOLS_LOG_MAX_LEVEL >= LEVEL_SUMMARY);
const bool detail_ok  = (OBTOOLS_LOG_MAX_LEVEL >= LEVEL_DETAIL);
const bool debug_ok   = (OBTOOLS_LOG_MAX_LEVEL >= LEVEL_DEBUG);
const bool dump_ok    = (OBTOOLS_LOG_MAX_LEVEL >= LEVEL_DUMP);

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_LOG_H




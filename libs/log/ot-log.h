//==========================================================================
// ObTools::Log: ot-log.h
//
// Public definitions for ObTools::Log
// Logging handling
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_LOG_H
#define __OBTOOLS_LOG_H

#include "ot-mt.h"
#include "ot-time.h"
#include <list>
#include <string>
#include <iostream>

namespace ObTools { namespace Log {

// Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Standard log levels
enum class Level
{
  none    = 0,   // Nothing at all (nothing should log at this level)
  error   = 1,   // Errors the operator should know about
  summary = 2,   // Summary of things happening
  detail  = 3,   // Detail of things happening
  debug   = 4,   // Debugging
  dump    = 5    // Gory detail (packet dumps etc.)
};

inline Level& operator++(Level& level)
{
  if (level != Level::dump)
    level = static_cast<Level>(static_cast<int>(level) + 1);
  return level;
}

inline Level& operator--(Level& level)
{
  if (level != Level::none)
    level = static_cast<Level>(static_cast<int>(level) - 1);
  return level;
}

// Parallel macros for use in preprocessor
#define OBTOOLS_LOG_LEVEL_NONE    0
#define OBTOOLS_LOG_LEVEL_ERROR   1
#define OBTOOLS_LOG_LEVEL_SUMMARY 2
#define OBTOOLS_LOG_LEVEL_DETAIL  3
#define OBTOOLS_LOG_LEVEL_DEBUG   4
#define OBTOOLS_LOG_LEVEL_DUMP    5

//==========================================================================
// Maximum log level ever allowed
// Used to optimise out log calls above this level - see dump_ok etc. below
// Overrideable in Makefile, otherwise checks DEBUG and PROFILE
// (Don't want unused debugging in PROFILE versions messing up figures)
#if !defined OBTOOLS_LOG_MAX_LEVEL
#if defined(DEBUG) && !defined(PROFILE)
#define OBTOOLS_LOG_MAX_LEVEL OBTOOLS_LOG_LEVEL_DUMP
#else
#define OBTOOLS_LOG_MAX_LEVEL OBTOOLS_LOG_LEVEL_DETAIL
#endif
#endif

//==========================================================================
// Log message
class Message
{
public:
  Level level;
  Time::Stamp timestamp;
  string text;          // Line of text without EOL

  //------------------------------------------------------------------------
  // Constructor
  Message(Level l, const string& t): level(l), text(t)
  { timestamp=Time::Stamp::now(); }
};

//==========================================================================
// Log channel
// Abstract virtual definition of a logging channel
// Note: Safe to share channels between threads because mutexed
class Channel
{
public:
  //------------------------------------------------------------------------
  // Abstract virtual logging function
  virtual void log(Message& msg) = 0;

  //------------------------------------------------------------------------
  // Virtual destructor
  virtual ~Channel() {}
};

//==========================================================================
// Abstract Filter
// Modifies, drops or passes message
class Filter
{
public:
  //------------------------------------------------------------------------
  // Pass a message - returns true if message should be passed on
  virtual bool pass(Message& msg) = 0;

  //------------------------------------------------------------------------
  // Virtual destructor
  virtual ~Filter() {}
};

//==========================================================================
// Abtract Filter channel
// Does something to the message and passes it on, or drops it
class FilteredChannel: public Channel
{
protected:
  list<unique_ptr<Filter>> filters;
  unique_ptr<Channel> output;

public:
  //------------------------------------------------------------------------
  // Constructor (takes ownership of output channel)
  FilteredChannel(Channel *_output): output{_output} {}

  //------------------------------------------------------------------------
  // Add a filter (takes ownership of filter)
  void append_filter(Filter *filter)
  {
    filters.emplace_back(filter);
  }

  //------------------------------------------------------------------------
  // Log message
  void log(Message& msg) override;
};

//==========================================================================
// LevelFilter
// Filters messages by maximum log level
class LevelFilter: public Filter
{
private:
  Level level;

public:
  //------------------------------------------------------------------------
  // Constructor
  LevelFilter(Level _level): level{_level} {}

  //------------------------------------------------------------------------
  // Pass a message - returns true if message should be passed on
  bool pass(Message& msg) override
  {
    return msg.level <= level;
  }
};

//==========================================================================
// PatternFilter
// Filters messages by a pattern applies to the text
class PatternFilter: public Filter
{
private:
  string pattern;

public:
  //------------------------------------------------------------------------
  // Constructor takes Text::pattern_match (glob) format
  PatternFilter(const string& _pattern): pattern{_pattern} {}

  //------------------------------------------------------------------------
  // Pass a message - returns true if message should be passed on
  bool pass(Message& msg) override;
};

//==========================================================================
// TimestampFilter
// Adds timestamps to the front of each message
// Accepts strftime format, plus the following extensions:
//   %*L:   The log level as a single digit
//   %*S:   The exact floating point seconds time (use instead of %S)
class TimestampFilter: public Filter
{
private:
  string format;

public:
  //------------------------------------------------------------------------
  // Constructor takes strftime format
  TimestampFilter(const string& _format): format{_format} {}

  //------------------------------------------------------------------------
  // Pass a message - returns true if message should be passed on
  bool pass(Message& msg) override;
};

//==========================================================================
// Channel to standard iostream
class StreamChannel: public Channel
{
private:
  ostream *stream;

public:
  //------------------------------------------------------------------------
  // Constructor
  StreamChannel(ostream *s): stream(s) {}

  //------------------------------------------------------------------------
  // Logging function
  void log(Message& msg);
};

//==========================================================================
// Channel to standard iostream
class OwnedStreamChannel: public Channel
{
private:
  unique_ptr<ostream> stream;

public:
  //------------------------------------------------------------------------
  // Constructor (takes ownership of stream)
  OwnedStreamChannel(ostream *s): stream(s) {}

  //------------------------------------------------------------------------
  // Logging function
  void log(Message& msg);
};

//==========================================================================
// Channel to reference of another Channel
class ReferencedChannel: public Channel
{
private:
  Channel& channel;

public:
  //------------------------------------------------------------------------
  // Constructor
  ReferencedChannel(Channel& _channel): channel(_channel) {}

  //------------------------------------------------------------------------
  // Logging function
  void log(Message& msg)
  {
    channel.log(msg);
  }
};

//==========================================================================
// Channel for syslog
class SyslogChannel: public Channel
{
public:
  //------------------------------------------------------------------------
  // Constructor
  SyslogChannel() {}

  //------------------------------------------------------------------------
  // Logging function
  void log(Message& msg);
};

//==========================================================================
// Log distribution point
// Also a channel, so they can be chained
class Distributor: public Channel
{
private:
  MT::Mutex mutex;
  list<unique_ptr<Channel>> channels;

public:
  //------------------------------------------------------------------------
  // Connect a channel (takes ownership)
  void connect(Channel *channel);

  //------------------------------------------------------------------------
  // Connect a channel with timestamp and level logging (takes ownership)
  void connect_full(Channel *channel, Level level, const string& time_format);

  //------------------------------------------------------------------------
  // Log a message
  void log(Message& msg) override;
};

//==========================================================================
// Log stream for use with sugared objects below

// Logging streambuf
class StreamBuf: public streambuf
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
  StreamBuf(Channel& _channel, Level level);

  // Close stream
  void close();
};

// Log stream
//  *** Note:  Not shareable between threads - see *** below
class Stream: public ostream
{
public:
  // Constructor - like ofstream
  Stream(Channel &channel, Level level):
    ostream(new StreamBuf(channel, level))
  {
  }

  //------------------------------------------------------------------------
  // Move constructor
  Stream(Stream&& s):
    ostream(std::move(s)) // Had to use std namespace
                          // - wtf is the other move()?
  {
    set_rdbuf(rdbuf());
    s.set_rdbuf(nullptr);
  }

  // Destructor
  ~Stream() { if (rdbuf()) delete rdbuf(); }

  // Close - pass on to StreamBuf to do
  void close() { static_cast<StreamBuf *>(rdbuf())->close(); }
};

//==========================================================================
// Singleton Distributor for simple Logs
// Note: Safe to share between threads
extern Distributor logger;

//==========================================================================
// Useful macros for optimising out high log levels
//
// e.g.
//  OBTOOLS_LOG_IF_DUMP(ObTools::Log::Dump << "Packet: " << packet;)
//
// Also simple macros for #if - e.g.
// #if OBTOOLS_LOG_DEBUG
//  ...

#if OBTOOLS_LOG_MAX_LEVEL >= OBTOOLS_LOG_LEVEL_DEBUG
#define OBTOOLS_LOG_IF_DEBUG(_s) _s
#define OBTOOLS_LOG_DEBUG 1
#else
#define OBTOOLS_LOG_IF_DEBUG(_s)
#define OBTOOLS_LOG_DEBUG 0
#endif

#if OBTOOLS_LOG_MAX_LEVEL >= OBTOOLS_LOG_LEVEL_DUMP
#define OBTOOLS_LOG_IF_DUMP(_s) _s
#define OBTOOLS_LOG_DUMP 1
#else
#define OBTOOLS_LOG_IF_DUMP(_s)
#define OBTOOLS_LOG_DUMP 0
#endif

//==========================================================================
// Single logging streams - use in multi-threaded code to get a local logging
// stream.  If you need more than two of these, use a Streams object instead
// e.g.
//   ObTools::Log::Error elog;
//   elog << "Oops\n";

class Error: public Stream
{
public:
  Error(): Stream(logger, Level::error) {}
  Error(Error&& e): Stream(std::move(e)) {}
};

class Summary: public Stream
{
public:
  Summary(): Stream(logger, Level::summary) {}
  Summary(Summary&& s): Stream(std::move(s)) {}
};

class Detail: public Stream
{
public:
  Detail(): Stream(logger, Level::detail) {}
  Detail(Detail&& d): Stream(std::move(d)) {}
};

#if OBTOOLS_LOG_DEBUG
class Debug: public Stream
{
public:
  Debug(): Stream(logger, Level::debug) {}
  Debug(Debug&& d): Stream(std::move(d)) {}
};
#endif

#if OBTOOLS_LOG_DUMP
class Dump: public Stream
{
public:
  Dump(): Stream(logger, Level::dump) {}
  Dump(Dump&& d): Stream(std::move(d)) {}
};
#endif

//==========================================================================
// Log streams structure - lazy way to get all the different levels of stream
// e.g.
//   ObTools::Log::Streams log;  // On stack or thread-local object
//
//   log.error << "Oops\n";
//   log.detail << "Something really bad happened\n";

struct Streams
{
  Error error;
  Summary summary;
  Detail detail;
#if OBTOOLS_LOG_DEBUG
  Debug debug;
#endif
#if OBTOOLS_LOG_DUMP
  Dump dump;
#endif
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_LOG_H

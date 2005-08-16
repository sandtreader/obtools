//==========================================================================
// ObTools::Log: ot-log.h
//
// Public definitions for ObTools::Log
// Logging handling
// 
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#ifndef __OBTOOLS_LOG_H
#define __OBTOOLS_LOG_H

#if !defined(_SINGLE)
#include "ot-mt.h"
#endif

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
  LEVEL_NONE    = 0,   // Nothing at all (nothing should log at this level)
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
// Note: Safe to share channels between threads because mutexed
class Channel
{
public:
#if !defined(_SINGLE)
  MT::Mutex mutex;  // Assume multi-threaded use
#endif

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
  LevelFilter(Level _l, Channel& _next): Filter(_next), level(_l) {}

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
    Filter(_next), pattern(_p) {}

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
    Filter(_next), format(_format) {}

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

//Log stream
//  *** Note:  Not shareable between threads - see *** below
class Stream: public ostream
{
public:
  // Constructor - like ofstream
  Stream(Channel &channel, Level level): 
    ostream(new StreamBuf(channel, level)) {}

  // Destructor
  ~Stream() { delete rdbuf(); }

  // Close - pass on to StreamBuf to do
  void close() { static_cast<StreamBuf *>(rdbuf())->close(); }
};

//==========================================================================
// Singleton Distributor for simple Logs
// Note: Safe to share between threads
extern Distributor logger;

//==========================================================================
// Simple log streams
// e.g.
//   ObTools::Log::Error << "The end of the world is nigh!" << endl;
//
// *** NOTE *** 
// *** Since ostreams are not thread-sharable, and I have made no attempt
// *** to make Log::StreamBuf be so, these global streams must NOT be used
// *** in multithreaded code by more than one thread, otherwise you WILL
// *** get weird unpredictable crashes when two threads log at the same time
// *** This is potentially such a Bad Thing that I have disabled these 
// *** globals for all multi-threaded code - subvert at your peril!
// *** Use a thread-local Log::Streams structure (see below) instead,
// *** or make your own temporary Stream
#if defined(_SINGLE)
extern Stream Error;
extern Stream Summary;
extern Stream Detail;

// Only provide debug streams if allowed - this forces people to use the
// LOG_IF_DEBUG/DUMP macros to optimise out code which would otherwise
// still generate the logging, but get thrown away at the LevelFilter
#if OBTOOLS_LOG_MAX_LEVEL >= LEVEL_DEBUG
extern Stream Debug;
#endif
#if OBTOOLS_LOG_MAX_LEVEL >= LEVEL_DUMP
extern Stream Dump;
#endif
#endif

//==========================================================================
// Log streams structure - use in multi-threaded code to get yoursel a
// thread-local stream group
// e.g.
//   ObTools::Log::Streams log;  // On stack or thread-local object
//
//   log.error << "Oops\n";

struct Streams
{
  Stream error;
  Stream summary;
  Stream detail;
#if OBTOOLS_LOG_MAX_LEVEL >= LEVEL_DEBUG
  Stream debug;
#endif
#if OBTOOLS_LOG_MAX_LEVEL >= LEVEL_DUMP
  Stream dump;
#endif

  Streams():
    error  (logger, LEVEL_ERROR),
    summary(logger, LEVEL_SUMMARY),
    detail (logger, LEVEL_DETAIL),
#if OBTOOLS_LOG_MAX_LEVEL >= LEVEL_DEBUG
    debug  (logger, LEVEL_DEBUG),
#endif
#if OBTOOLS_LOG_MAX_LEVEL >= LEVEL_DUMP
    dump   (logger, LEVEL_DUMP) 
#endif
    {}
};

//==========================================================================
// Useful macros for optimising out high log levels
//
// e.g.
//  OBTOOLS_LOG_IF_DUMP(ObTools::Log::Dump << "Packet: " << packet;)

#
#if OBTOOLS_LOG_MAX_LEVEL >= LEVEL_DEBUG
#define OBTOOLS_LOG_IF_DEBUG(_s) _s
#else
#define OBTOOLS_LOG_IF_DEBUG(_s)
#endif

#if OBTOOLS_LOG_MAX_LEVEL >= LEVEL_DUMP
#define OBTOOLS_LOG_IF_DUMP(_s) _s
#else
#define OBTOOLS_LOG_IF_DUMP(_s)
#endif

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_LOG_H




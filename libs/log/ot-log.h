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
#include <map>
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
  string text;

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
  Level level;
  MT::Mutex mutex;  // Assume multi-threaded use

  //--------------------------------------------------------------------------
  // Constructor
  Channel(Level l=LEVEL_SUMMARY): level(l) {}

  //--------------------------------------------------------------------------
  // Abstract virtual logging function
  virtual void log(Message& msg) = 0;

  //--------------------------------------------------------------------------
  // Virtual destructor
  virtual ~Channel() {}
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
  StreamChannel(ostream& s, Level l=LEVEL_SUMMARY): Channel(l), stream(s) {}

  //--------------------------------------------------------------------------
  // Logging function
  void log(Message& msg);
};

//==========================================================================
// Log distribution point
// Also a channel, so they can be chained
// Own level applies as well as channels'
class Distributor: public Channel
{
private:
  map<string, Channel *> channels;

public:
  //--------------------------------------------------------------------------
  // Constructor
  // Set own level to maximum
  Distributor(): Channel(LEVEL_DUMP) {}
  
  //--------------------------------------------------------------------------
  // Connect a channel under the given name
  // Takes ownership and will dispose of it unless disconnected again
  void connect(const string& name, Channel *chan);

  //--------------------------------------------------------------------------
  // Disconnect the named channel but don't dispose of it
  // Returns the channel if found, 0 if not
  Channel *disconnect(const string& name);

  //--------------------------------------------------------------------------
  // Disconnect and dispose of the named channel
  // Whether it was found
  bool dispose(const string& name);

  //--------------------------------------------------------------------------
  // Log a message
  void log(Message& msg);

  //--------------------------------------------------------------------------
  // Destructor - disposes of all registered channels
  ~Distributor();
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
  // Constructor/destructor
  LogStreamBuf(Channel& _channel, Level level);
  ~LogStreamBuf();

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
  ~LogStream() { close(); delete rdbuf(); }

  // Close - pass on to LogStreamBuf to do
  void close() { static_cast<LogStreamBuf *>(rdbuf())->close(); }
};

//==========================================================================
// Null stream for use for channels above global maximum level

//Null streambuf
class NullBuf: public streambuf
{
protected:
  int overflow(int) { return 0; }
   
public:
  NullBuf() {}
  ~NullBuf() {}
  void close() {}
};

//Null stream
class NullStream: public ostream
{
public:
  NullStream(): ostream(new NullBuf()) {}
  ~NullStream() { delete rdbuf(); }
  void close() { }
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




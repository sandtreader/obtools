//==========================================================================
// ObTools::Redis: ot-redis.h
//
// Public definitions for ObTools::Redis
// Minimal Redis client - RESP2 protocol over TCP
//
// Copyright (c) 2026 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_REDIS_H
#define __OBTOOLS_REDIS_H

#include "ot-net.h"
#include "ot-mt.h"
#include "ot-time.h"
#include <cstdint>
#include <istream>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace ObTools { namespace Redis {

// Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// A RESP2 value - a reply, or a command (an array of bulk strings)
struct Value
{
  enum class Type
  {
    nil,          // Null bulk string or array: $-1, *-1
    status,       // +OK
    error,        // -ERR ...
    integer,      // :1
    bulk,         // $3\r\nfoo\r\n
    array         // *2\r\n...
  };

  Type type{Type::nil};
  string str;                   // status, error and bulk
  int64_t integer{0};
  vector<Value> elements;       // array

  //------------------------------------------------------------------------
  // Check type
  bool is_nil() const   { return type == Type::nil; }
  bool is_error() const { return type == Type::error; }
};

//==========================================================================
// Protocol

//--------------------------------------------------------------------------
// Limits on values read, against a broken or hostile server - a bulk
// string's length, an array's elements and arrays' nesting depth
struct Limits
{
  int64_t max_bulk{512*1024*1024};   // As Redis' proto-max-bulk-len
  int64_t max_elements{16*1024*1024};
  int max_depth{16};
};

//--------------------------------------------------------------------------
// Encode a command as a RESP array of bulk strings, e.g. {"GET", "x"} ->
// "*2\r\n$3\r\nGET\r\n$1\r\nx\r\n"
string encode_command(const vector<string>& args);

//--------------------------------------------------------------------------
// Read a RESP2 value from a stream (lines ending CRLF)
// Returns false on end of stream, or a malformed value or one over the
// limits
bool read_value(istream& in, Value& value, const Limits& limits = {});

//--------------------------------------------------------------------------
// Get a hash's fields from an HGETALL reply (array of alternate field
// names and values)
// Returns false if it isn't an array of an even number of bulk strings
// (e.g. a WRONGTYPE error for a key that isn't a hash)
bool hash_from_value(const Value& value, map<string, string>& fields);

//==========================================================================
// Client errors - connection, protocol, and (from the convenience
// methods) Redis error replies
class Error: public runtime_error
{
public:
  Error(const string& what): runtime_error(what) {}
};

//==========================================================================
// Client options
struct Options
{
  Net::EndPoint server;           // Required
  Time::Duration timeout{10};     // Connect, and each read and write
  string username;                // AUTH with username (Redis 6+) if set
  string password;                // AUTH if set
  int database{0};                // SELECT if not 0
  Limits limits;
};

//==========================================================================
// Redis client - one connection, made when first needed (with AUTH and
// SELECT as configured) and kept.  After a connection or protocol failure
// it is closed, and remade by the next call.  Calls are serialised, so it
// can be shared between threads
class Client
{
  Options options;
  MT::Mutex mutex;
  unique_ptr<Net::TCPClient> socket;
  unique_ptr<Net::TCPStream> stream;

  void connect();
  void write(const string& data);
  Value read_reply();
  Value do_command(const vector<string>& args);
  vector<Value> do_pipeline(const vector<vector<string>>& commands);
  void disconnect();
  [[noreturn]] void fail(const string& what);
  [[noreturn]] void fail_reply(const string& command, const Value& reply);

public:
  //------------------------------------------------------------------------
  // Constructor - doesn't connect
  Client(const Options& _options);

  //------------------------------------------------------------------------
  // Execute a command, e.g. {"HGET", "key", "field"}
  // Returns the reply - including a Redis error reply, as an error value
  // Throws Error on connection or protocol failure
  Value command(const vector<string>& args);

  //------------------------------------------------------------------------
  // Execute several commands, sending them all before reading the replies
  // (in batches of at most 1000, to bound buffering), over one connection
  // Returns the replies, in order, including error values
  // Throws Error on connection or protocol failure
  vector<Value> pipeline(const vector<vector<string>>& commands);

  //------------------------------------------------------------------------
  // Convenience methods - these also throw Error on a Redis error reply

  //------------------------------------------------------------------------
  // Get a string value - nullopt if not set
  optional<string> get(const string& key);

  //------------------------------------------------------------------------
  // Set a string value
  void set(const string& key, const string& value);

  //------------------------------------------------------------------------
  // Get all keys matching a pattern, with SCAN (non-blocking on the
  // server), each once, in the order first returned
  vector<string> scan(const string& pattern, int count = 1000);

  //------------------------------------------------------------------------
  // Get a hash's fields - empty if not set
  map<string, string> hgetall(const string& key);

  //------------------------------------------------------------------------
  // Get several hashes' fields, pipelined - keys which aren't hashes (a
  // WRONGTYPE error reply) or aren't set are left out
  map<string, map<string, string>> hgetall(const vector<string>& keys);

  //------------------------------------------------------------------------
  // Close the connection, if open
  void close();
};

//==========================================================================
}} // namespaces
#endif // !__OBTOOLS_REDIS_H

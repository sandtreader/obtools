//==========================================================================
// ObTools::Redis: client.cc
//
// Redis client - one kept connection, with commands, pipelines and
// convenience methods
//
// Copyright (c) 2026 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-redis.h"
#include <cmath>
#include <set>

namespace ObTools { namespace Redis {

namespace {
  // Most commands pipelined before reading their replies
  const size_t max_pipeline{1000};

  //------------------------------------------------------------------------
  // Get a timeout in whole seconds, at least one
  int timeout_secs(const Time::Duration& timeout)
  {
    const auto secs = static_cast<int>(ceil(timeout.seconds()));
    return secs < 1 ? 1 : secs;
  }
}

//--------------------------------------------------------------------------
// Constructor - doesn't connect
Client::Client(const Options& _options):
  options(_options)
{
}

//--------------------------------------------------------------------------
// Close the connection, without locking
void Client::disconnect()
{
  stream.reset();
  socket.reset();  // Closes it
}

//--------------------------------------------------------------------------
// Close the connection and throw an Error
void Client::fail(const string& what)
{
  disconnect();
  throw Error(what + " (Redis at " + options.server.str() + ")");
}

//--------------------------------------------------------------------------
// Throw an Error for a Redis error (or unexpected) reply to a command,
// keeping the connection - it is still in step
void Client::fail_reply(const string& command, const Value& reply)
{
  if (reply.is_error())
    throw Error("Redis error from " + command + ": " + reply.str
                + " (Redis at " + options.server.str() + ")");
  throw Error("Unexpected reply to " + command + " (Redis at "
              + options.server.str() + ")");
}

//--------------------------------------------------------------------------
// Write data to the connection
void Client::write(const string& data)
{
  try
  {
    socket->write(data);
  }
  catch (const Net::SocketError& e)
  {
    fail("Write failed: " + e.get_string());
  }
}

//--------------------------------------------------------------------------
// Read a reply - an error reply is returned as a value
Value Client::read_reply()
{
  Value value;
  try
  {
    if (read_value(*stream, value, options.limits)) return value;
  }
  catch (const Net::SocketError& e)
  {
    fail("Read failed: " + e.get_string());
  }
  fail("Connection closed, or bad reply");
}

//--------------------------------------------------------------------------
// Connect, if not already, with AUTH and SELECT as configured
void Client::connect()
{
  if (stream) return;

  const auto secs = timeout_secs(options.timeout);
  socket.reset(new Net::TCPClient(options.server, secs));
  if (!*socket) fail("Can't connect");
  socket->set_timeout(secs);
  stream.reset(new Net::TCPStream(*socket));

  if (!options.password.empty())
  {
    vector<string> auth{"AUTH"};
    if (!options.username.empty()) auth.push_back(options.username);
    auth.push_back(options.password);
    write(encode_command(auth));
    const auto reply = read_reply();
    if (reply.is_error()) fail("AUTH failed: " + reply.str);
  }

  if (options.database)
  {
    write(encode_command({"SELECT", to_string(options.database)}));
    const auto reply = read_reply();
    if (reply.is_error()) fail("SELECT failed: " + reply.str);
  }
}

//--------------------------------------------------------------------------
// Execute a command, without locking
Value Client::do_command(const vector<string>& args)
{
  connect();
  write(encode_command(args));
  return read_reply();
}

//--------------------------------------------------------------------------
// Execute several commands, without locking
vector<Value> Client::do_pipeline(const vector<vector<string>>& commands)
{
  vector<Value> replies;
  if (commands.empty()) return replies;
  replies.reserve(commands.size());
  connect();

  // Write a batch of commands, then read its replies - so neither side's
  // buffers grow without limit
  for(size_t start = 0; start < commands.size(); start += max_pipeline)
  {
    const auto end = min(commands.size(), start + max_pipeline);
    string data;
    for(auto i = start; i < end; i++)
      data += encode_command(commands[i]);
    write(data);

    for(auto i = start; i < end; i++)
      replies.push_back(read_reply());
  }

  return replies;
}

//--------------------------------------------------------------------------
// Execute a command
Value Client::command(const vector<string>& args)
{
  MT::Lock lock(mutex);
  return do_command(args);
}

//--------------------------------------------------------------------------
// Execute several commands, pipelined
vector<Value> Client::pipeline(const vector<vector<string>>& commands)
{
  MT::Lock lock(mutex);
  return do_pipeline(commands);
}

//--------------------------------------------------------------------------
// Get a string value - nullopt if not set
optional<string> Client::get(const string& key)
{
  MT::Lock lock(mutex);
  const auto reply = do_command({"GET", key});
  if (reply.is_nil()) return nullopt;
  if (reply.type != Value::Type::bulk) fail_reply("GET", reply);
  return reply.str;
}

//--------------------------------------------------------------------------
// Set a string value
void Client::set(const string& key, const string& value)
{
  MT::Lock lock(mutex);
  const auto reply = do_command({"SET", key, value});
  if (reply.type != Value::Type::status) fail_reply("SET", reply);
}

//--------------------------------------------------------------------------
// Get all keys matching a pattern, with SCAN
vector<string> Client::scan(const string& pattern, int count)
{
  MT::Lock lock(mutex);

  // Keys may be returned more than once while the keyspace changes
  vector<string> keys;
  std::set<string> seen;
  string cursor = "0";
  do
  {
    const auto reply = do_command({"SCAN", cursor, "MATCH", pattern,
                                   "COUNT", to_string(count)});
    if (reply.type != Value::Type::array
        || reply.elements.size() != 2
        || reply.elements[0].type != Value::Type::bulk
        || reply.elements[1].type != Value::Type::array)
      fail_reply("SCAN", reply);

    cursor = reply.elements[0].str;
    for(const auto& k: reply.elements[1].elements)
      if (k.type == Value::Type::bulk && seen.insert(k.str).second)
        keys.push_back(k.str);
  } while (cursor != "0");

  return keys;
}

//--------------------------------------------------------------------------
// Get a hash's fields - empty if not set
map<string, string> Client::hgetall(const string& key)
{
  MT::Lock lock(mutex);
  const auto reply = do_command({"HGETALL", key});
  map<string, string> fields;
  if (!hash_from_value(reply, fields)) fail_reply("HGETALL", reply);
  return fields;
}

//--------------------------------------------------------------------------
// Get several hashes' fields, pipelined
map<string, map<string, string>> Client::hgetall(const vector<string>& keys)
{
  MT::Lock lock(mutex);

  vector<vector<string>> commands;
  commands.reserve(keys.size());
  for(const auto& key: keys)
    commands.push_back({"HGETALL", key});
  const auto replies = do_pipeline(commands);

  map<string, map<string, string>> hashes;
  for(auto i = 0u; i < keys.size(); i++)
  {
    // Not a hash (WRONGTYPE) or not set - left out
    map<string, string> fields;
    if (hash_from_value(replies[i], fields) && !fields.empty())
      hashes[keys[i]] = std::move(fields);
  }
  return hashes;
}

//--------------------------------------------------------------------------
// Close the connection, if open
void Client::close()
{
  MT::Lock lock(mutex);
  disconnect();
}

}} // namespaces

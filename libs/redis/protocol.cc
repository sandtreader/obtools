//==========================================================================
// ObTools::Redis: protocol.cc
//
// RESP2 protocol - encoding commands and reading values
//
// Copyright (c) 2026 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-redis.h"
#include <cerrno>
#include <cstdlib>

namespace ObTools { namespace Redis {

namespace {
  // Longest line (type and status / error / number) we accept - Redis'
  // own are far shorter, so this only guards against a broken server
  const size_t max_line{64*1024};

  // Chunk a bulk string is read in, so a length which the data never
  // arrives for doesn't allocate it all up front
  const int64_t bulk_chunk{64*1024};

  // Most array elements reserved up front, likewise
  const int64_t max_reserve{1024};

  //------------------------------------------------------------------------
  // Read a CRLF terminated line, without the CRLF
  // Returns false at end of stream, without the CR, or if too long
  bool read_line(istream& in, string& line)
  {
    line.clear();
    for(;;)
    {
      char c;
      if (!in.get(c)) return false;
      if (c == '\n') break;
      if (line.size() >= max_line) return false;
      line += c;
    }
    if (line.empty() || line.back() != '\r') return false;
    line.pop_back();
    return true;
  }

  //------------------------------------------------------------------------
  // Parse a signed decimal integer, which must be the whole string
  // Returns whether valid (and in range)
  bool parse_int(const string& s, int64_t& value)
  {
    const auto start = (!s.empty() && s[0] == '-') ? 1u : 0u;
    if (s.size() == start
        || s.find_first_not_of("0123456789", start) != string::npos)
      return false;
    errno = 0;
    value = strtoll(s.c_str(), nullptr, 10);
    return !errno;
  }

  //------------------------------------------------------------------------
  // Read a bulk string's data of the given length, and its CRLF
  // Returns whether all read
  bool read_bulk(istream& in, int64_t length, string& str)
  {
    str.clear();
    while (static_cast<int64_t>(str.size()) < length)
    {
      const auto done = static_cast<int64_t>(str.size());
      const auto n = min(bulk_chunk, length - done);
      str.resize(done + n);
      if (!in.read(&str[done], n)) return false;
    }

    char cr = 0, lf = 0;
    return in.get(cr) && in.get(lf) && cr == '\r' && lf == '\n';
  }

  //------------------------------------------------------------------------
  // Read a value at the given depth of array nesting (0 at the top)
  bool read_value_at(istream& in, Value& value, const Limits& limits,
                     int depth)
  {
    value = Value();

    string line;
    if (!read_line(in, line) || line.empty()) return false;
    const auto rest = line.substr(1);

    switch (line[0])
    {
      case '+':
        value.type = Value::Type::status;
        value.str = rest;
        return true;

      case '-':
        value.type = Value::Type::error;
        value.str = rest;
        return true;

      case ':':
        value.type = Value::Type::integer;
        return parse_int(rest, value.integer);

      case '$':
      {
        // Length delimited, so may hold anything - checked against the
        // limit before reading any of it
        int64_t length = 0;
        if (!parse_int(rest, length) || length < -1
            || length > limits.max_bulk) return false;
        if (length < 0) return true;  // nil
        value.type = Value::Type::bulk;
        return read_bulk(in, length, value.str);
      }

      case '*':
      {
        int64_t count = 0;
        if (!parse_int(rest, count) || count < -1
            || count > limits.max_elements) return false;
        if (count < 0) return true;   // nil
        if (depth >= limits.max_depth) return false;
        value.type = Value::Type::array;
        value.elements.reserve(min(count, max_reserve));
        for(int64_t i = 0; i < count; i++)
        {
          Value element;
          if (!read_value_at(in, element, limits, depth+1)) return false;
          value.elements.push_back(std::move(element));
        }
        return true;
      }

      default:
        return false;
    }
  }
}

//--------------------------------------------------------------------------
// Encode a command as a RESP array of bulk strings
string encode_command(const vector<string>& args)
{
  string resp = "*" + to_string(args.size()) + "\r\n";
  for(const auto& arg: args)
    resp += "$" + to_string(arg.size()) + "\r\n" + arg + "\r\n";
  return resp;
}

//--------------------------------------------------------------------------
// Read a RESP2 value from a stream
// Returns false on end of stream, or a malformed value or one over the
// limits
bool read_value(istream& in, Value& value, const Limits& limits)
{
  return read_value_at(in, value, limits, 0);
}

//--------------------------------------------------------------------------
// Get a hash's fields from an HGETALL reply
bool hash_from_value(const Value& value, map<string, string>& fields)
{
  if (value.type != Value::Type::array || value.elements.size() % 2)
    return false;
  for(const auto& e: value.elements)
    if (e.type != Value::Type::bulk) return false;

  for(auto i = 0u; i < value.elements.size(); i += 2)
    fields[value.elements[i].str] = value.elements[i+1].str;
  return true;
}

}} // namespaces

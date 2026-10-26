# CLAUDE.md - ObTools::Redis Library

## Overview

`ObTools::Redis` is a minimal Redis client - RESP2 encoding and decoding, and a client keeping one TCP connection, with plain commands, pipelines and a few convenience methods (GET, SET, SCAN, HGETALL). Lives under `namespace ObTools::Redis`.

**Header:** `ot-redis.h`
**Dependencies:** `ot-net`, `ot-mt`, `ot-time`

## Key Classes

| Class | Purpose |
|-------|---------|
| `Value` | A RESP2 value: nil, status, error, integer, bulk or array |
| `Limits` | Bounds on values read (bulk length, array elements, nesting depth) |
| `Options` | Client server, timeout, AUTH username/password, database, limits |
| `Client` | Redis client - one lazily made, kept connection, thread safe |
| `Error` | `runtime_error` thrown by `Client` |

## Protocol

```cpp
string encode_command(const vector<string>& args);   // RESP array of bulk strings
bool read_value(istream& in, Value& value, const Limits& limits = {});
bool hash_from_value(const Value& value, map<string, string>& fields);
```

## Client

```cpp
Client(const Options& options);                      // Doesn't connect
Value command(const vector<string>& args);           // Error replies as values
vector<Value> pipeline(const vector<vector<string>>& commands);
optional<string> get(const string& key);             // nullopt if not set
void set(const string& key, const string& value);
vector<string> scan(const string& pattern, int count = 1000);
map<string, string> hgetall(const string& key);
map<string, map<string, string>> hgetall(const vector<string>& keys);
void close();
```

## Notes

- `read_value()` returns false at end of stream, on a malformed value, or one over the `Limits` - bulk lengths and array counts are checked before anything is allocated, and lengths < -1 are malformed
- The connection is made by the first call, with `AUTH [username] password` and `SELECT n` if configured - an error reply to either throws `Error`
- Timeout is in whole seconds (at least 1), for connect and each read/write
- Any socket error, EOF or malformed reply closes the connection and throws `Error` (message includes the server address); the next call reconnects
- `command()` / `pipeline()` return Redis error replies as error values; `get()`, `set()`, `scan()` and `hgetall(key)` throw `Error` on them, keeping the connection
- `pipeline()` writes commands in batches of 1000, reading each batch's replies before the next
- `hgetall(keys)` is pipelined and leaves out keys which aren't hashes (WRONGTYPE) or aren't set
- `scan()` dedups keys, keeping first-seen order
- All public methods lock a mutex, so a `Client` can be shared between threads
- Tests: `test-protocol.cc` (RESP2), `test-client.cc` (against a fake `TCPServer` on ports from 26379)

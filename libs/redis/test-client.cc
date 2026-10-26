//==========================================================================
// ObTools::Redis: test-client.cc
//
// Test harness for Redis client, against a fake server
//
// Copyright (c) 2026 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-redis.h"
#include <functional>
#include <thread>

namespace {

using namespace std;
using namespace ObTools;

typedef vector<string> Command;

//--------------------------------------------------------------------------
// Fake Redis server - reads commands, replies with the handler's raw RESP,
// or drops the connection if it returns "CLOSE"
class FakeServer: public Net::TCPServer
{
  function<string(const Command&)> handler;
  MT::Mutex mutex;
  vector<Command> commands;
  int connections{0};

public:
  FakeServer(int port, function<string(const Command&)> _handler):
    Net::TCPServer(port), handler(_handler) {}

  void process(Net::TCPSocket& s, Net::EndPoint) override
  {
    {
      MT::Lock lock(mutex);
      connections++;
    }

    try
    {
      Net::TCPStream ss(s);
      Redis::Value v;
      while (Redis::read_value(ss, v))
      {
        Command c;
        for(const auto& e: v.elements) c.push_back(e.str);
        {
          MT::Lock lock(mutex);
          commands.push_back(c);
        }
        const auto reply = handler(c);
        if (reply == "CLOSE") return;
        s.write(reply);
      }
    }
    catch (const Net::SocketError&) {}
  }

  vector<Command> get_commands()
  { MT::Lock lock(mutex); return commands; }

  int get_connections()
  { MT::Lock lock(mutex); return connections; }
};

// Fresh port for each server, to avoid lingering binds
int next_port()
{
  static int port = 26379;
  return port++;
}

Redis::Options options(int port)
{
  Redis::Options o;
  o.server = Net::EndPoint(Net::IPAddress("127.0.0.1"), port);
  o.timeout = Time::Duration(2);
  return o;
}

string bulk(const string& s)
{
  return "$" + to_string(s.size()) + "\r\n" + s + "\r\n";
}

// Server running in a thread for the life of the test
struct Running
{
  int port;
  FakeServer server;
  Net::TCPServerThread thread;

  Running(function<string(const Command&)> handler):
    port(next_port()), server(port, handler), thread(server)
  {
    this_thread::sleep_for(chrono::milliseconds{50});
  }

  ~Running() { server.shutdown(); }
};

TEST(RedisClientTest, TestCommandAndConnectionKept)
{
  Running r([](const Command& c) -> string
  {
    if (c[0] == "PING") return "+PONG\r\n";
    if (c[0] == "HGET") return bulk("v-" + c[2]);
    return "-ERR unknown command\r\n";
  });

  {
    Redis::Client client(options(r.port));
    auto v = client.command({"PING"});
    EXPECT_EQ(Redis::Value::Type::status, v.type);
    EXPECT_EQ("PONG", v.str);

    v = client.command({"HGET", "k", "f"});
    EXPECT_EQ("v-f", v.str);

    // Error replies are values, not exceptions
    v = client.command({"NOPE"});
    EXPECT_TRUE(v.is_error());
    EXPECT_EQ("ERR unknown command", v.str);
  }

  EXPECT_EQ(1, r.server.get_connections());
  EXPECT_EQ((vector<Command>{{"PING"}, {"HGET", "k", "f"}, {"NOPE"}}),
            r.server.get_commands());
}

TEST(RedisClientTest, TestAuthAndSelectOnConnect)
{
  Running r([](const Command& c) -> string
  {
    if (c[0] == "AUTH" || c[0] == "SELECT") return "+OK\r\n";
    return ":1\r\n";
  });

  {
    auto o = options(r.port);
    o.username = "rep";
    o.password = "secret";
    o.database = 3;
    Redis::Client client(o);
    EXPECT_EQ(1, client.command({"EXISTS", "x"}).integer);
  }

  EXPECT_EQ((vector<Command>{{"AUTH", "rep", "secret"}, {"SELECT", "3"},
                             {"EXISTS", "x"}}),
            r.server.get_commands());
}

TEST(RedisClientTest, TestAuthWithoutUsername)
{
  Running r([](const Command&) -> string { return "+OK\r\n"; });
  {
    auto o = options(r.port);
    o.password = "secret";
    Redis::Client client(o);
    client.set("a", "b");
  }
  EXPECT_EQ((vector<Command>{{"AUTH", "secret"}, {"SET", "a", "b"}}),
            r.server.get_commands());
}

TEST(RedisClientTest, TestAuthFailureThrows)
{
  Running r([](const Command& c) -> string
  {
    if (c[0] == "AUTH") return "-WRONGPASS invalid password\r\n";
    return "+OK\r\n";
  });
  auto o = options(r.port);
  o.password = "wrong";
  Redis::Client client(o);
  EXPECT_THROW(client.command({"PING"}), Redis::Error);
}

TEST(RedisClientTest, TestConnectionRefusedThrows)
{
  Redis::Client client(options(next_port()));   // Nothing listening
  EXPECT_THROW(client.command({"PING"}), Redis::Error);
}

TEST(RedisClientTest, TestReconnectsAfterDrop)
{
  int n = 0;
  Running r([&n](const Command&) -> string
  {
    return ++n == 2 ? "CLOSE" : "+PONG\r\n";
  });

  Redis::Client client(options(r.port));
  EXPECT_EQ("PONG", client.command({"PING"}).str);
  EXPECT_THROW(client.command({"PING"}), Redis::Error);
  EXPECT_EQ("PONG", client.command({"PING"}).str);
  EXPECT_EQ(2, r.server.get_connections());
}

TEST(RedisClientTest, TestPipeline)
{
  Running r([](const Command& c) -> string
  {
    if (c[0] == "GET") return bulk(c[1] + "!");
    return "-ERR no\r\n";
  });

  Redis::Client client(options(r.port));
  vector<vector<string>> commands;
  for(int i = 0; i < 2500; i++)     // Over a batch
    commands.push_back({"GET", to_string(i)});
  commands.push_back({"BAD"});
  const auto replies = client.pipeline(commands);
  ASSERT_EQ(2501u, replies.size());
  EXPECT_EQ("0!", replies[0].str);
  EXPECT_EQ("2499!", replies[2499].str);
  EXPECT_TRUE(replies[2500].is_error());
  EXPECT_TRUE(client.pipeline({}).empty());
}

TEST(RedisClientTest, TestGetAndSet)
{
  Running r([](const Command& c) -> string
  {
    if (c[0] == "GET") return c[1] == "there" ? bulk("yes") : "$-1\r\n";
    if (c[0] == "SET") return c[1] == "bad" ? "-ERR read only\r\n"
                                            : "+OK\r\n";
    return "-ERR\r\n";
  });

  Redis::Client client(options(r.port));
  EXPECT_EQ("yes", client.get("there").value_or("none"));
  EXPECT_FALSE(client.get("not"));
  client.set("k", "v");
  EXPECT_THROW(client.set("bad", "v"), Redis::Error);
}

TEST(RedisClientTest, TestScan)
{
  // Two pages, with a key repeated (as SCAN may)
  Running r([](const Command& c) -> string
  {
    if (c[0] != "SCAN") return "-ERR\r\n";
    if (c[1] == "0")
      return "*2\r\n" + bulk("17") + "*2\r\n" + bulk("mac_a") + bulk("mac_b");
    return "*2\r\n" + bulk("0") + "*2\r\n" + bulk("mac_b") + bulk("mac_c");
  });

  Redis::Client client(options(r.port));
  EXPECT_EQ((vector<string>{"mac_a", "mac_b", "mac_c"}),
            client.scan("mac_*", 500));
  EXPECT_EQ((vector<Command>{{"SCAN", "0", "MATCH", "mac_*", "COUNT", "500"},
                             {"SCAN", "17", "MATCH", "mac_*", "COUNT", "500"}}),
            r.server.get_commands());
}

TEST(RedisClientTest, TestScanErrorThrows)
{
  Running r([](const Command&) -> string { return "-ERR busy\r\n"; });
  Redis::Client client(options(r.port));
  EXPECT_THROW(client.scan("*"), Redis::Error);
}

TEST(RedisClientTest, TestHGetAll)
{
  Running r([](const Command& c) -> string
  {
    if (c[0] != "HGETALL") return "-ERR\r\n";
    if (c[1] == "h1") return "*4\r\n" + bulk("a") + bulk("1")
                             + bulk("b") + bulk("2");
    if (c[1] == "h2") return "*2\r\n" + bulk("x") + bulk("y");
    if (c[1] == "str") return "-WRONGTYPE Operation against a key\r\n";
    return "*0\r\n";
  });

  Redis::Client client(options(r.port));
  EXPECT_EQ((map<string, string>{{"a", "1"}, {"b", "2"}}),
            client.hgetall("h1"));
  EXPECT_TRUE(client.hgetall("gone").empty());
  EXPECT_THROW(client.hgetall("str"), Redis::Error);

  const auto hashes = client.hgetall(vector<string>{"h1", "str", "gone",
                                                    "h2"});
  ASSERT_EQ(2u, hashes.size());
  EXPECT_EQ("2", hashes.at("h1").at("b"));
  EXPECT_EQ("y", hashes.at("h2").at("x"));
}

TEST(RedisClientTest, TestCloseReconnects)
{
  Running r([](const Command&) -> string { return "+PONG\r\n"; });
  Redis::Client client(options(r.port));
  client.command({"PING"});
  client.close();
  client.command({"PING"});
  EXPECT_EQ(2, r.server.get_connections());
}

} // anonymous namespace

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

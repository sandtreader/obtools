//==========================================================================
// ObTools REST test harness: main.cc
//
// Scriptable multi-threaded REST client
//
// Copyright (c) 2024 Paul Clark
//==========================================================================

#include "ot-log.h"
#include "ot-file.h"
#include "ot-script.h"
#include "ot-cache.h"
#include "ot-web.h"

#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

namespace {
const auto user_agent = "ObTools REST test client";
const auto connection_timeout = 15;
const auto operation_timeout  = 5;
}

using namespace std;
using namespace ObTools;

//==========================================================================
// Globals

typedef Cache::BasicPointerCache<int, Web::HTTPClient> client_cache_t;
client_cache_t clients;  // By session ID
MT::Mutex session_id_mutex;
int session_id = 0;

//==========================================================================
// Open action
class OpenAction: public Script::SingleAction
{
public:
  //------------------------------------------------------------------------
  // Constructor
  OpenAction(Script::Action::CP cp): Script::SingleAction(cp) {}

  //------------------------------------------------------------------------
  // Run action
  bool run(Script::Context& con)
  {
    // Interpolate URL with context vars
    Web::URL url(con.vars.interpolate(xml["url"]));
    bool persistent = xml.get_attr_bool("persistent", true);
    auto client = new Web::HTTPClient(url, 0, user_agent,
                                      connection_timeout, operation_timeout);
    if (persistent) client->enable_persistence();

    // Create session
    int id;
    {
      MT::Lock lock(session_id_mutex);
      id = ++session_id;
    }

    clients.add(id, client);
    Log::Summary log;
    log << "Session " << id << " opened on " << url << endl;

    // Add it to context for other operations
    con.vars.add("session", id);

    // Store the current URL as well, to save repeating it
    con.vars.add("url", url.str());

    return true;
  }
};

Init::NewFactory<Script::Action, OpenAction,
                 Script::Action::CP> open_factory;

//==========================================================================
// Get action
class GetAction: public Script::SingleAction
{
public:
  //------------------------------------------------------------------------
  // Constructor
  GetAction(Script::Action::CP cp): Script::SingleAction(cp) {}

  //------------------------------------------------------------------------
  // Run action
  bool run(Script::Context& con)
  {
    Log::Streams log;
    int session = con.vars.get_int("session");
    auto url_s = con.vars.interpolate(xml["url"]);

    // If not set, default to that set in session
    if (url_s.empty()) url_s = con.vars.get("url");
    Web::URL url(url_s);
    log.detail << "GET " << url << " for session " << session << endl;

    auto client = clients.lookup(session);
    if (client)
    {
      string body;
      auto rc = client->get(url, body);
      if (rc != 200)
      {
        log.error << "GET error: " << rc << ": " << body << endl;
        return false;
      }
      return true;
    }
    else
    {
      log.error << "No HTTP client for session '" << session << "'\n";
      return false;
    }
  }
};

Init::NewFactory<Script::Action, GetAction,
                 Script::Action::CP> get_factory;

//==========================================================================
// Close action
class CloseAction: public Script::SingleAction
{
public:
  //------------------------------------------------------------------------
  // Constructor
  CloseAction(Script::Action::CP cp): Script::SingleAction(cp) {}

  //------------------------------------------------------------------------
  // Run action
  bool run(Script::Context& con)
  {
    Log::Streams log;
    int session = con.vars.get_int("session");
    log.detail << "Closing session " << session << endl;
    clients.remove(session);
    return true;
  }
};

Init::NewFactory<Script::Action, CloseAction,
                 Script::Action::CP> close_factory;

//==========================================================================
// REST Test language
class TestLanguage: public Script::BaseLanguage
{
public:
  //------------------------------------------------------------------------
  // Constructor
  TestLanguage(): Script::BaseLanguage()
  {
    register_action("open",  open_factory);
    register_action("get",   get_factory);
    register_action("close", close_factory);
  }
};

//--------------------------------------------------------------------------
// Print help
void print_help(const std::string& path)
{
  cout << "ObTools REST test client " << VERSION << endl;
  cout << endl;
  cout << "Usage:" << endl;
  cout << "  " << path
       << " [options] [<configuration file>]" << endl << endl;
  cout << "Options:" << endl;
  cout << "  -?  --help           Print this help" << endl;
}

//==========================================================================
// Main
int main(int argc, char **argv)
{
  int i = 1;
  for (; i < argc; ++i)
  {
    string arg = argv[i];
    if (arg[0] != '-') break;

    if (arg == "-?" || arg == "--help")
    {
      print_help(argv[0]);
      return 0;
    }
    else cerr << "Unknown option " << arg << " ignored" << endl;
  }

  // Grab config filename if specified
  XML::Configuration config;
  if (i < argc)
    config.add_file(argv[i]);
  else
    config.add_file("rest-test.cfg.xml");

  // Read config
  if (!config.read("rest-test")) return 2;

  // Set up logging
  auto chan_out = new Log::StreamChannel{&cout};
  const auto log_level = static_cast<Log::Level>(
    config.get_value_int("log/@level", static_cast<int>(Log::Level::summary)));
  const auto time_format = config["log/@timestamp"];
  Log::logger.connect_full(chan_out, log_level, time_format);
  Log::Streams log;

  XML::Element& root = config.get_root();

  // Create our language
  TestLanguage language;

  // Create the script
  Script::Script script(language, root.get_child("script"));

  // Run script
  log.summary << "Starting script\n";
  script.run();
  log.summary << "Script finished\n";

  return 0;
}

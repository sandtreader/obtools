//==========================================================================
// ObTools::FDB: fdb.cc
//
// FoundationDB API implementation
//
// Copyright (c) 2021 Paul Clark.  All rights reserved
//==========================================================================

#include "ot-fdb.h"
#include "ot-log.h"

namespace ObTools { namespace FDB {

// Constructor - initialises API
Client::Client()
{
  if (fdb_select_api_version(FDB_API_VERSION))
  {
    Log::Error log;
    log << "Unable to select FDB API version " << FDB_API_VERSION << endl;
  }
}

// Background network thread
void Client::NetworkThread::run()
{
  Log::Detail log;
  log << "FDB network starting\n";
  if (fdb_run_network())
  {
    Log::Error elog;
    elog << "Failed to run FDB network\n";
  }
  else log << "FDB network stopped\n";
}

// Start the network
bool Client::start()
{
  Log::Streams log;
  log.summary << "Starting FoundationDB client\n";

  if (fdb_setup_network())
  {
    log.error << "Can't setup FDB network\n";
    return false;
  }

  network_thread.start();

  running = true;
  return true;
}

// Stop the network
void Client::stop()
{
  if (running)
  {
    if (fdb_stop_network())
    {
      Log::Error log;
      log << "Can't stop FDB network thread\n";
    }
    network_thread.join();
  }
}


}} // namespaces

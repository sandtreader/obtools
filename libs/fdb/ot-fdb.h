//==========================================================================
// ObTools::FDB: ot-fdb.h
//
// Definition of C++ API for FoundationDB
//
// Copyright (c) 2021 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __OBTOOLS_FDB_H
#define __OBTOOLS_FDB_H

#include <string>
#include <ot-mt.h>

#define FDB_API_VERSION 630
#include "foundationdb/fdb_c.h"

namespace ObTools { namespace FDB {

// Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Singleton client - create this with the lifetime of the application
class Client
{
  bool running = false;

  // Background network thread class
  class NetworkThread: public MT::Thread
  {
    void run();
  };

  NetworkThread network_thread;

public:
  // Constructor - initialises API
  Client();

  // Start the network
  bool start();   // TODO: Take options

  // Stop the network
  void stop();

  // Destructor
  ~Client() { stop(); }
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_FDB_H

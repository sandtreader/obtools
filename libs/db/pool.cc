//==========================================================================
// ObTools::DB: pool.cc
//
// Connection pooling
//
// Copyright (c) 2007 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-db.h"
#include "ot-log.h"
#include <algorithm>

namespace ObTools { namespace DB {

//------------------------------------------------------------------------
// Constructor
ConnectionPool::ConnectionPool(ConnectionFactory& _factory, 
			       unsigned _min, unsigned _max):
  factory(_factory), min_connections(_min), max_connections(_max), mutex()
{
  // Create minimum number of connections
  for(unsigned i=0; i<min_connections; i++)
  {
    Connection *conn = factory.create();
    if (conn)
    {
      if (!!*conn)
      {
	connections.push_back(conn);
	available.push_back(conn);
      }
      else
      {
	delete conn;
	break;
      }
    }
  }
}

//------------------------------------------------------------------------
// Claim a connection
// Returns connection, or 0 if one could not be created or all are active
Connection *ConnectionPool::claim()
{
  MT::Lock lock(mutex);
  Connection *conn;

  // Check if we have one available
  if (available.size())
  {
    conn = available.front();
    available.pop_front();
    return conn;
  }

  // Are we allowed to create any more?
  if (connections.size() < max_connections)
  {
    // Try to create one
    conn = factory.create();
    if (conn)
    {
      if (!!*conn)
      {
	connections.push_back(conn);
	return conn;
      }
      else
      {
	delete conn;
	return 0;
      }
    }
    else return 0;
  }

  Log::Streams log;
  log.error << "Database pool reached maximum size: " << max_connections 
	    << endl;

  return 0;
}

//------------------------------------------------------------------------
// Release a connection after use
void ConnectionPool::release(Connection *conn)
{
  MT::Lock lock(mutex);

  // Check for double release
  if (find(available.begin(), available.end(), conn) != available.end())
    available.push_back(conn);
  else
  {
    Log::Streams log;
    log.error << "Database connection released more than once\n";
  }
}

//------------------------------------------------------------------------
// Destructor
ConnectionPool::~ConnectionPool()
{
  // Delete all connections
  for(list<Connection *>::iterator p = connections.begin(); 
      p!=connections.end(); ++p)
    delete *p;
}

}} // namespaces

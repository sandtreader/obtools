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

//==========================================================================
// Background thread class
class BackgroundThread: public MT::Thread
{
  ConnectionPool& pool;
  virtual void run() { pool.run_background(); }

public:
  BackgroundThread(ConnectionPool& _pool): pool(_pool) { start(); }
};

//------------------------------------------------------------------------
// Constructor
ConnectionPool::ConnectionPool(ConnectionFactory& _factory, 
			       unsigned _min, unsigned _max,
			       Time::Duration _max_inactivity):
  factory(_factory), min_connections(_min), max_connections(_max), 
  max_inactivity(_max_inactivity), mutex(), background_thread(0)
{
  Log::Streams log;
  log.summary << "Creating database connection pool with ("
	      << min_connections << "-" << max_connections 
	      << ") connections, max inactivity " << max_inactivity.seconds() 
	      << endl;

  // Start with base level
  fill_to_minimum();

  // Start background thread
  background_thread = new BackgroundThread(*this);
  background_thread->detach();
}

//------------------------------------------------------------------------
// Create connections to minimum level (call within mutex)
void ConnectionPool::fill_to_minimum()
{
  // Create minimum number of connections
  for(unsigned i=connections.size(); i<min_connections; i++)
  {
    Connection *conn = factory.create();
    if (conn)
    {
      if (!!*conn)
      {
	connections.push_back(conn);
	available.push_back(conn);
	last_used[conn] = Time::Stamp::now();
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
    last_used[conn] = Time::Stamp::now();
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
	last_used[conn] = Time::Stamp::now();
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
  if (find(available.begin(), available.end(), conn) == available.end())
    available.push_back(conn);
  else
  {
    Log::Streams log;
    log.error << "Database connection released more than once\n";
  }
}

//------------------------------------------------------------------------
// Run background timeout loop (called from internal thread)
void ConnectionPool::run_background()
{
  Log::Streams log;

  for(;;)
  {
    { // Not inside sleep
      MT::Lock lock(mutex);
      Time::Stamp now = Time::Stamp::now();

      for(map<Connection *, Time::Stamp>::iterator p = last_used.begin(); 
	  p!=last_used.end();)
      {
	map<Connection *, Time::Stamp>::iterator q = p++;
        Connection *conn = q->first;
	Time::Stamp t = q->second;
	
	if (now-t >= max_inactivity)
	{
	  // Check if it's the available list
	  if (find(available.begin(), available.end(), conn)==available.end())
	  {
	    log.error << "Claimed connection is inactive since " 
		      << t.iso() << " - ignoring\n";
	    // Post back a last_used to suppress errors until next timeout
	    q->second = now;
	  }
	  else
	  {
	    log.detail << "MySQL connection is inactive - reaping\n";

	    last_used.erase(q);
	    connections.remove(conn);
	    available.remove(conn);
	  }
	}
      }

      // Refill in case any deleted
      fill_to_minimum();
    }

    sleep(1);
  }
}

//------------------------------------------------------------------------
// Destructor
ConnectionPool::~ConnectionPool()
{
  // Stop background thread
  if (background_thread) delete background_thread;

  // Delete all connections
  for(list<Connection *>::iterator p = connections.begin(); 
      p!=connections.end(); ++p)
    delete *p;
}

}} // namespaces

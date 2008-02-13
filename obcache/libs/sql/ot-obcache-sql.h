//==========================================================================
// ObTools::ObCache::SQL ot-obcache-sql.h
//
// SQL interface definitions for ObCache
//
// Copyright (c) 2008 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#ifndef __OBTOOLS_OBCACHE_SQL_H
#define __OBTOOLS_OBCACHE_SQL_H

#include "ot-obcache.h"
#include "ot-db.h"
#include "ot-xml.h"

namespace ObTools { namespace ObCache { namespace SQL {

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
/// SQL Storage singleton class
class Storage: public ObCache::Storage
{
  DB::ConnectionPool& db_pool;

public:
  //--------------------------------------------------------------------------
  /// Constructor 
  Storage(DB::ConnectionPool& _db_pool        ///< Database connection pool
	  ):
    db_pool(_db_pool) {}

  //--------------------------------------------------------------------------
  /// Load an object
  Object *load(object_id_t id) throw (Exception);

  //--------------------------------------------------------------------------
  /// Save an object
  void save(Object *ob) throw (Exception);
};

//==========================================================================
/// Storer interface for individual objects
class Storer
{
public:
  //--------------------------------------------------------------------------
  /// Interface to load an object from the given DB connection
  virtual Object *load(object_id_t id, DB::Connection& db) 
    throw (Exception) = 0;

  //--------------------------------------------------------------------------
  /// Interface to save an object to the given DB connection
  virtual void save(Object *ob, DB::Connection& db) throw (Exception) = 0;
};

//==========================================================================
}}} //namespaces
#endif // !__OBTOOLS_OBCACHE_SQL_H




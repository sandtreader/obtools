//==========================================================================
// ObTools::ObCache::SQL ot-obcache-sql.h
//
// SQL interface definitions for ObCache
//
// Copyright (c) 2008 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_OBCACHE_SQL_H
#define __OBTOOLS_OBCACHE_SQL_H

#include "ot-obcache.h"
#include "ot-db.h"
#include "ot-xml.h"

namespace ObTools { namespace ObCache { namespace SQL {

//Make our lives easier without polluting anyone else
using namespace std;

class Storer;  // Forward

//==========================================================================
// Typedefs

typedef uint64_t type_id_t;        ///< Unique type discriminator ID

//==========================================================================
/// SQL Storage singleton class
class Storage: public ObCache::Storage
{
  DB::ConnectionPool& db_pool;       ///< Database connection pool
  map<type_id_t, Storer *> storers;  ///< Map of type ID to storer for it

public:
  //------------------------------------------------------------------------
  /// Constructor
  Storage(DB::ConnectionPool& _db_pool        ///< Database connection pool
          ):
    db_pool(_db_pool) {}

  //------------------------------------------------------------------------
  /// Register a type storer
  void register_storer(type_id_t type, Storer *storer)
  { storers[type] = storer; }

  //------------------------------------------------------------------------
  /// Load an object
  Object *load(object_id_t id) throw (Exception);

  //------------------------------------------------------------------------
  /// Save an object
  void save(Object *ob) throw (Exception);
};

//==========================================================================
/// Storer interface for individual objects
class Storer
{
public:
  //------------------------------------------------------------------------
  /// Interface to load an object from the given DB connection
  virtual Object *load(object_id_t id, DB::AutoConnection& db)
    throw (Exception) = 0;

  //------------------------------------------------------------------------
  /// Interface to save an object to the given DB connection
  virtual void save(Object *ob, DB::AutoConnection& db) throw (Exception) = 0;

  //------------------------------------------------------------------------
  // Virtual destructor
  virtual ~Storer() {}
};

//==========================================================================
}}} //namespaces
#endif // !__OBTOOLS_OBCACHE_SQL_H




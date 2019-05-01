//==========================================================================
// ObTools::ObCache::SQL: storage.cc
//
// Implementation of SQL storage manager
//
// Copyright (c) 2008 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-obcache-sql.h"
#include "ot-log.h"
#include "ot-text.h"

namespace ObTools { namespace ObCache { namespace SQL {

//--------------------------------------------------------------------------
// Load an object
Object *Storage::load(object_id_t id)
{
  // Get DB connection
  DB::AutoConnection db(db_pool);

  // Look up ID in main object table, to get type ref
  string types = db.select_value_by_id64("root", "_type", id, "_id");
  if (types.empty())
    throw Exception("Attempt to load non-existent object "+Text::i64tos(id));

  type_id_t type = Text::stoi64(types);

  // Look up storer interface by type ref
  map<type_id_t, Storer *>::iterator p = storers.find(type);
  if (p == storers.end())
    throw Exception("Attempt to load unknown type "+Text::i64tos(type));

  Storer *storer = p->second;

  // Get storer to load it, using the same DB connection
  return storer->load(id, db);
}

//--------------------------------------------------------------------------
// Save an object
void Storage::save(Object * /*ob*/)
{
  // !!! Get type name from object get_name
  // !!! Look up storer interface
  // !!! Get DB connection
  // !!! Get storer to save it
  throw Exception("Not yet implemented!");
}

}}} // namespaces






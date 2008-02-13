//==========================================================================
// ObTools::ObCache::SQL: storage.cc
//
// Implementation of SQL storage manager
//
// Copyright (c) 2008 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-obcache-sql.h"
#include "ot-log.h"

namespace ObTools { namespace ObCache { namespace SQL {

//--------------------------------------------------------------------------
// Load an object
Object *Storage::load(object_id_t id) throw (Exception)
{
  // !!! Look up ID in main object table, to get type ref/name
  // !!! Look up storer interface by type ref
  // !!! Get DB connection
  // !!! Get storer to load it
  throw Exception("Not yet implemented!");
}

//--------------------------------------------------------------------------
// Save an object
void Storage::save(Object *ob) throw (Exception)
{
  // !!! Get type name from object get_name
  // !!! Look up storer interface
  // !!! Get DB connection
  // !!! Get storer to save it
  throw Exception("Not yet implemented!");
}

}}} // namespaces






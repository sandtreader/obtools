//==========================================================================
// ObTools::ObCache:Core ot-obcache.h
//
// Core definitions for ObCache
//
// Copyright (c) 2008 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#ifndef __OBTOOLS_OBCACHE_H
#define __OBTOOLS_OBCACHE_H

#include "ot-mt.h"
#include "ot-cache.h"

namespace ObTools { namespace ObCache {

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Typedefs
typedef uint64_t object_id_t;  ///< Typedef of unique object ID

//==========================================================================
/// Central object abstraction, inherited by all model roots
class Object
{
public:
  object_id_t id;              ///< Unique object ID

  //--------------------------------------------------------------------------
  /// Constructor
  Object(object_id_t _id): id(_id) {}

  //--------------------------------------------------------------------------
  /// Get type name
  virtual string get_type_name() = 0;
};

//==========================================================================
/// General exception
class Exception
{
public:
  string why;                  ///< Error string

  //--------------------------------------------------------------------------
  /// Constructor
  Exception(const string& _why): why(_why) {}
};

//==========================================================================
/// Interface for something that can load/save a real object
class Storage
{
public:
  //--------------------------------------------------------------------------
  /// Load a real object from an ID (create new object)
  /// Throw Exception if it can't be loaded
  virtual Object *load(object_id_t id) throw (Exception) = 0;

  //--------------------------------------------------------------------------
  /// Save a real object
  /// Throw Exception if it can't be saved
  virtual void save(Object* ob) throw (Exception) = 0;

  //--------------------------------------------------------------------------
  /// Virtual destructor
  virtual ~Storage() {}
};

//==========================================================================
/// Stub object
/// Acts as a placeholder to the real object when associations
/// are lazy-loaded
class Stub
{
  object_id_t id;        ///< Unique object ID
  Storage& storage;      ///< Object loader/saver
  Object *real;          ///< Real object, or 0 if not yet loaded

protected:
  //--------------------------------------------------------------------------
  /// Get real object, loading if necessary
  Object *real_object() throw (Exception)
  {
    if (!real) real = storage.load(id);
    return real;
  }

public:
  //--------------------------------------------------------------------------
  /// Constructor from id and storage
  Stub(object_id_t _id, Storage& _storage): 
    id(_id), storage(_storage), real(0) {}

};

//==========================================================================
/// Object cache
/// Maintains LRU cache of objects in memory
/// Also implements Storage so it can be used by a Stub
class Cache:
  public ObTools::Cache::LRUEvictionPointerCache<object_id_t, Object>, 
  public Storage
{
  Storage& storage;      ///< Real storage 

public:
  //--------------------------------------------------------------------------
  /// Constructor 
  Cache(Storage& _storage, unsigned int _limit=0):
  ObTools::Cache::LRUEvictionPointerCache<object_id_t, Object>::
    LRUEvictionPointerCache(_limit),
    storage(_storage) {}

  //--------------------------------------------------------------------------
  /// Load and cache an object
  Object *load(object_id_t id) throw (Exception)
  {
    // !!! For now, just pass straight to storage and don't cache at all!
    return storage.load(id);
  }

  //--------------------------------------------------------------------------
  /// Save an object from cache
  void save(Object* ob) throw (Exception)
  {
    storage.save(ob);
  }
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_OBCACHE_H




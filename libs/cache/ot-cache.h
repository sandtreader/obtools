//==========================================================================
// ObTools::Misc: ot-cache.h
//
// Public definitions for ObTools::Cache
// General-purpose 'evictor cache' template
// 
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#ifndef __OBTOOLS_CACHE_H
#define __OBTOOLS_CACHE_H

#include "ot-mt.h"

#include <map>
#include <iostream>
#include <time.h>

namespace ObTools { namespace Cache { 

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// General per-item data useful to policies
struct PolicyData
{
  time_t add_time;          // Time added
  time_t use_time;          // Time last used
  unsigned long use_count;  // Number of times used

  // Constructor - set times to now
  PolicyData(): use_count(0)
  {
    time(&add_time);
    use_time = add_time;
  }

  // Useful functions
  void touch()  { time(&use_time); use_count++; }
};

//==========================================================================
// Structure for map values - includes user's content, plus useful
// data for eviction policies
template<class CONTENT> struct MapContent
{
  CONTENT content;                       
  PolicyData policy_data;

  MapContent() {}  // Required for map => CONTENT default constructor
  MapContent(const CONTENT& _content): content(_content) {}
};

//==========================================================================
// Background tidy policy template (abstract interface)
// Called in background to cull 'dead' items
template<class ID, class CONTENT> class TidyPolicy
{
public:
  TidyPolicy() {}

  //--------------------------------------------------------------------------
  // Background check whether to keep an entry
  // Returns true if the entry should be kept
  // Only called for unlocked entries
  // Also passed the current time for efficiency and consistency
  //  - non-time-based checks are free to ignore it!
  virtual bool keep_entry(const PolicyData& pd, time_t now) = 0;
};

//==========================================================================
// Evictor policy template (abstract interface)
// Called to kill off least-wanted entries in cache when full
template<class ID, class CONTENT> class EvictorPolicy
{
public:
  EvictorPolicy() {}

  //--------------------------------------------------------------------------
  // Eviction policy - find the worst entry
  // Called for every unlocked cache entry with current policy data 
  // and 'worst' policy data so far.  Return true if the current one is
  // 'worse' according to your policy than the worst one;  the current
  // will then become the worst for subsequent calls (if any)
  // Initial value of 'worst' is default value (now, no use)
  // The last entry you return true for will be evicted
  // If you never return true, nothing will be evicted
  virtual bool check_worst(const PolicyData& current, 
			   const PolicyData& worst) = 0;
};

//==========================================================================
// Cache Iterator template
// Sugars evil map iterator
template <class ID, class CONTENT> struct CacheIterator
{
  // Standard iterator typedefs
  typedef bidirectional_iterator_tag iterator_category;
  typedef ptrdiff_t difference_type;
  typedef CONTENT value_type;
  typedef CONTENT& reference;
  typedef CONTENT *pointer;

  // Other handy typedefs
  typedef map<ID, MapContent<CONTENT> > MapType;
  typedef typename MapType::iterator MapIteratorType;

  // My state - just uses map's iterator
  MapIteratorType map_iterator;

  // Constructors
  CacheIterator() {}
  CacheIterator(const MapIteratorType& mi): map_iterator(mi) {}
  CacheIterator(const CacheIterator& ci): map_iterator(ci.map_iterator) {}

  // Incrementors - note p++ requires a temporary: use ++p in preference
  CacheIterator& operator++() { map_iterator++; return *this; }
  CacheIterator operator++(int) 
  { CacheIterator t = *this; map_iterator++; return t; }
    
  CacheIterator& operator--() { map_iterator--; return *this; }
  CacheIterator operator--(int) 
  { CacheIterator t = *this; map_iterator--; return t; }

  // Comparators
  bool operator==(const CacheIterator& o) const
  { return map_iterator == o.map_iterator; }
  bool operator!=(const CacheIterator& o) const
  { return map_iterator != o.map_iterator; }

  // Standard dereference
  // * operator gets content ref
  reference operator*() const { return map_iterator->second.content; }
  pointer operator->() const { return &(operator*()); }

  // ID accessor
  const ID& id() const { return map_iterator->first; }
};

//==========================================================================
// Cache template

// Template arguments:
//   ID:              Key of cache map (e.g. string, int) - must be hashable
//   CONTENT:         What we want the cache to hold
//   TIDY_POLICY:     Tidy Policy class (inherits from TidyPolicy<ID, CONTENT>)
//   EVICTOR_POLICY:  Eviction Policy class 
//                      (inherits from EvictorPolicy<ID, CONTENT>)

// At this level, we assume CONTENT is fairly primitive (e.g, string,
// pointer or small struct) and we don't worry too much about copying
// it frequently.  It must also have a default constructor, for maps

// See PointerCache if you want to store something bigger.

template<class ID, class CONTENT, class TIDY_POLICY, class EVICTOR_POLICY> 
  class Cache
{
protected:
  //--------------------------------------------------------------------------
  // Useful internal typedefs
  typedef MapContent<CONTENT> MCType;
  typedef map<ID, MCType> MapType;
  typedef typename MapType::iterator MapIterator;

  //--------------------------------------------------------------------------
  // Internal state

  // Limit on entries, 0 if unlimited
  unsigned int limit;

  // Core map
  MapType cachemap;

  // Policies
  TIDY_POLICY tidy_policy;
  EVICTOR_POLICY evictor_policy;

public:
  // Overall recursive mutex
  MT::RMutex mutex;

  //--------------------------------------------------------------------------
  // Constructor
  Cache(const TIDY_POLICY& _tpol, 
	const EVICTOR_POLICY& _epol,
	unsigned int _limit=0): 
    limit(_limit), tidy_policy(_tpol), evictor_policy(_epol), mutex() {}

  //--------------------------------------------------------------------------
  // Add an item of content to the cache
  // item is COPIED
  // Any existing content under this ID is deleted
  // Whether successful - can fail if limit reached and no eviction possible
  bool add(const ID& id, const CONTENT& content)
  { 
    if (limit && cachemap.size() > limit && !evict()) return false;

    MT::RLock lock(mutex);  // NB Don't lock around evict
    cachemap[id] = MCType(content); 
    return true; 
  }

  //--------------------------------------------------------------------------
  // Check (without copying) whether a given ID exists in the cache
  bool contains(const ID& id)
  { 
    MT::RLock lock(mutex);
    return (cachemap.find(id) != cachemap.end()); 
  }

  //--------------------------------------------------------------------------
  // Get current size of map
  unsigned int size()
  { 
    return cachemap.size();
  }

  //--------------------------------------------------------------------------
  // Returns copy of content of a given ID in the map
  // Whether found - if not, result is not changed
  bool lookup(const ID& id, CONTENT& result)
  {
    MT::RLock lock(mutex);
    MapIterator p = cachemap.find(id);
    if (p != cachemap.end()) result = p->second.content;
  }

  //--------------------------------------------------------------------------
  // Touches an entry, renewing its use-time and incrementing use-count
  // Ignored if ID doesn't exist
  void touch(const ID& id)
  {
    MT::RLock lock(mutex);
    MapIterator p = cachemap.find(id);
    if (p != cachemap.end()) p->second.policy_data.touch();
  }

  //--------------------------------------------------------------------------
  // Remove content of given ID
  virtual void remove(const ID& id) 
  { 
    MT::RLock lock(mutex);
    MapIterator p = cachemap.find(id);
    if (p != cachemap.end()) cachemap.erase(p);
  }

  //--------------------------------------------------------------------------
  // Run background evictor policy
  virtual void tidy() 
  { 
    time_t now = time(0);
    MT::RLock lock(mutex);

    for(MapIterator p = cachemap.begin();
	p!=cachemap.end();)
    {
      MapIterator q=p++;
      MCType &mc = q->second;
      if (!tidy_policy.keep_entry(mc.policy_data, now))
	cachemap.erase(q);
    }
  }

  //--------------------------------------------------------------------------
  // Run emergency evictor policy
  // Evicts until map size is less than limit (room to add one)
  // Returns whether there is now room
  virtual bool evict()
  {
    MT::RLock lock(mutex);

    // Number we need to evict
    unsigned int needed = cachemap.size() - limit + 1;  
    if (needed <= 0) return true;

    while (needed)
    {
      PolicyData worst_data;
      MapIterator worst = cachemap.end();  

      // Show the policy all the entries, let them choose the worst
      for(MapIterator p = cachemap.begin();
	  p!=cachemap.end();
	  ++p)
      {
	MCType &mc = p->second;
	if (evictor_policy.check_worst(mc.policy_data, worst_data))
	{
	  // Keep this as the worst
	  worst = p;
	  worst_data = mc.policy_data;
	}
      }

      // Did we find one?
      if (worst!=cachemap.end())
      {
	cachemap.erase(worst);
	needed--;
      }
      else return false;  // Can't do it
    }

    return true; // Got enough 
  }

  //--------------------------------------------------------------------------
  // Dump contents to given stream
  void dump(ostream& s, bool show_content=false)
  { 
    MT::RLock lock(mutex);
    time_t now = time(0);

    s << "Cache size " << cachemap.size() << ", limit " << limit << ":\n";
    for(MapIterator p = cachemap.begin();
	p!=cachemap.end();
	++p)
    {
      MCType &mc = p->second;
      PolicyData &pd = mc.policy_data;

      s << p->first;
      if (show_content) s << " -> " << mc.content << endl;
      s << " (at=" << pd.add_time-now << 
	", ut=" << pd.use_time-now <<
	", use=" << pd.use_count << ")\n";
    }
  }

  //--------------------------------------------------------------------------
  // Iterators
  // NB! If you use these in a MT environment you must manually lock the mutex 
  // This is also the reason there is no const_iterator - it is never safe
  // to use a const cache, since you won't be able to lock it
  typedef CacheIterator<ID, CONTENT> iterator;
  iterator begin() { return iterator(cachemap.begin()); }
  iterator end() { return iterator(cachemap.end()); }

  //--------------------------------------------------------------------------
  // Clear all content
  virtual void clear() { cachemap.clear(); }

  //--------------------------------------------------------------------------
  // Virtual destructor 
  // (does nothing here, but PointerCache uses it to free pointers)
  virtual ~Cache() {}
};

//==========================================================================
// PointerContent template

// It's tempting to define an auto_ptr-like thing as the CONTENT here,
// rather than a plain pointer, but we then have to worry about every
// assignment etc. within <map>, rather than the much simpler case of
// deleting the pointer when it comes out of the cache.  So we just
// define a simple wrapper:
template<class CONTENT> struct PointerContent
{
  CONTENT *ptr;

  PointerContent(): ptr(0) {} // default constructor for maps
  PointerContent(CONTENT *p): ptr(p) {}
};

// << operator to make dump work - requires that the content itself has
// a << operator, too
template<class CONTENT> 
ostream& operator<<(ostream&s, const PointerContent<CONTENT>& pc)
{
  if (pc.ptr) s << *(pc.ptr);
  return s;
}

//==========================================================================
// PointerCache Iterator template
// Like CacheIterator, but sugars deref
template <class ID, class CONTENT> struct PointerCacheIterator
{
  // Standard iterator typedefs
  typedef CONTENT *value_type;
  typedef CONTENT& reference;
  typedef CONTENT *pointer;

  // Other handy typedefs
  typedef map<ID, MapContent<PointerContent<CONTENT> > > MapType;
  typedef typename MapType::iterator MapIteratorType;

  // My state - just uses map's iterator
  MapIteratorType map_iterator;

  // Constructors
  PointerCacheIterator() {}
  PointerCacheIterator(const MapIteratorType& mi): map_iterator(mi) {}
  PointerCacheIterator(const PointerCacheIterator& ci): 
    map_iterator(ci.map_iterator) {}

  // Incrementors - note p++ requires a temporary: use ++p in preference
  PointerCacheIterator& operator++() { map_iterator++; return *this; }
  PointerCacheIterator operator++(int) 
  { PointerCacheIterator t = *this; map_iterator++; return t; }
    
  PointerCacheIterator& operator--() { map_iterator--; return *this; }
  PointerCacheIterator operator--(int) 
  { PointerCacheIterator t = *this; map_iterator--; return t; }

  // Comparators
  bool operator==(const PointerCacheIterator& o) const
  { return map_iterator == o.map_iterator; }
  bool operator!=(const PointerCacheIterator& o) const
  { return map_iterator != o.map_iterator; }

  // Standard dereference
  // NB: * operator gets content ref
  pointer operator->() const { return map_iterator->second.content.ptr; }
  reference operator*() const { return *(operator->()); }

  // ID accessor
  const ID& id() const { return map_iterator->first; }
};

//==========================================================================
// PointerCache template
// Like Cache, but assuming larger objects requiring dynamic allocation

// Main PointerCache template
template<class ID, class CONTENT, class TIDY_POLICY, class EVICTOR_POLICY> 
  class PointerCache: 
  public Cache<ID, PointerContent<CONTENT>, TIDY_POLICY, EVICTOR_POLICY>  
{
protected:
  //--------------------------------------------------------------------------
  // Useful internal typedefs
  typedef MapContent<PointerContent<CONTENT> > MCType;
  typedef map<ID, MCType> MapType;
  typedef typename MapType::iterator MapIterator;

public:
  //--------------------------------------------------------------------------
  // Constructor
  PointerCache(const TIDY_POLICY& _tpol,
	       const EVICTOR_POLICY& _epol,
	       unsigned int _limit=0): 
    Cache<ID, PointerContent<CONTENT>, 
          TIDY_POLICY, EVICTOR_POLICY>::Cache(_tpol, _epol, _limit) {}

  //--------------------------------------------------------------------------
  // Add an item of content to the cache by pointer
  // item is TAKEN, and will be deleted on exit from the cache
  // Any existing content under this ID is deleted
  // Whether successful - can fail if limit reached and no eviction possible
  bool add(const ID& id, CONTENT *content)
  {
    // Try removing first to ensure deletion of old one
    remove(id);

    // Add PointerContent
    return Cache<ID, PointerContent<CONTENT>, TIDY_POLICY, EVICTOR_POLICY>::
      add(id, PointerContent<CONTENT>(content));
  }

  //--------------------------------------------------------------------------
  // Returns pointer to content of a given ID in the map
  // Pointer found, or 0 if not in cache
  // Pointer returned is owned by cache and will be deleted by it
  CONTENT *lookup(const ID& id)
  {
    MT::RLock lock(mutex);
    MapIterator p = cachemap.find(id);
    if (p != cachemap.end()) 
      return p->second.content.ptr;
    else
      return 0;
  }

  //--------------------------------------------------------------------------
  // Detaches pointer to content of a given ID in the map
  // Pointer found, or 0 if not in cache
  // Pointer returned is detached from cache and should be disposed by caller
  CONTENT *detach(const ID& id)
  {
    MT::RLock lock(mutex);
    MapIterator p = cachemap.find(id);
    if (p != cachemap.end()) 
    {
      CONTENT *r = p->second.content.ptr;
      cachemap.erase(p);
      return r;
    }
    else return 0;
  }

  //--------------------------------------------------------------------------
  // Remove content of given ID
  virtual void remove(const ID& id) 
  { 
    CONTENT *to_delete = 0;

    // Avoid locking around delete to prevent deadlocks if destruction
    // attempts locks itself
    {
      MT::RLock lock(mutex);
      MapIterator p = cachemap.find(id);
      if (p != cachemap.end())
      {
	to_delete = p->second.content.ptr;
	cachemap.erase(p);
      }
    }
	
    if (to_delete) delete(to_delete);
  }

  //--------------------------------------------------------------------------
  // Run background evictor policy
  virtual void tidy() 
  { 
    list<CONTENT *> to_delete;
    time_t now = time(0);

    // Avoid locking around delete
    {
      MT::RLock lock(mutex);

      for(MapIterator p = cachemap.begin();
	  p!=cachemap.end();)
      {
	MapIterator q=p++;
	MCType &mc = q->second;
	if (!tidy_policy.keep_entry(mc.policy_data, now))
	{
	  to_delete.push_back(q->second.content.ptr);
	  cachemap.erase(q);
	}
      }
    }

    // Now do the delete outside the lock
    for(typename list<CONTENT *>::iterator p = to_delete.begin();
	p!=to_delete.end(); ++p)
      delete(*p);
  }

  //--------------------------------------------------------------------------
  // Run emergency evictor policy
  // Evicts until map size is less than limit (room to add one)
  // Returns whether there is now room
  virtual bool evict()
  {
    // Number we need to evict
    unsigned int needed = cachemap.size() - limit + 1;  
    if (needed <= 0) return true;

    while (needed)
    {
      CONTENT *to_delete = 0;
      PolicyData worst_data;

      {
	MT::RLock lock(mutex);
	MapIterator worst = cachemap.end();  

	// Show the policy all the entries, let them choose the worst
	for(MapIterator p = cachemap.begin();
	    p!=cachemap.end();
	    ++p)
	{
	  MCType &mc = p->second;
	  if (evictor_policy.check_worst(mc.policy_data, worst_data))
	  {
	    // Keep this as the worst
	    worst = p;
	    worst_data = mc.policy_data;
	  }
	}

	// Did we find one?
	if (worst!=cachemap.end())
	{
	  to_delete = worst->second.content.ptr;
	  cachemap.erase(worst);
	  needed--;
	}
	else return false;  // Can't do it
      }
      
      // Do delete outside lock
      if (to_delete) delete(to_delete);
    }

    return true; // Got enough 
  }


  //--------------------------------------------------------------------------
  // Iterators
  typedef PointerCacheIterator<ID, CONTENT> iterator;
  iterator begin() { return iterator(cachemap.begin()); }
  iterator end() { return iterator(cachemap.end()); }

  //--------------------------------------------------------------------------
  // Clear all content
  virtual void clear() 
  {
    for(MapIterator p = cachemap.begin(); p!=cachemap.end(); ++p)
      delete(p->second.content.ptr); 
    cachemap.clear(); 
  }

  //--------------------------------------------------------------------------
  // Destructor to clear all entries
  // Has to be here to ensure the right clear() gets called
  ~PointerCache() { clear(); }
};

////////////////////////////////////////////////////////////////////////////
// Policies
////////////////////////////////////////////////////////////////////////////

//==========================================================================
// No tidy policy - does nothing
template<class ID, class CONTENT> class NoTidyPolicy: 
  public TidyPolicy<ID, CONTENT>
{
public:
  NoTidyPolicy() {}

  //------------------------------------------------------------------------
  // Background check whether to keep an entry
  // Always says OK
  bool keep_entry(const PolicyData&, time_t)
  { return true; }
};

//==========================================================================
// No eviction policy - does nothing
template<class ID, class CONTENT> class NoEvictorPolicy: 
  public EvictorPolicy<ID, CONTENT>
{
public:
  NoEvictorPolicy() {}

  //------------------------------------------------------------------------
  // Eviction policy - find the oldest entry
  // Never says this is the 'worst' - hence nothing is ever evicted
  bool check_worst(const PolicyData&, const PolicyData&)
  { return false; }
};

//==========================================================================
// Timeout-since-last-use tidy policy
// Simply removes entries a given time after last use
template<class ID, class CONTENT> class UseTimeoutTidyPolicy: 
  public TidyPolicy<ID, CONTENT>
{
public:
  int timeout;  // In seconds

  UseTimeoutTidyPolicy(int _timeout): timeout(_timeout) {}

  //------------------------------------------------------------------------
  // Background check whether to keep an entry
  // Checks time since last use isn't greater than timeout
  bool keep_entry(const PolicyData& pd, time_t now)
  { return now-pd.use_time < timeout; }
};

//==========================================================================
// Timeout total age tidy policy
// Simply removes entries a given time after creation
template<class ID, class CONTENT> class AgeTimeoutTidyPolicy: 
  public TidyPolicy<ID, CONTENT>
{
public:
  int timeout;  // In seconds

  AgeTimeoutTidyPolicy(int _timeout): timeout(_timeout) {}

  //------------------------------------------------------------------------
  // Background check whether to keep an entry
  // Checks time since addition isn't greater than timeout
  bool keep_entry(const PolicyData& pd, time_t now)
  { return now-pd.add_time < timeout; }
  
};

//==========================================================================
// LRU eviction policy
// Removes least recently used
template<class ID, class CONTENT> class LRUEvictorPolicy: 
  public EvictorPolicy<ID, CONTENT>
{
public:
  LRUEvictorPolicy() {}

  //------------------------------------------------------------------------
  // Eviction policy - find the least recently used entry
  bool check_worst(const PolicyData& current, const PolicyData& worst)
  { return current.use_time < worst.use_time; }
};

//==========================================================================
// Oldest eviction policy
// Removes oldest entry
template<class ID, class CONTENT> class AgeEvictorPolicy: 
  public EvictorPolicy<ID, CONTENT>
{
public:
  AgeEvictorPolicy() {}

  //------------------------------------------------------------------------
  // Eviction policy - find the oldest entry
  bool check_worst(const PolicyData& current, const PolicyData& worst)
  { return current.add_time < worst.add_time; }
};

////////////////////////////////////////////////////////////////////////////
// Standard combinations
////////////////////////////////////////////////////////////////////////////

//==========================================================================
// Use Timeout cache, no eviction
template<class ID, class CONTENT> class UseTimeoutCache:
  public Cache<ID, CONTENT, UseTimeoutTidyPolicy<ID, CONTENT>,
               NoEvictorPolicy<ID, CONTENT> >
{
public:
  UseTimeoutCache(int _timeout, unsigned int _limit = 0): 
    Cache<ID, CONTENT, UseTimeoutTidyPolicy<ID, CONTENT>,
          NoEvictorPolicy<ID,CONTENT> >::Cache
    (UseTimeoutTidyPolicy<ID, CONTENT>(_timeout), 
     NoEvictorPolicy<ID, CONTENT>(), _limit) {}
  void set_timeout(int _timeout) { tidy_policy.timeout = _timeout; }
};

//==========================================================================
// Use Timeout pointer cache, no eviction
template<class ID, class CONTENT> class UseTimeoutPointerCache:
  public PointerCache<ID, CONTENT, UseTimeoutTidyPolicy<ID, CONTENT>,
               NoEvictorPolicy<ID, CONTENT> >
{
public:
  UseTimeoutPointerCache(int _timeout, unsigned int _limit = 0): 
    PointerCache<ID, CONTENT, UseTimeoutTidyPolicy<ID, CONTENT>,
                 NoEvictorPolicy<ID,CONTENT> >::PointerCache
    (UseTimeoutTidyPolicy<ID, CONTENT>(_timeout), 
     NoEvictorPolicy<ID, CONTENT>(), _limit) {}
  void set_timeout(int _timeout) { tidy_policy.timeout = _timeout; }
};

//==========================================================================
// Age Timeout cache, no eviction
template<class ID, class CONTENT> class AgeTimeoutCache:
  public Cache<ID, CONTENT, AgeTimeoutTidyPolicy<ID, CONTENT>,
               NoEvictorPolicy<ID, CONTENT> >
{
public:
  AgeTimeoutCache(int _timeout, unsigned int _limit = 0): 
    Cache<ID, CONTENT, AgeTimeoutTidyPolicy<ID, CONTENT>,
          NoEvictorPolicy<ID,CONTENT> >::Cache
    (AgeTimeoutTidyPolicy<ID, CONTENT>(_timeout), 
     NoEvictorPolicy<ID, CONTENT>(), _limit) {}
  void set_timeout(int _timeout) { tidy_policy.timeout = _timeout; }
};

//==========================================================================
// Age Timeout pointer cache, no eviction
template<class ID, class CONTENT> class AgeTimeoutPointerCache:
  public PointerCache<ID, CONTENT, AgeTimeoutTidyPolicy<ID, CONTENT>,
               NoEvictorPolicy<ID, CONTENT> >
{
public:
  AgeTimeoutPointerCache(int _timeout, unsigned int _limit = 0): 
    PointerCache<ID, CONTENT, AgeTimeoutTidyPolicy<ID, CONTENT>,
                 NoEvictorPolicy<ID,CONTENT> >::PointerCache
    (AgeTimeoutTidyPolicy<ID, CONTENT>(_timeout), 
     NoEvictorPolicy<ID, CONTENT>(), _limit) {}
  void set_timeout(int _timeout) { tidy_policy.timeout = _timeout; }
};

//==========================================================================
// LRU eviction cache, no tidying
template<class ID, class CONTENT> class LRUEvictionCache:
  public Cache<ID, CONTENT, NoTidyPolicy<ID, CONTENT>,
               LRUEvictorPolicy<ID, CONTENT> >
{
public:
  LRUEvictionCache(int _timeout, unsigned int _limit = 0): 
    Cache<ID, CONTENT, NoTidyPolicy<ID, CONTENT>,
          LRUEvictorPolicy<ID,CONTENT> >::Cache
    (NoTidyPolicy<ID, CONTENT>(), LRUEvictorPolicy<ID, CONTENT>(), _limit) {}
};

//==========================================================================
// LRU eviction pointer cache, no tidying
template<class ID, class CONTENT> class LRUEvictionPointerCache:
  public PointerCache<ID, CONTENT, NoTidyPolicy<ID, CONTENT>,
                      LRUEvictorPolicy<ID, CONTENT> >
{
public:
  LRUEvictionPointerCache(int _timeout, unsigned int _limit = 0): 
    PointerCache<ID, CONTENT, NoTidyPolicy<ID, CONTENT>,
                 LRUEvictorPolicy<ID,CONTENT> >::PointerCache
    (NoTidyPolicy<ID, CONTENT>(), LRUEvictorPolicy<ID, CONTENT>(), _limit) {}
};

//==========================================================================
// Age eviction cache, no tidying
template<class ID, class CONTENT> class AgeEvictionCache:
  public Cache<ID, CONTENT, NoTidyPolicy<ID, CONTENT>,
               AgeEvictorPolicy<ID, CONTENT> >
{
public:
  AgeEvictionCache(int _timeout, unsigned int _limit = 0): 
    Cache<ID, CONTENT, NoTidyPolicy<ID, CONTENT>,
          AgeEvictorPolicy<ID,CONTENT> >::Cache
    (NoTidyPolicy<ID, CONTENT>(), AgeEvictorPolicy<ID, CONTENT>(), _limit) {}
};

//==========================================================================
// Age eviction pointer cache, no tidying
template<class ID, class CONTENT> class AgeEvictionPointerCache:
  public PointerCache<ID, CONTENT, NoTidyPolicy<ID, CONTENT>,
                      AgeEvictorPolicy<ID, CONTENT> >
{
public:
  AgeEvictionPointerCache(int _timeout, unsigned int _limit = 0): 
    PointerCache<ID, CONTENT, NoTidyPolicy<ID, CONTENT>,
                 AgeEvictorPolicy<ID,CONTENT> >::PointerCache
    (NoTidyPolicy<ID, CONTENT>(), AgeEvictorPolicy<ID, CONTENT>(), _limit) {}
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_CACHE_H




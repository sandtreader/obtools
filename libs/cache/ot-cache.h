//==========================================================================
// ObTools::Misc: ot-cache.h
//
// Public definitions for ObTools::Cache
// General-purpose 'evictor cache' template
// 
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
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
// Evictor policy class (abstract interface)
class Policy
{
public:
  Policy() {}

  //--------------------------------------------------------------------------
  // Background check whether to keep an entry
  // Returns true if the entry should be kept
  // Only called for unlocked entries
  // Also passed the current time for efficiency and consistency
  //  - non-time-based checks are free to ignore it!
  virtual bool keep_entry(const PolicyData& pd, time_t now) = 0;
  
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
// Cache template

// Template arguments:
//   ID:      Key of cache map (e.g. string, int) - must be hashable
//   CONTENT: What we want the cache to hold
//   POLICY:  Eviction Policy class (inherits from Policy)

// At this level, we assume CONTENT is fairly primitive (e.g, string,
// pointer or small struct) and we don't worry too much about copying
// it frequently.  It must also have a default constructor, for maps

// See PointerCache if you want to store something bigger.

template<class ID, class CONTENT, class POLICY> class Cache
{
protected:
  //--------------------------------------------------------------------------
  // Structure for map values - includes user's content, plus useful
  // data for eviction policies
  struct MapContent
  {
    CONTENT content;                       
    PolicyData policy_data;

    MapContent() {}  // Required for map => CONTENT default constructor
    MapContent(const CONTENT& _content): content(_content) {}
  };

  //--------------------------------------------------------------------------
  // Useful internal typedefs
  typedef map<ID, MapContent> MapType;
  typedef typename MapType::iterator MapIterator;

  //--------------------------------------------------------------------------
  // Internal state

  // Limit on entries, 0 if unlimited
  int limit;

  // Overall mutex
  MT::Mutex mutex;

  // Core map
  MapType cachemap;

  // Policy instance
  POLICY policy;

  //--------------------------------------------------------------------------
  // evict() version (q.v.) to call with mutex locked
  bool evict_locked() 
  { 
    int needed = cachemap.size() - limit + 1;  // Number we need to evict
    if (needed <= 0) return true;

    while (needed)
    {
      PolicyData worst_data;
      MapIterator worst = cachemap.end();  

      // Show the policy all the entries, let them choose the worst
      for(MapIterator p = cachemap.begin();
	  p!=cachemap.end();
	  p++)
      {
	MapContent &mc = p->second;
	if (policy.check_worst(mc.policy_data, worst_data))
	{
	  // Keep this as the worst
	  worst = p;
	  worst_data = mc.policy_data;
	}
      }

      // Did we find one?
      if (worst!=cachemap.end())
      {
	clear(worst->second);
	cachemap.erase(worst);
	needed--;
      }
      else return false;  // Can't do it
    }

    return true; // Got enough 
  }

  //--------------------------------------------------------------------------
  // Clear the given content - does nothing here, but implemented in
  // PointerCache to free pointers
  virtual void clear(const MapContent& mc) {}

public:
  //--------------------------------------------------------------------------
  // Constructor
  Cache(const POLICY& _policy, int _limit=0): 
    policy(_policy), limit(_limit) {}

  //--------------------------------------------------------------------------
  // Add an item of content to the cache
  // item is COPIED
  // Any existing content under this ID is deleted
  // Whether successful - can fail if limit reached and no eviction possible
  bool add(const ID& id, const CONTENT& content)
  { 
    MT::Lock lock(mutex);
    if (limit && cachemap.size() > limit && !evict_locked()) return false;
    cachemap[id] = MapContent(content); 
    return true; 
  }

  //--------------------------------------------------------------------------
  // Check (without copying) whether a given ID exists in the cache
  bool contains(const ID& id)
  { 
    MT::Lock lock(mutex);
    return (cachemap.find(id) != cachemap.end()); 
  }

  //--------------------------------------------------------------------------
  // Returns copy of content of a given ID in the map
  // Whether found - if not, result is not changed
  bool lookup(const ID& id, CONTENT& result)
  {
    MT::Lock lock(mutex);
    MapIterator p = cachemap.find(id);
    if (p != cachemap.end()) result = p->second.content;
  }

  //--------------------------------------------------------------------------
  // Touches an entry, renewing its use-time and incrementing use-count
  // Ignored if ID doesn't exist
  void touch(const ID& id)
  {
    MT::Lock lock(mutex);
    MapIterator p = cachemap.find(id);
    if (p != cachemap.end()) p->second.policy_data.touch();
  }

  //--------------------------------------------------------------------------
  // Remove content of given ID
  void remove(const ID& id) 
  { 
    MT::Lock lock(mutex);
    MapIterator p = cachemap.find(id);
    if (p != cachemap.end())
    {
      clear(p->second);
      cachemap.erase(p);
    }
  }

  //--------------------------------------------------------------------------
  // Run background evictor policy
  void tidy() 
  { 
    MT::Lock lock(mutex);
    time_t now = time(0);

    for(MapIterator p = cachemap.begin();
	p!=cachemap.end();)
    {
      MapContent &mc = p->second;
      if (!policy.keep_entry(mc.policy_data, now))
      {
	MapIterator q=p;
	p++;  // Protect from deletion
	clear(q->second);
	cachemap.erase(q);
      }
      else p++;
    }
  }

  //--------------------------------------------------------------------------
  // Run emergency evictor policy
  // Evicts until map size is less than limit (room to add one)
  // Returns whether there is now room
  bool evict()
  {
    MT::Lock lock(mutex);
    return evict_locked();
  }

  //--------------------------------------------------------------------------
  // Dump contents to given stream
  void dump(ostream& s)
  { 
    MT::Lock lock(mutex);
    time_t now = time(0);

    s << "Cache size " << cachemap.size() << ", limit " << limit << ":\n";
    for(MapIterator p = cachemap.begin();
	p!=cachemap.end();
	p++)
    {
      MapContent &mc = p->second;
      PolicyData &pd = mc.policy_data;

      s << p->first << " -> " << mc.content << endl;
      s << "  (at=" << pd.add_time-now << 
	", ut=" << pd.use_time-now <<
	", use=" << pd.use_count << ")\n";
    }
  }

  //--------------------------------------------------------------------------
  // Virtual destructor - does nothing here, but PointerCache uses it to
  // free pointers
  virtual ~Cache() {}
};

//==========================================================================
// PointerCache template
// Like Cache, but assuming larger objects requiring dynamic allocation

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
}

// Main PointerCache template
template<class ID, class CONTENT, class POLICY> class PointerCache:
  public Cache<ID, PointerContent<CONTENT>, POLICY>  
{
protected:
  // Evil typedefs to allow us to access parent structs/typedefs
  typedef typename 
    Cache<ID, PointerContent<CONTENT>, POLICY>::MapIterator PMapIterator;
  typedef typename 
    Cache<ID, PointerContent<CONTENT>, POLICY>::MapContent PMapContent;

  //--------------------------------------------------------------------------
  // Clear the given content - frees pointer
  virtual void clear(const PMapContent& mc)
  { delete mc.content.ptr; }

public:
  //--------------------------------------------------------------------------
  // Constructor
  PointerCache(const POLICY& _policy, int _limit=0): 
    Cache<ID, PointerContent<CONTENT>, POLICY>::Cache(_policy, _limit) {}

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
    return Cache<ID, PointerContent<CONTENT>, POLICY>::
      add(id, PointerContent<CONTENT>(content));
  }

  //--------------------------------------------------------------------------
  // Returns pointer to content of a given ID in the map
  // Pointer found, or 0 if not in cache
  // Pointer returned is owned by cache and will be deleted by it
  CONTENT *lookup(const ID& id)
  {
    MT::Lock lock(mutex);
    PMapIterator p = cachemap.find(id);
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
    MT::Lock lock(mutex);
    PMapIterator p = cachemap.find(id);
    if (p != cachemap.end()) 
    {
      CONTENT *r = p->second.content.ptr;
      cachemap.erase(p);
      return r;
    }
    else return 0;
  }

  //--------------------------------------------------------------------------
  // Destructor - delete all pointers
  ~PointerCache()
  {
    for(PMapIterator p = cachemap.begin(); p!=cachemap.end(); p++)
      delete p->second.content.ptr; 
  }
};

//==========================================================================
// Manual policy
// Does no eviction automatically
class ManualPolicy: public Policy
{
public:
  ManualPolicy() {}

  //------------------------------------------------------------------------
  // Background check whether to keep an entry
  // Always says OK
  bool keep_entry(const PolicyData& pd, time_t now)
  { return true; }
  
  //------------------------------------------------------------------------
  // Eviction policy - find the oldest entry
  // Never says this is the 'worst' - hence nothing is ever evicted
  bool check_worst(const PolicyData& current, const PolicyData& worst)
  { return false; }
};

//==========================================================================
// Cache template using Manual policy
template<class ID, class CONTENT> class ManualCache: 
  public Cache<ID, CONTENT, ManualPolicy>
{
public:
  ManualCache(int _limit=0): 
    Cache<ID, CONTENT, ManualPolicy>::Cache(ManualPolicy(), _limit) {}
};

//==========================================================================
// PointerCache template using Manual policy
template<class ID, class CONTENT> class ManualPointerCache: 
  public PointerCache<ID, CONTENT, ManualPolicy>
{
public:
  ManualPointerCache(int _limit=0): 
    PointerCache<ID, CONTENT, ManualPolicy>::PointerCache(ManualPolicy(), 
							  _limit) {}
};

//==========================================================================
// Timeout-since-last-use policy
// Simply removes entries a given time after last use
class TimeoutUsePolicy: public Policy
{
private:
  int timeout;  // In seconds

public:
  TimeoutUsePolicy(int _timeout): timeout(_timeout) {}

  //------------------------------------------------------------------------
  // Background check whether to keep an entry
  // Checks time since last use isn't greater than timeout
  bool keep_entry(const PolicyData& pd, time_t now)
  { return now-pd.use_time < timeout; }
  
  //------------------------------------------------------------------------
  // Eviction policy - find the oldest entry
  bool check_worst(const PolicyData& current, const PolicyData& worst)
  { return current.use_time < worst.use_time; }
};

//==========================================================================
// Cache template using TimeoutUse policy
template<class ID, class CONTENT> class TimeoutUseCache: 
  public Cache<ID, CONTENT, TimeoutUsePolicy>
{
public:
  TimeoutUseCache(int _timeout, int _limit=0): 
    Cache<ID, CONTENT, TimeoutUsePolicy>::Cache
    (TimeoutUsePolicy(_timeout), _limit) {}
};

//==========================================================================
// PointerCache template using TimeoutUse policy
template<class ID, class CONTENT> class TimeoutUsePointerCache: 
  public PointerCache<ID, CONTENT, TimeoutUsePolicy>
{
public:
  TimeoutUsePointerCache(int _timeout, int _limit=0): 
    PointerCache<ID, CONTENT, TimeoutUsePolicy>::PointerCache
    (TimeoutUsePolicy(_timeout), _limit) {}
};

//==========================================================================
// Timeout total age policy
// Simply removes entries a given time after creation
class TimeoutAgePolicy: public Policy
{
private:
  int timeout;  // In seconds

public:
  TimeoutAgePolicy(int _timeout): timeout(_timeout) {}

  //------------------------------------------------------------------------
  // Background check whether to keep an entry
  // Checks time since addition isn't greater than timeout
  bool keep_entry(const PolicyData& pd, time_t now)
  { return now-pd.add_time < timeout; }
  
  //------------------------------------------------------------------------
  // Eviction policy - find the oldest entry
  bool check_worst(const PolicyData& current, const PolicyData& worst)
  { return current.use_time < worst.use_time; }
};

//==========================================================================
// Cache template using TimeoutAge policy
template<class ID, class CONTENT> class TimeoutAgeCache: 
  public Cache<ID, CONTENT, TimeoutAgePolicy>
{
public:
  TimeoutAgeCache(int _timeout, int _limit=0): 
    Cache<ID, CONTENT, TimeoutAgePolicy>::Cache
    (TimeoutAgePolicy(_timeout), _limit) {}
};

//==========================================================================
// PointerCache template using TimeoutAge policy
template<class ID, class CONTENT> class TimeoutAgePointerCache: 
  public PointerCache<ID, CONTENT, TimeoutAgePolicy>
{
public:
  TimeoutAgePointerCache(int _timeout, int _limit=0): 
    PointerCache<ID, CONTENT, TimeoutAgePolicy>::PointerCache
    (TimeoutAgePolicy(_timeout), _limit) {}
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_CACHE_H




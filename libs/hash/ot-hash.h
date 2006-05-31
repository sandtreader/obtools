//==========================================================================
// ObTools::Hash: ot-hash.h
//
// Public definitions for ObTools::Hash
//
// Fast ID hash template.  Hashes sparse integer IDs into another integer
// index - e.g. flat table indices, or another sparse ID
//
// Top level is a simple cut of N top bits into a flat table of pointers
// Second level is a coalesced chain hash with an internal freelist
// 
// Performance:
//   Addition: O(1) in all cases
//   Lookup:   O(1) usually, O(N) when full and hashing is perverse
//   Removal:  as lookup
//
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#ifndef __OBTOOLS_HASH_H
#define __OBTOOLS_HASH_H

#if !defined(_SINGLE)
#include "ot-mt.h"
#endif

#include <stdint.h>
#include <map>
#include <iomanip>
#include <sstream>

namespace ObTools { namespace Hash { 

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Types

// Invalid index value marker - implies INDEX_T's must always be signed
const int INVALID_INDEX = -1;

// Invalid hash id marker - implies HASH_INDEX_T's must always be signed, too
const int INVALID_HASH_INDEX = -1;

//==========================================================================
// Individual hash entry template
//   HASH_ID_T :   UNSIGNED integer type for lower part of ID
//   HASH_INDEX_T: SIGNED integer type for index within hash block
//   INDEX_T:      SIGNED integer type for output index
template<class HASH_ID_T, class HASH_INDEX_T, class INDEX_T> struct Entry
{
  union
  {
    HASH_ID_T id;      // Used:   ID (bottom bits only)
    HASH_INDEX_T prev; // Unused: Previous entry in freelist
  };

  HASH_INDEX_T next;  // Chain to next in this bucket or freelist
  INDEX_T index;      // Index into main table, or INVALID_INDEX if unused

  //--------------------------------------------------------------------------
  // Check whether used
  bool used() { return index != INVALID_INDEX; }
};

//==========================================================================
// Statistics
struct Stats
{
  unsigned long entries;   // Total entries
  int max_fullness;        // Maximum fullness of any block
  int max_chain;           // Longest collision chain
};

//==========================================================================
// Hash block template - parameters as above
template<class HASH_ID_T, class HASH_INDEX_T, class INDEX_T> class Block
{
private:
  // Handy typedefs
  typedef Entry<HASH_ID_T, HASH_INDEX_T, INDEX_T> entry_t;

  int size;
  entry_t *table;        
  HASH_INDEX_T freelist;  // First available free entry for collisions
#if !defined(_SINGLE)
  MT::Mutex mutex;  // On freelist state
#endif

public:
  //--------------------------------------------------------------------------
  // Constructor
  Block(int _size): size(_size)
#if !defined(_SINGLE)
  , mutex()
#endif
  {
    int table_size = size*sizeof(entry_t);
    table = (entry_t *)malloc(table_size);

    // Clear it and create freelist
    for(HASH_INDEX_T i=0; i<size; i++)
    {
      entry_t *e = table+i;
      
      // Chain forwards and back, with end markers
      e->next = (i<size-1)?(i+1):INVALID_HASH_INDEX;
      e->prev = i?i-1:INVALID_HASH_INDEX;
      e->index = INVALID_INDEX;
    }
    freelist = 0;
  }

  //--------------------------------------------------------------------------
  // Check constructed OK
  bool ok() { return table!=0; }
  bool operator!() { return !ok(); }

  //--------------------------------------------------------------------------
  // Add an ID (bottom bits only) mapped to the given index
  // Whether successful (can only fail if table is full)
  bool add(HASH_ID_T id, INDEX_T index)
  {
    // Get hash to start with
    HASH_INDEX_T start = (HASH_INDEX_T)(id % size);
    entry_t *p = table+start;

#if !defined(_SINGLE)
    MT::Lock lock(mutex);
#endif

    // Check this entry - if not used, take it, and also snap it out of 
    // the free list
    if (!p->used())
    {
      // Snap out forward pointer
      if (p->next != INVALID_HASH_INDEX)
	table[p->next].id = p->id; 

      // Snap out previous pointer
      if (p->prev != INVALID_HASH_INDEX)
	table[p->prev].next = p->next;
      else // It's the first one
	freelist = p->next;

      // Clear chain and set index
      p->id = id;
      p->next = INVALID_HASH_INDEX;
      p->index = index;  // Set last for atomic update in lookup

      return true;
    }
    else
    {
      // Collision - take freelist entry and chain from current one
      if (freelist == INVALID_HASH_INDEX) return false;
      HASH_INDEX_T qi = freelist;
      entry_t *q = table+qi;

      // Remove 'q' from freelist, and clear back-link from the new head
      freelist = q->next;
      if (freelist != INVALID_HASH_INDEX)
	table[freelist].id = INVALID_HASH_INDEX;

      // Set up q
      q->id = id;
      q->index = index;

      // Splice q into p's forward chain
      q->next = p->next;
      p->next = qi;  // Set last to ensure atomic update in lookup
      
      return true;
    }
  }

  //--------------------------------------------------------------------------
  // Lookup the bottom bits of an ID
  // Returns main table index, or INVALID_INDEX if not found
  INDEX_T lookup(HASH_ID_T id)
  {
    // Get hash to start with
    HASH_INDEX_T start = (HASH_INDEX_T)(id % size);
    entry_t *p = table+start;

    // NB! This is not mutexed, because it doesn't touch the freelist
    // However, changes to block chains must be made carefully to ensure
    // atomic consistency

    // Check for empty start
    if (p->index == INVALID_INDEX) return INVALID_INDEX;

    // Search along chains until we hit or the chain runs out
    for(;;)
    {
      // Check for hit (most likely case)
      if (p->id == id) return p->index;

      // Chain to next
      if (p->next != INVALID_HASH_INDEX)
	p = table+p->next;
      else
	return INVALID_INDEX;
    }
  }

  //--------------------------------------------------------------------------
  // Delete an ID (bottom bits only)
  // Returns old index if found, otherwise INVALID_INDEX
  INDEX_T remove(HASH_ID_T id)
  {
    // Get hash to start with
    HASH_INDEX_T i = (HASH_INDEX_T)(id % size);
    HASH_INDEX_T previous = INVALID_HASH_INDEX;

#if !defined(_SINGLE)
    MT::Lock lock(mutex);
#endif

    // Search along chains until we hit or the chain runs out
    for(;;)
    {
      entry_t *p = table+i;

      // Check for hit (most likely case)
      if (p->id == id)
      {
	// Remember index for result
	INDEX_T index = p->index;

	// Splice out from collision chain...
	// If we're at the head with other things following, we can't leave 
	// this empty, so we pull forward the next one, and delete that instead
	if (previous == INVALID_HASH_INDEX)
	{
	  if (p->next != INVALID_HASH_INDEX)
	  {
	    HASH_INDEX_T qi = p->next;
	    entry_t *q = table+qi;

	    // Pull forward data
	    p->id = q->id;
	    p->next = q->next;
	    p->index = q->index;  // Safe to do with concurrent lookup

	    // Now pretend we're deleting q
	    p = q;
	    i = qi;
	  }
	  else
	  {
	    // At the start, but nothing following - that's OK, this chain
	    // can go empty
	  }
	}
	else
	{
	  // Not at the start - splice out from previous link's next chain
	  table[previous].next = p->next;  // Atomic change for lookup
	}

	// Clear contents and back-link
	p->index = INVALID_INDEX;
	p->prev = INVALID_HASH_INDEX;

	// This becomes the new head of the freelist
	// First back-link the current freelist (if any) to this
	if (freelist != INVALID_HASH_INDEX)
	  table[freelist].prev = i;

	// Now splice to head
	p->next = freelist;
	freelist = i;

	return index;
      }

      // Chain to next
      if (p->next != INVALID_HASH_INDEX)
      {
	previous = i;
	i = p->next;
      }
      else
	return INVALID_INDEX;
    }
  }

  //--------------------------------------------------------------------------
  // Validate internal consistency, dumping errors to given stream
  // Returns whether valid
  bool check(ostream& sout)
  {
    bool ok = true;
    HASH_INDEX_T previous = INVALID_HASH_INDEX;

    // Keep map of visited entries
    map<HASH_INDEX_T, bool> marks;

    if (!table)
    {
      sout << "Table not allocated\n";
      return false;
    }

#if !defined(_SINGLE)
    MT::Lock lock(mutex);
#endif

    // Walk free list, checking forward and back links
    for(HASH_INDEX_T i=freelist; i!=INVALID_HASH_INDEX;)
    {
      // First sanity check pointer
      if (i<0)
      {
	sout << "Freelist goes negative (" << i << ") after " 
	     << previous << endl;
	ok = false;
	break;
      }

      if (i>=size)
      {
	sout << "Freelist goes too large (" << i << ") after " 
	     << previous << endl;
	ok = false;
	break;
      }

      entry_t *p = table+i;
      marks[i] = true;

      // Check this is empty
      if (p->used())
      {
	sout << "Freelist entry at " << i << " is used by index " 
	     << p->index << endl;
	ok = false;
	break;
      }

      // Check we point back to previous
      if (p->prev != previous)
      {
	sout << "Back-pointer misses at " << i << ": expected " << previous
	     << ", got " << p->id << endl;
	ok = false;
      }

      // Get next
      previous = i;
      i = p->next;
    }

    // Now look at every used entry, finding heads of chains
    for(HASH_INDEX_T i=0; i<size; i++)
    {
      entry_t *p = table+i;
      map<HASH_INDEX_T, bool> valid_hashes;

      // See if it's used and hashes to where it should be - 
      // if so, it's the head of a chain
      if (p->used() && p->id % size == i)
      {
	// Follow chain
	for(HASH_INDEX_T j=i; j!=INVALID_HASH_INDEX; j=table[j].next)
	{
	  // Sanity check pointer
	  if (j<0)
	  {
	    sout << "Chain started at " << i 
		 << " goes negative (" << j << ")\n";
	    ok = false;
	    break;
	  }

	  if (j>=size)
	  {
	    sout << "Chain started at " << i 
		 << " goes too large (" <<j<< ")\n";
	    ok = false;
	    break;
	  }

	  entry_t *q = table+j;
	  marks[j] = true;
	  valid_hashes[j] = true;

	  // Check we are used
	  if (!q->used())
	  {
	    sout << "Entry at " << j << " is marked unused\n";
	    ok = false;
	  }

	  // Check we hash correctly, too.  We could validly have the hash
	  // of anything we have seen on the chain, including ourselves
	  if (!valid_hashes[(HASH_INDEX_T)(q->id % size)])
	  {
	    sout << "Entry at " << j << " has wrong hash: " 
		 << (q->id%size) << endl;
	    ok = false;
	  }
	}
      }
    }
    
    // Now check that every entry is marked
    for(HASH_INDEX_T i=0; i<size; i++)
    {
      if (!marks[i])
      {
	sout << "Entry at " << i << " is orphaned\n";
	ok = false;
      }
    }
    
    return ok;
  }

  //--------------------------------------------------------------------------
  // Get statistics on hash block
  void get_stats(Stats& stats_p)
  {
    stats_p.entries = 0;
    stats_p.max_chain = 0;

    for(HASH_INDEX_T i=0; i<size; i++)
    {
      entry_t *p = table+i;

      // See if it's used
      if (p->used())
      {
	stats_p.entries++;

	// If it hashes to where it should be, it's the head of a chain
	if (p->id % size == i)
	{
	  int length = 0;

	  // Follow chain
	  for(HASH_INDEX_T j=i; 
	      j!=INVALID_HASH_INDEX; 
	      length++, j=table[j].next)
	    ;
	  
	  if (length > stats_p.max_chain)
	    stats_p.max_chain = length;
	}
      }
    }
    
    // Calculate fullness
    stats_p.max_fullness = 100*stats_p.entries/size;
  }

  //--------------------------------------------------------------------------
  // Dump block to given output stream 
  void dump(ostream& sout)
  {
#if !defined(_SINGLE)
    MT::Lock lock(mutex);
#endif

    for(HASH_INDEX_T i=0; i<size; i++)
    {
      entry_t *e = table+i;
      sout << setw(8) << i << ": ";
      if (e->used())
      {
	sout << setw(5) << e->id << " -> " 
	     << setw(10) << setiosflags(ios_base::left) << e->index
	     << resetiosflags(ios_base::left);
	if (e->next != INVALID_HASH_INDEX) 
	  sout << " next: " << e->next;
	sout << endl;
      }
      else
      {
	sout << "EMPTY";
	if (i == freelist) sout << " FREELIST";
	if (e->next != INVALID_HASH_INDEX) 
	  sout << " next: " << e->next;
	if (e->prev != INVALID_HASH_INDEX) 
	  sout << " previous: " << e->id;
	sout << endl;
      }
    }
  }

  //--------------------------------------------------------------------------
  // Destructor
  ~Block()
  {
    if (table) free(table);
  }
};

//==========================================================================
// Hash table template - parameters:
//   ID_T:         Unsigned input ID type
//   HASH_ID_T :   UNSIGNED integer type for lower part of ID
//   HASH_INDEX_T: SIGNED integer type for index within hash block
//   INDEX_T:      SIGNED integer type for output index

// Defaults are for a sensible 32 into 16+16 => 32 structure 
template<class ID_T         = uint32_t, 
         class HASH_ID_T    = uint16_t, 
         class HASH_INDEX_T = int16_t,
         class INDEX_T      = int32_t
        > class Table
{
private:
  // Handy typedef
  typedef Block<HASH_ID_T, HASH_INDEX_T, INDEX_T> block_t;
  typedef Entry<HASH_ID_T, HASH_INDEX_T, INDEX_T> entry_t;

  int nbits;                // Number of bits of ID we take
  int block_size;           // Size of hash blocks
  block_t **table;          // Malloc'ed table of (2^bits) Block pointers
  bool built;               // Whether successfully built

  // Precalculated stuff for speed
  int top_shift;            // Shift for top bits     
  ID_T bottom_mask;         // AND mask for bottom bits

public:
  //--------------------------------------------------------------------------
  // Constructor
  // _nbits gives number of bits to chop off at top level
  // _block_size gives number of entries to create at each second-level block
  Table(int _nbits, int _block_size):
    nbits(_nbits), block_size(_block_size), table(0), built(false)
  {
    int nblocks = 1<<nbits;
    int size = nblocks*sizeof(block_t *);
    table = (block_t **)malloc(size);

    if (!table) return;

    // Set shift and mask 
    top_shift   = sizeof(ID_T)*8 - nbits;
    bottom_mask = (ID_T)((1<<top_shift)-1);

    // Clear table for clean shutdown in case it fails
    int i;
    for(i=0; i<nblocks; i++) table[i] = 0;

    // Build table 
    for(i=0; i<nblocks; i++)
    {
      table[i] = new block_t(block_size);
      if (!*table[i]) return;
    }
    
    built = true;
  }

  //--------------------------------------------------------------------------
  // Check constructed OK
  bool ok() { return built; }
  bool operator!() { return !built; }

  //--------------------------------------------------------------------------
  // Get maximum ID bits we can support
  int max_id_bits() { return nbits+sizeof(HASH_ID_T)*8; }

  //--------------------------------------------------------------------------
  // Get total capacity
  unsigned long capacity() { return (1<<nbits)*block_size; }

  //--------------------------------------------------------------------------
  // Get total memory used
  unsigned long memory() 
  { return (1<<nbits)*(sizeof(block_t)+block_size*sizeof(entry_t)); }

  //--------------------------------------------------------------------------
  // Add an ID mapped to the given index
  // Whether successful (can only fail if table is full)
  bool add(ID_T id, INDEX_T index)
  {
    if (!built) return false;

    // Cut into top and bottom
    ID_T top = id >> top_shift;
    HASH_ID_T bottom = (HASH_ID_T)(id & bottom_mask);

    // Get Block and ask it
    block_t *block = table[top];
    return block->add(bottom, index);
  }

  //--------------------------------------------------------------------------
  // Lookup an ID
  // Returns main table index, or INVALID_INDEX if not found
  INDEX_T lookup(ID_T id)
  {
    if (!built) return INVALID_INDEX;

    // Cut into top and bottom
    ID_T top = id >> top_shift;
    HASH_ID_T bottom = (HASH_ID_T)(id & bottom_mask);

    // Get Block and ask it
    block_t *block = table[top];
    return block->lookup(bottom);
  }

  //--------------------------------------------------------------------------
  // Lookup and remove an ID
  // Returns previous main table index, or INVALID_INDEX if not found
  INDEX_T lookup_and_remove(ID_T id)
  {
    if (!built) return INVALID_INDEX;

    // Cut into top and bottom
    ID_T top = id >> top_shift;
    HASH_ID_T bottom = (HASH_ID_T)(id & bottom_mask);

    // Get Block and ask it
    block_t *block = table[top];
    return block->remove(bottom);
  }

  //--------------------------------------------------------------------------
  // Delete an ID (bottom bits only)
  // Returns whether found and deleted
  bool remove(ID_T id)
  { return lookup_and_remove(id) != INVALID_INDEX; }

  //--------------------------------------------------------------------------
  // Validate internal consistency, dumping errors to given stream
  // Returns whether valid
  bool check(ostream& sout)
  {
    bool ok = true;

    for(int i=0; i<(1<<nbits); i++)
    {
      ostringstream oss;  // Accumulate as string so we can mark it
      if (!table[i]->check(oss))
      {
	sout << "Consistency check failed in block " << i << ":\n";
	sout << oss.str();
	sout << "Block dump:\n";
	table[i]->dump(sout);
	ok = false;
      }
    }
    
    return ok;
  }

  //--------------------------------------------------------------------------
  // Get statistics on entire hash table
  void get_stats(Stats& stats_p)
  {
    stats_p.entries = 0;
    stats_p.max_fullness = 0;
    stats_p.max_chain = 0;

    for(int i=0; i<(1<<nbits); i++)
    {
      Stats block_stats;
      table[i]->get_stats(block_stats);

      stats_p.entries += block_stats.entries;
      if (block_stats.max_fullness > stats_p.max_fullness)
	stats_p.max_fullness = block_stats.max_fullness;
      if (block_stats.max_chain > stats_p.max_chain)
	stats_p.max_chain = block_stats.max_chain;
    }
  }

  //--------------------------------------------------------------------------
  // Dump table to given output stream 
  void dump(ostream& sout)
  {
    if (!table)
    {
      sout << "INVALID TABLE!\n";
      return;
    }

    for(int i=0; i<(1<<nbits); i++)
    {
      sout << "--- Block " << i << ":\n";
      table[i]->dump(sout);
    }
  }

  //--------------------------------------------------------------------------
  // Destructor
  ~Table()
  {
    if (table)
    {
      // Delete all blocks
      for(int i=0; i<(1<<nbits); i++)
	if (table[i]) delete table[i];
      
      free(table);
    }
  }
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_HASH_H



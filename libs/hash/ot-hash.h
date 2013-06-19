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
// Copyright (c) 2006 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_HASH_H
#define __OBTOOLS_HASH_H

#if !defined(_SINGLE)
#include "ot-mt.h"
#endif

#include <stdint.h>
#include <stdlib.h>
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
  HASH_ID_T id;       // Used:   ID (bottom bits only)
  HASH_INDEX_T prev;  // Previous entry in chain or freelist

  HASH_INDEX_T next;  // Chain to next in this bucket or freelist
  HASH_INDEX_T head;  // Used: Link to head of my chain (where I hash to)
                      // NB, this can be in the middle of a larger chain
         // Also, set to INVALID_HASH_INDEX indicates an item being deleted
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
// Hash block template - parameters as above, plus helper for ID
// splitting/comparison
template<class HASH_ID_T, class HASH_INDEX_T, class INDEX_T,
	 class HELPER_T> class Block
{
private:
  // Handy typedefs
  typedef Entry<HASH_ID_T, HASH_INDEX_T, INDEX_T> entry_t;

  int size;

  // ID splitting helper
  HELPER_T& helper;

  entry_t *table;        
  HASH_INDEX_T freelist;  // First available free entry for collisions
#if !defined(_SINGLE)
  MT::RWMutex mutex;      // On freelist and chain state
#endif


public:
  //--------------------------------------------------------------------------
  // Constructor
  Block(int _size, HELPER_T& _helper): size(_size), helper(_helper)
#if !defined(_SINGLE)
  , mutex()
#endif
  {
    int table_size = size*sizeof(entry_t);
    table = static_cast<entry_t *>(malloc(table_size));

    // Clear it and create freelist
    for(HASH_INDEX_T i=0; i<size; i++)
    {
      entry_t *e = table+i;
      
      // Chain forwards and back, with end markers
      e->next  = (i<size-1)?(i+1):INVALID_HASH_INDEX;
      e->prev  = i?i-1:INVALID_HASH_INDEX;
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
    HASH_INDEX_T start = helper.get_start(id);
    entry_t *p = table+start;

#if !defined(_SINGLE)
    MT::RWWriteLock lock(mutex);
#endif

    // Check this entry - if not used, take it, and also snap it out of 
    // the free list
    if (!p->used())
    {
      // Snap out forward pointer
      if (p->next != INVALID_HASH_INDEX)
	table[p->next].prev = p->prev; 

      // Snap out previous pointer
      if (p->prev != INVALID_HASH_INDEX)
	table[p->prev].next = p->next;
      else // It's the first one
	freelist = p->next;

      // Clear chain and set index
      p->id    = id;
      p->index = index;  
      p->next  = INVALID_HASH_INDEX;
      p->prev  = INVALID_HASH_INDEX;
      p->head  = start;  // Where we would naturally hash to

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
	table[freelist].prev = INVALID_HASH_INDEX;

      // Set up q
      q->id    = id;
      q->index = index;
      q->prev  = start;  // Back-link to 'p'
      q->head  = start;  // Where we would hash to

      // Splice q into p's forward chain
      q->next = p->next;
      if (p->next != INVALID_HASH_INDEX)
	table[p->next].prev = qi;
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
    HASH_INDEX_T start = helper.get_start(id);
    entry_t *p = table+start;

#if !defined(_SINGLE)
    MT::RWReadLock lock(mutex);
#endif

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
    HASH_INDEX_T i = helper.get_start(id);

#if !defined(_SINGLE)
    MT::RWWriteLock lock(mutex);
#endif

    // Search along chains until we hit or the chain runs out
    for(;;)
    {
      entry_t *p = table+i;

      // If we hit freelist, stop (should only happen in first iteration
      // if the entry doesn't exist, but good safety anyway)
      if (!p->used()) return INVALID_INDEX;

      // Check for hit (most likely case)
      if (p->id == id)
      {
	// Remember index for result
	INDEX_T index = p->index;

	// Splice out from collision chain...
	// We have to be very careful here...  The entry we are splicing out
	// could be at the head of a chain for items which follow later.
	// Because the chain is coalesced, multiple points along it can be
	// heads of chains.  Hence we avoid actually deleting an entry if
	// there is anything that requires it to be present

	// The algorithm here is basically this:
	//   To delete an entry, search forward for any later entry in the
	//   same chain which hashes to the location to be deleted (using prev)
	//   If found:  Replace the deleted entry with it, and delete
	//              (recursively) the new gap
	//   If not found:  Safe to snap out in place

	// Start by trying to delete this one
	HASH_INDEX_T to_delete_i = i;

	// Loop while more to delete (flattening linear recursion on delete)
	for(;;)
	{
	  entry_t *to_delete = table+to_delete_i;

	  // First find 'q', an entry which hashes to here
	  HASH_INDEX_T qi = to_delete->next;

	  while (qi != INVALID_HASH_INDEX)
	  {
	    entry_t *q = table+qi;

	    // Does this hash to the one we're trying to delete?
	    if (q->head == to_delete_i)
	    {
	      // Copy its data back to the one being deleted
	      // note: next/prev remain the same
	      to_delete->index = q->index;
	      to_delete->id    = q->id; 
	      to_delete->head  = q->head;

	      // Now continue and delete this one
	      to_delete_i    = qi;
	      to_delete      = table+qi;
	      break;  // From search on qi
	    }
	    
	    qi = q->next;
	  }

	  // Did we reach the end?
	  if (qi == INVALID_HASH_INDEX)
	  {
	    // Nothing relevant found - we can delete here

	    // Snap out link from previous item in chain
	    if (to_delete->prev != INVALID_HASH_INDEX)
	      table[to_delete->prev].next = to_delete->next;

	    // Ditto for prev link from next item
	    if (to_delete->next != INVALID_HASH_INDEX)
	      table[to_delete->next].prev = to_delete->prev;

	    // Clear contents and back-link
	    to_delete->index = INVALID_INDEX;
	    to_delete->prev  = INVALID_HASH_INDEX;

	    // This becomes the new head of the freelist
	    // First back-link the current freelist (if any) to this
	    if (freelist != INVALID_HASH_INDEX)
	      table[freelist].prev = to_delete_i;

	    // Now splice to head
	    to_delete->next = freelist;
	    freelist = to_delete_i;

	    // Nothing more to delete
	    break;
	  }
	}

	return index;
      }

      // Chain to next
      i = p->next;
      if (i == INVALID_HASH_INDEX) return INVALID_INDEX;
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
    MT::RWReadLock lock(mutex);
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

      // Check for loops
      if (marks[i])
      {
	sout << "Freelist loops back to " << i << " after " << previous
	     << endl;
	ok = false;
	break;
      }

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
      HASH_INDEX_T start = helper.get_start(p->id);

      // See if it's used and hashes to where it should be - 
      // if so, it's the head of a chain
      if (p->used() && start == i)
      {
	HASH_INDEX_T previous = p->prev;  // May be in middle of chain

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

	  // Check we are used
	  if (!q->used())
	  {
	    sout << "Entry at " << j << " is marked unused\n";
	    ok = false;
	  }

	  // Check previous pointer
	  if (q->prev != previous)
	  {
	    sout << "Entry at " << j << " has bad prev link\n";
	    ok = false;
	  }
	  previous = j;

	  // Check our head marker for where we hash to is correct
	  HASH_INDEX_T q_start = helper.get_start(q->id);
	  if (q_start != q->head)
	  {
	    sout << "Entry at " << j << " has bad head marker\n";
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

#if !defined(_SINGLE)
    MT::RWReadLock lock(mutex);
#endif

    for(HASH_INDEX_T i=0; i<size; i++)
    {
      entry_t *p = table+i;

      // See if it's used
      if (p->used())
      {
	stats_p.entries++;

	// If it hashes to where it should be, it's the head of a chain
	if (helper.get_start(p->id) == i)
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
    MT::RWReadLock lock(mutex);
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
	if (e->prev != INVALID_HASH_INDEX) 
	  sout << " prev: " << e->prev;
	sout << " head: " << e->head;
	sout << endl;
      }
      else
      {
	sout << "EMPTY";
	if (i == freelist) sout << " FREELIST";
	if (e->next != INVALID_HASH_INDEX) 
	  sout << " next: " << e->next;
	if (e->prev != INVALID_HASH_INDEX) 
	  sout << " previous: " << e->prev;
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
// General hash table template 

// GeneralTable takes a splitter for ID type, which need not be integral,
// as long as it has an assignment and comparison operator.
// For the more common case of integer IDs, see Table below

// Template parameters:
//   ID_T:         Input ID type
//   HASH_ID_T :   UNSIGNED integer type for lower part of ID
//   HASH_INDEX_T: SIGNED integer type for index within hash block
//   INDEX_T:      SIGNED integer type for output index
//   HELPER_T:     Helper template for ID_T functions

template<class ID_T, class HASH_ID_T, class HASH_INDEX_T,
         class INDEX_T, class HELPER_T> class GeneralTable
{
private:
  // Handy typedef
  typedef Block<HASH_ID_T, HASH_INDEX_T, INDEX_T, HELPER_T> block_t;
  typedef Entry<HASH_ID_T, HASH_INDEX_T, INDEX_T> entry_t;

  int nbits;                // Number of bits of ID we take
  int block_size;           // Size of hash blocks
  block_t **table;          // Malloc'ed table of (2^bits) Block pointers
  bool built;               // Whether successfully built

  // ID splitting helper
  HELPER_T helper;

public:
  //--------------------------------------------------------------------------
  // Constructor
  // _nbits gives number of bits to chop off at top level
  // _block_size gives number of entries to create at each second-level block
  GeneralTable(int _nbits, int _block_size):
    nbits(_nbits), block_size(_block_size), table(0), built(false), 
    helper(_nbits, _block_size)
  {
    int nblocks = 1<<nbits;
    int size = nblocks*sizeof(block_t *);
    table = static_cast<block_t **>(malloc(size));

    if (!table) return;

    // Clear table for clean shutdown in case it fails
    int i;
    for(i=0; i<nblocks; i++) table[i] = 0;

    // Build table 
    for(i=0; i<nblocks; i++)
    {
      table[i] = new block_t(block_size, helper);
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
    int block_no = helper.get_block_no(id);
    HASH_ID_T hash_id = helper.get_hash_id(id);

    // Get Block and ask it
    block_t *block = table[block_no];
    return block->add(hash_id, index);
  }

  //--------------------------------------------------------------------------
  // Lookup an ID
  // Returns main table index, or INVALID_INDEX if not found
  INDEX_T lookup(ID_T id)
  {
    if (!built) return INVALID_INDEX;

    // Cut into top and bottom
    int block_no = helper.get_block_no(id);
    HASH_ID_T hash_id = helper.get_hash_id(id);

    // Get Block and ask it
    block_t *block = table[block_no];
    return block->lookup(hash_id);
  }

  //--------------------------------------------------------------------------
  // Lookup and remove an ID
  // Returns previous main table index, or INVALID_INDEX if not found
  INDEX_T lookup_and_remove(ID_T id)
  {
    if (!built) return INVALID_INDEX;

    // Cut into top and bottom
    int block_no = helper.get_block_no(id);
    HASH_ID_T hash_id = helper.get_hash_id(id);

    // Get Block and ask it
    block_t *block = table[block_no];
    return block->remove(hash_id);
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
  ~GeneralTable()
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
// ID helper template for integral ID types - splits ID into block number
// and hash ID parts
// Note we use the _least significant_ part of the integer for the block
// on the principle that this is likely to be more random than the upper
// parts
// - Implement this for other ID types for GeneralTable
template<class ID_T, class HASH_ID_T, class HASH_INDEX_T> class IntegerIDHelper
{
private:
  int nbits;
  int block_size;

  int block_mask;           // Mask of bottom bits to get block no

public:
  //--------------------------------------------------------------------------
  // Constructor
  IntegerIDHelper(int _nbits, int _block_size): 
    nbits(_nbits), block_size(_block_size)
  {
    block_mask = (1<<nbits)-1;
  }

  //--------------------------------------------------------------------------
  // Get block array index from ID
  int get_block_no(ID_T id) { return static_cast<int>(id & block_mask); }

  //--------------------------------------------------------------------------
  // Get hash ID within block from full ID
  HASH_ID_T get_hash_id(ID_T id) { return static_cast<HASH_ID_T>(id >> nbits); }

  //--------------------------------------------------------------------------
  // Get start offset in hash block from hash ID
  HASH_INDEX_T get_start(HASH_ID_T id) 
  { return static_cast<HASH_INDEX_T>(id % block_size); }

};

//==========================================================================
// Standard hash table template for integer IDs

// Template parameters:
//   ID_T:         Input ID type
//   HASH_ID_T :   UNSIGNED integer type for lower part of ID
//   HASH_INDEX_T: SIGNED integer type for index within hash block
//   INDEX_T:      SIGNED integer type for output index

// Defaults are for a sensible 32 into 16+16 => 32 structure 
template<class ID_T         = uint32_t, 
         class HASH_ID_T    = uint16_t, 
         class HASH_INDEX_T = int16_t,
         class INDEX_T      = int32_t
       > class Table:
  public GeneralTable<ID_T, HASH_ID_T, HASH_INDEX_T, 
                      INDEX_T, 
		      IntegerIDHelper<ID_T, HASH_ID_T, HASH_INDEX_T> >
{
public:
  //--------------------------------------------------------------------------
  // Constructor
  // _nbits gives number of bits to chop off at top level
  // _block_size gives number of entries to create at each second-level block
  Table(int _nbits, int _block_size): 
    GeneralTable<ID_T, HASH_ID_T, HASH_INDEX_T, INDEX_T, 
                 IntegerIDHelper<ID_T, HASH_ID_T, HASH_INDEX_T> 
                >::GeneralTable(_nbits, _block_size) 
  {}
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_HASH_H




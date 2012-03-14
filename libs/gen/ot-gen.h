//==========================================================================
// ObTools::Gen: ot-gen.h
//
// Public definitions for ObTools::Gen
// Generic utility classes
//
// Copyright (c) 2012 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_GEN_H
#define __OBTOOLS_GEN_H

#include "ot-gen.h"

namespace ObTools { namespace Gen {

//==========================================================================
// Shared Pointer
template<class T>
class SharedPointer
{
private:
  unsigned int *ref_count;
  T *pointer;

  //------------------------------------------------------------------------
  // Release our reference to pointer
  // Deletes pointer and reference count if no instances remain
  void release()
  {
    if (!--*ref_count)
    {
      if (pointer)
        delete pointer;
      delete ref_count;
    }
  }

public:
  //------------------------------------------------------------------------
  // Constructor from pointer
  SharedPointer(T *_pointer = 0):
    ref_count(new unsigned int(1)), pointer(_pointer)
  {}

  //------------------------------------------------------------------------
  // Copy constructor
  SharedPointer(const SharedPointer<T>& shared_pointer):
    ref_count(shared_pointer.ref_count), pointer(shared_pointer.pointer)
  {
    ++*ref_count;
  }

  //------------------------------------------------------------------------
  // Assignment operator
  SharedPointer& operator=(const SharedPointer<T>& shared_pointer)
  {
    release();
    ref_count = shared_pointer.ref_count;
    pointer = shared_pointer.pointer;
    ++*ref_count;
    return *this;
  }

  //------------------------------------------------------------------------
  // Pointer operators
  T& operator*() const
  {
    return *pointer;
  }

  T* operator->() const
  {
    return pointer;
  }

  //------------------------------------------------------------------------
  // Destructor
  ~SharedPointer()
  {
    release();
  }
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_GEN_H

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

#include <iostream>

namespace ObTools { namespace Gen {

using namespace std;

//==========================================================================
// Tristate enumeration
enum Tristate
{
  UNSET,
  ON,
  OFF,
};

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
  // Reset function - release and point to something else
  void reset(T *_pointer = 0)
  {
    release();
    ref_count = new unsigned int(1);
    pointer = _pointer;
  }

  //------------------------------------------------------------------------
  // Equality operator
  bool operator==(const SharedPointer<T>& shared_pointer) const
  {
    return pointer == shared_pointer.pointer;
  }

  //------------------------------------------------------------------------
  // Inequality operator
  bool operator!=(const SharedPointer<T>& shared_pointer) const
  {
    return !operator==(shared_pointer);
  }

  //------------------------------------------------------------------------
  // Not operator (check if invalid)
  bool operator!() const
  {
    return !pointer;
  }

  //------------------------------------------------------------------------
  // Less than operator
  bool operator<(const SharedPointer<T>& shared_pointer) const
  {
    return pointer < shared_pointer.pointer;
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
  // Get pointer to target
  T* get() const
  {
    return pointer;
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

//--------------------------------------------------------------------------
// Stream output handler
template<class T>
ostream& operator<<(ostream& os, const SharedPointer<T>& shared_pointer)
{
  return os << "Gen::SharedPointer *" << shared_pointer.get();
}

//==========================================================================
// Unique Pointer
// !! This isn't really a unique pointer, rather just a copy of shared pointer
// !! Replace with std::unique_ptr when the move to C++11 or above happens
template<class T>
class UniquePointer
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
  UniquePointer(T *_pointer = 0):
    ref_count(new unsigned int(1)), pointer(_pointer)
  {}

  //------------------------------------------------------------------------
  // Copy constructor
  UniquePointer(const UniquePointer<T>& unique_pointer):
    ref_count(unique_pointer.ref_count), pointer(unique_pointer.pointer)
  {
    ++*ref_count;
  }

  //------------------------------------------------------------------------
  // Reset function - release and point to something else
  void reset(T *_pointer = 0)
  {
    release();
    ref_count = new unsigned int(1);
    pointer = _pointer;
  }

  //------------------------------------------------------------------------
  // Equality operator
  bool operator==(const UniquePointer<T>& unique_pointer) const
  {
    return pointer == unique_pointer.pointer;
  }

  //------------------------------------------------------------------------
  // Inequality operator
  bool operator!=(const UniquePointer<T>& unique_pointer) const
  {
    return !operator==(unique_pointer);
  }

  //------------------------------------------------------------------------
  // Not operator (check if invalid)
  bool operator!() const
  {
    return !pointer;
  }

  //------------------------------------------------------------------------
  // Less than operator
  bool operator<(const UniquePointer<T>& unique_pointer) const
  {
    return pointer < unique_pointer.pointer;
  }

  //------------------------------------------------------------------------
  // Assignment operator
  UniquePointer& operator=(const UniquePointer<T>& unique_pointer)
  {
    release();
    ref_count = unique_pointer.ref_count;
    pointer = unique_pointer.pointer;
    ++*ref_count;
    return *this;
  }

  //------------------------------------------------------------------------
  // Get pointer to target
  T* get() const
  {
    return pointer;
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
  ~UniquePointer()
  {
    release();
  }
};

//--------------------------------------------------------------------------
// Stream output handler
template<class T>
ostream& operator<<(ostream& os, const UniquePointer<T>& unique_pointer)
{
  return os << "Gen::UniquePointer *" << unique_pointer.get();
}

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_GEN_H

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

#include <string>
#include <iostream>
#include <memory>
#include <algorithm>

//==========================================================================
// Additions to std
// For forthcoming functions that we'd like now
namespace std {

#ifndef __cpp_lib_make_unique
//--------------------------------------------------------------------------
// Make a unique pointer from target class' constructor arguments
template<typename T, typename ...Args>
std::unique_ptr<T> make_unique(Args&& ...args)
{
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
#endif

} // std namespace

namespace ObTools { namespace Gen {

using namespace std;

//==========================================================================
// Tristate enumeration
enum class Tristate
{
  unset,
  on,
  off,
};

//==========================================================================
// Id class
// Basically a restricted version of string
// Put anything in for T as long as it's unique
template<class T>
class Id: public string
{
public:
  //------------------------------------------------------------------------
  // Constructors & copy assignment
  Id()
  {}
  Id(const Id<T>& id):
    string(id)
  {}
  Id& operator=(const Id<T>& id)
  { assign(id); return *this; }
  explicit Id(const string& id):
    string(id)
  {}

  //------------------------------------------------------------------------
  // Validity check by bool cast
  explicit operator bool() const
  {
    return !empty();
  }

private:
  //------------------------------------------------------------------------
  // hide string modification functions
  string& operator+= (const string&) { return *this; }
  string& operator+= (const char*) { return *this; }
  string& operator+= (char) { return *this; }
};

//==========================================================================
// constexpr map template
template <typename K, typename V, size_t S>
struct ConstExprMap {
  array<pair<K, V>, S> data;

  constexpr V lookup(const K& key) const {
    const auto it = find_if(begin(data), end(data),
                            [&key](const auto &d) { return d.first == key; });
    if (it == end(data))
      throw std::range_error("Not found");
    return it->second;
  }

  constexpr K reverse_lookup(const V& value) const {
    const auto it = find_if(begin(data), end(data),
                        [&value](const auto &d) { return d.second == value; });
    if (it == end(data))
      throw std::range_error("Not found");
    return it->first;
  }
};

//==========================================================================
// Bit shift a value in a direction based on sign of value
template <typename T>
T shiftr(const T& value, int places)
{
  if (places >= 0) return value >> places;
  else return value << abs(places);
}
template <typename T>
T shiftl(const T& value, int places)
{
  if (places >= 0) return value << places;
  else return value >> abs(places);
}

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_GEN_H

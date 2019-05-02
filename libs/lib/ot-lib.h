//==========================================================================
// ObTools::Gen: ot-lib.h
//
// Public definitions for ObTools::Lib
// Dynamic library loader
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_LIB_H
#define __OBTOOLS_LIB_H

#if defined(PLATFORM_WINDOWS)
#include <windef.h>
#else
#include <dlfcn.h>
#endif

namespace ObTools { namespace Lib {

using namespace std;

//==========================================================================
// Library class
// A class that represents a dynamic library
class Library
{
private:
#if defined(PLATFORM_WINDOWS)
  HINSTANCE lib = nullptr;
#else
  void *lib = nullptr;
#endif
public:
  //------------------------------------------------------------------------
  // Constructors
  Library(const string& path)
  {
#if defined(PLATFORM_WINDOWS)
    lib = LoadLibrary(path.c_str());
#else
    lib = dlopen(path.c_str(), RTLD_NOW);
#endif
  }

  //------------------------------------------------------------------------
  // Check successfully loaded
  explicit operator bool() const
  {
    return lib;
  }

  //------------------------------------------------------------------------
  // Get a function
  template<typename T>
  T get_function(const string& name) const
  {
    if (!lib)
      return nullptr;
#if defined(PLATFORM_WINDOWS)
    // Windows assigns a type (FARPROC) to the result of GetProcAddress()
    // and GCC warns about casting it away from that
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
    return reinterpret_cast<T>(GetProcAddress(lib, name.c_str()));
#pragma GCC diagnostic pop
#else
    return reinterpret_cast<T>(dlsym(lib, name.c_str()));
#endif
  }

  //------------------------------------------------------------------------
  // Get error
  string get_error() const
  {
#if defined(PLATFORM_WINDOWS)
    return "unknown";
#else
    return dlerror();
#endif
  }

  //------------------------------------------------------------------------
  // Destructor
  ~Library()
  {
    if (!lib)
      return;
#if defined(PLATFORM_WINDOWS)
    FreeLibrary(lib);
#else
    dlclose(lib);
#endif
  }
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_LIB_H

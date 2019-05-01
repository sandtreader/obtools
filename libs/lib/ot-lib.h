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

#ifdef __WIN32__
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
#ifdef __WIN32__
  HINSTANCE lib = nullptr;
#else
  void *lib = nullptr;
#endif
public:
  //------------------------------------------------------------------------
  // Constructors
  Library(const string& path)
  {
#ifdef __WIN32__
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
#ifdef __WIN32__
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
#ifdef __WIN32__
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
#ifdef __WIN32__
    FreeLibrary(lib);
#else
    dlclose(lib);
#endif
  }
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_LIB_H

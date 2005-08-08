//==========================================================================
// ObTools::File: ot-file.h
//
// Public definitions for ObTools::File
// Generally useful portable file handling extensions
//
// Provides all kinds of functionality to do with combining and splitting
// pathnames, getting file information etc. - everything you can do without
// needing to open the file;  for that, use C++ streams!
// 
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#ifndef __OBTOOLS_FILE_H
#define __OBTOOLS_FILE_H

#include <string>
#include <stdint.h>

namespace ObTools { namespace File { 

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Path class
class Path
{
private:
  string path;

#if defined(__WIN32__)
  static const char SEPCHAR = '\\';
#else
  static const char SEPCHAR = '/';
#endif
  static const char EXTCHAR = '.';

public:
  // Constructors-------------------------------------------------------------
  //--------------------------------------------------------------------------
  // Constructor from string
  Path(const string& _path): path(_path) {}

  //--------------------------------------------------------------------------
  // Constructor from directory and leaf
  // If directory is empty or ends with slash already, doesn't add a slash
  Path(const string& dir, const string& leaf);

  //--------------------------------------------------------------------------
  // Constructor from existing path and leaf (combines as above)
  Path(const Path& _path, const string& leaf);

  // Accessors ---------------------------------------------------------------
  //--------------------------------------------------------------------------
  // Get path as string
  const string& str() const { return path; }

  //--------------------------------------------------------------------------
  // Get path as C-string (for fopen etc.)
  const char *c_str() const { return path.c_str(); }

  // Methods to split paths --------------------------------------------------
  //--------------------------------------------------------------------------
  // Find whether it's an absolute path (begins with slash)
  bool is_absolute() const { return !path.empty() && path[0] == SEPCHAR; }

  //--------------------------------------------------------------------------
  // Get directory: everything before last slash, if any, not including
  // trailing slash.  If no slashes, returns empty path
  string dirname() const;

  //--------------------------------------------------------------------------
  // Get leafname: everything after last slash, if any, otherwise everything
  string leafname() const;

  //--------------------------------------------------------------------------
  // Get extension: Part of leafname following dot, if any
  string extension() const;

  //--------------------------------------------------------------------------
  // Get basename: leafname with extension (if any) removed
  string basename() const;
 
  // Methods to resolve paths ------------------------------------------------
  //--------------------------------------------------------------------------
  // Resolve one path against another:
  //   If new path is absolute, return new path
  //   If relative, make absolute path relative to dirname of old path
  Path resolve(const Path& new_path) const;

  // Methods to get file information -----------------------------------------
  //--------------------------------------------------------------------------
  // Does the file exist?
  bool exists() const;

  //--------------------------------------------------------------------------
  // Is it a directory?
  bool is_dir() const;

  //--------------------------------------------------------------------------
  // Is the file readable (by me)?
  bool readable() const;

  //--------------------------------------------------------------------------
  // Is the file writable (by me)?
  bool writeable() const;

  //--------------------------------------------------------------------------
  // Get the file's length
  uint64_t length() const;

  // Methods to manipulate whole files ---------------------------------------
  //--------------------------------------------------------------------------
  // Delete the file/directory (directories are always deleted recursively)
  // Returns whether successful
  bool erase() const;

  //--------------------------------------------------------------------------
  // Rename file to new path
  // New path will be resolved against old one before rename
  // Note: You probably can't rename between filing systems
  bool rename(const Path& new_path) const;
};

//------------------------------------------------------------------------
// << operator to write Path to ostream
ostream& operator<<(ostream& s, const Path& p);

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_FILE_H

















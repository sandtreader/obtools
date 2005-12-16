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
#include <time.h>
#include <sys/types.h>
#include <list>

namespace ObTools { namespace File { 

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Path class
class Path
{
protected:
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
  // Copy constructor
  Path(const Path& _o): path(_o.path) {}

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
 
  //--------------------------------------------------------------------------
  // Get the file's last-modified time (mtime)
  time_t last_modified() const;

  //--------------------------------------------------------------------------
  // Set the file's last-modified time (mtime)
  // Also sets access time to now
  // Returns whether successful
  bool set_last_modified(time_t t) const;

  //--------------------------------------------------------------------------
  // Get the file's mode
  mode_t mode() const;

  //--------------------------------------------------------------------------
  // Set file permissions mode (chmod)
  bool set_mode(mode_t mode) const;

  // String version
  bool set_mode(const string& mode) const { return set_mode(otoi(mode)); }

  //--------------------------------------------------------------------------
  // Get the file's owner
  uid_t owner() const;

  //--------------------------------------------------------------------------
  // Get the file's group
  gid_t group() const;

  //--------------------------------------------------------------------------
  // Get the file's owner & group
  bool set_ownership(uid_t owner, uid_t group) const;

  // String version
  bool set_ownership(const string& owner, const string& group) const
  { return set_ownership(user_name_to_id(owner), group_name_to_id(group)); }

  //--------------------------------------------------------------------------
  // Delete the file/directory (directories are always deleted recursively)
  // Returns whether successful
  bool erase() const;

  //--------------------------------------------------------------------------
  // Touch the file, creating it if not already existing, setting mtime if so
  // Returns whether successful
  bool touch(mode_t mode=0644) const;

  //--------------------------------------------------------------------------
  // Rename file to new path
  // Note: You probably can't rename between filing systems
  bool rename(const Path& new_path) const;

  //--------------------------------------------------------------------------
  //Handy octal conversion functions for file modes
  //Convert integer to octal string
  static string itoo(int mode_i);

  //Convert octal string to integer
  static int otoi(const string& mode_s);

  //--------------------------------------------------------------------------
  //Handy user/group id functions for ownership
  // Get user name from uid
  static string user_id_to_name(uid_t uid);

  // Get user id from name
  static uid_t user_name_to_id(const string& uname);

  // Get group name from gid
  static string group_id_to_name(gid_t gid);

  // Get group id from name
  static gid_t group_name_to_id(const string& gname);
};

//------------------------------------------------------------------------
// << operator to write Path to ostream
ostream& operator<<(ostream& s, const Path& p);

//==========================================================================
// Directory class
// Adds directory iterator to basic Path
class Directory: public Path
{
public:
  //--------------------------------------------------------------------------
  // Constructors, as Path (q.v.) 
  Directory(const Path& _o): Path(_o) {}
  Directory(const string& _path): Path(_path) {}
  Directory(const string& dir, const string& leaf): Path(dir, leaf) {}
  Directory(const Path& _path, const string& leaf): Path(_path, leaf) {}

  //--------------------------------------------------------------------------
  // Ensure a directory path exists
  // With parents set, acts like 'mkdir -p' and creates full path if required
  // Returns whether successful
  bool ensure(bool parents=false, int mode=0777) const;

  //--------------------------------------------------------------------------
  // Get list of directory contents, as leaf strings
  // pattern takes a glob pattern (see Text::pattern_match)
  // If all is set, hidden/dotfiles are returned (including . and ..)
  // Return whether successful (directory readable)
  // Fills in leaves if so
  bool inspect(list<string>& leaves, const string& pattern="*", 
	       bool all=false);

  //--------------------------------------------------------------------------
  // Get list of directory contents, as full paths prefixed by directory path
  // Other parameters as above
  // Returns whether successful (directory readable)
  // Fills in paths if so
  bool inspect(list<Path>& paths, const string& pattern="*", bool all=false);
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_FILE_H

















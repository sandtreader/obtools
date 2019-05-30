//==========================================================================
// ObTools::File: directory.cc
//
// Additional support for directory iteration
//
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-file.h"
#include "ot-text.h"
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#endif

namespace ObTools { namespace File {

//--------------------------------------------------------------------------
// Ensure a directory path exists
// With parents set, acts like 'mkdir -p' and creates full path if required
// Returns whether successful
bool Directory::ensure(bool parents, int mode) const
{
  // Bottom out empty paths
  if (path.empty()) return true;

  // Bottom out existing paths
  if (exists()) return true;

  if (parents)
  {
    // Split directory into leaf and parent dir and recurse to parent
    Directory ppath(dirname());
    if (!ppath.ensure(parents, mode)) return false;
  }

#if defined(PLATFORM_WINDOWS)
  if (_wmkdir(wide_path().c_str())) return false;
  return !_wchmod(wide_path().c_str(), mode);
#else
  int result = ::mkdir(c_str(), mode);
  // Someone else beat us to it maybe!
  if (result)
    return exists();
  else
    return true;
#endif
}

//--------------------------------------------------------------------------
// Get list of directory contents, as leaf strings
// pattern takes a glob pattern (see Text::pattern_match)
// If all is set, hidden/dotfiles are returned (including . and ..)
// Return whether successful (directory readable)
// Fills in leaves if so
bool Directory::inspect(list<string>& leaves, const string& pattern,
                        bool all)
{
#if defined(PLATFORM_WINDOWS)
  // Use full '*' search, so we can use full pattern format without
  // worrying whether Windows implements it natively
  Path pat_path(*this, "*");
  WIN32_FIND_DATAW data;
  HANDLE h = FindFirstFileW(pat_path.wide_path().c_str(), &data);
  if (h != INVALID_HANDLE_VALUE)
  {
    for(;;)
    {
      string fn = wide_to_utf8(data.cFileName);
      if ((all || fn[0] != '.') && Text::pattern_match(pattern, fn))
        leaves.push_back(fn);

      if (!FindNextFileW(h, &data)) break;
    }

    FindClose(h);
  }
#else
  DIR *dir = opendir(c_str());
  if (!dir) return false;

  struct dirent *de;
  while ((de=readdir(dir))!=0)
  {
    if ((all || de->d_name[0] != '.')
     && Text::pattern_match(pattern, de->d_name))
      leaves.push_back(string(de->d_name));
  }

  closedir(dir);
#endif
  return true;
}

//--------------------------------------------------------------------------
// Get list of directory contents, as full paths prefixed by directory path
// Other parameters as above
// Returns whether successful (directory readable)
// Fills in paths if so
bool Directory::inspect(list<Path>& paths, const string& pattern, bool all)
{
  // Get leaves
  list<string> leaves;
  if (!inspect(leaves, pattern, all)) return false;

  // Now convert each leaf into a Path
  for(list<string>::iterator p=leaves.begin(); p!=leaves.end(); ++p)
  {
    string& leaf = *p;
    // Block . and .. even when all is set
    if (!all || (leaf != "." && leaf != ".."))
      paths.push_back(Path(*this, leaf));
  }

  return true;
}

//--------------------------------------------------------------------------
// Get list of directory contents, as full paths prefixed by directory path
// Recursing to subdirectories
// Other parameters as above
// Returns whether successful (directory readable)
// Fills in paths if so
bool Directory::inspect_recursive(list<Path>& paths, const string& pattern,
                                  bool all)
{
  // Look in this directory directly
  if (!inspect(paths, pattern, all)) return false;

  // Now look in all subdirs
  list<Path> subdirs;
  if (!inspect(subdirs)) return true;  // Own listing worked above

  for(const auto& p: subdirs)
  {
    File::Directory dir(p);
    if (dir.is_dir()) dir.inspect_recursive(paths, pattern, all);
  }

  return true;
}

//--------------------------------------------------------------------------
// Does the directory exist and is it actually dir?
bool Directory::exists() const
{
  return Path::exists() && is_dir();
}

//--------------------------------------------------------------------------
// Is the directory empty?
bool Directory::empty() const
{
  bool result(true);

#if defined(PLATFORM_WINDOWS)
  // Use full '*' search, so we can use full pattern format without
  // worrying whether Windows implements it natively
  Path pat_path(*this, "*");
  WIN32_FIND_DATAW data;
  HANDLE h = FindFirstFileW(pat_path.wide_path().c_str(), &data);
  if (h != INVALID_HANDLE_VALUE)
  {
    for(;;)
    {
      string fn = wide_to_utf8(data.cFileName);
      if (fn != "." && fn != "..")
      {
        result = false;
        break;
      }
      if (!FindNextFileW(h, &data)) break;
    }

    FindClose(h);
  }
#else
  DIR *dir = opendir(c_str());
  if (!dir) return false;

  struct dirent *de;
  while ((de=readdir(dir))!=0)
  {
    string fn(de->d_name);
    if (fn != "." && fn != "..")
    {
      result = false;
      break;
    }
  }

  closedir(dir);
#endif

  return result;
}

//--------------------------------------------------------------------------
// Extend a path
Directory& Directory::extend(const string& leaf)
{
  path += SEPCHAR + leaf;
  return *this;
}

//--------------------------------------------------------------------------
// Extend a path
Directory& Directory::extend(const Path& p)
{
  if (p.is_absolute())
    path += p.str();
  else
    path += SEPCHAR + p.str();
  return *this;
}

//--------------------------------------------------------------------------
// Resolve a path against a directory
//   If new path is absolute, return new path
//   If relative, make absolute path relative to within the directory
// Note difference with Path::resolve is that this works from the directory
// path itself, not its parent
Path Directory::resolve(const Path& new_path) const
{
  return Path(path+"/.").resolve(new_path);  // Create fake leaf in path
}


}} // namespaces

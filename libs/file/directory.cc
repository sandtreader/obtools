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

#if defined(__WIN32__)
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
    if (!ppath.ensure()) return false;
  }

#if defined(__WIN32__)
  if (::mkdir(c_str())) return false;
  return !::chmod(c_str(), mode);
#else
  return !::mkdir(c_str(), mode);
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
#if defined(__WIN32__)
  // Use full '*' search, so we can use full pattern format without
  // worrying whether Windows implements it natively
  Path pat_path(*this, "*");
  WIN32_FIND_DATA data;
  HANDLE h = FindFirstFile(pat_path.c_str(), &data);
  if (h != INVALID_HANDLE_VALUE)
  {
    for(;;)
    {
      if ((all || data.cFileName[0] != '.')
       && Text::pattern_match(pattern, data.cFileName))
	leaves.push_back(string(data.cFileName));

      if (!FindNextFile(h, &data)) break;
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


}} // namespaces

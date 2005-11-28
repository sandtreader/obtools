//==========================================================================
// ObTools::File: directory.cc
//
// Additional support for directory iteration
//
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-file.h"
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>

namespace ObTools { namespace File {

#if defined(_WIN32)
#error Sorry, you need to implement these with findfirst/findnext!
#endif

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

  return !::mkdir(c_str(), mode);
}

//--------------------------------------------------------------------------
// Get list of directory contents, as leaf strings
// If all is set, hidden/dotfiles are returned (including . and ..)
// Returns whether successful (directory readable)
// Fills in leaves if so
bool Directory::inspect(list<string>& leaves, bool all)
{
  DIR *dir = opendir(c_str());
  if (!dir) return false;

  struct dirent *de;
  while ((de=readdir(dir))!=0) 
  {
    if (all || de->d_name[0] != '.')
      leaves.push_back(string(de->d_name));
  }

  closedir(dir);
  return true;
}

//--------------------------------------------------------------------------
// Get list of directory contents, as full paths prefixed by directory path
// If all is set, hidden/dotfiles are returned (including . and ..)
// Returns whether successful (directory readable)
// Fills in paths if so
bool Directory::inspect(list<Path>& paths, bool all)
{
  // Get leaves
  list<string> leaves;
  if (!inspect(leaves, all)) return false;

  // Now convert each leaf into a Path
  for(list<string>::iterator p=leaves.begin(); p!=leaves.end(); ++p)
    paths.push_back(Path(*this, *p));

  return true;
}


}} // namespaces

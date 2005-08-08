//==========================================================================
// ObTools::File: path.cc
//
// Wrapper around a file path to provide portability and ease of use of
// file manipulation
//
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-file.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace ObTools { namespace File {

//--------------------------------------------------------------------------
// Get directory: everything before last slash, if any, not including
// trailing slash.  If no slashes, returns empty path
string Path::dirname() const
{
  string::size_type slash = path.rfind(SEPCHAR);
  if (slash == string::npos) return "";
  return string(path, 0, slash);
}

//--------------------------------------------------------------------------
// Get leafname: everything after last slash, if any, otherwise everything
string Path::leafname() const
{
  string::size_type slash = path.rfind(SEPCHAR);
  if (slash == string::npos) return path;
  return string(path, slash+1);
}

//--------------------------------------------------------------------------
// Get extension: Part of leafname following dot, if any
string Path::extension() const
{
  string leaf = leafname();
  string::size_type dot = leaf.rfind(EXTCHAR);
  if (dot == string::npos) return "";
  return string(leaf, dot+1);
}

//--------------------------------------------------------------------------
// Get basename: leafname with extension (if any) removed
string Path::basename() const
{
  string leaf = leafname();
  string::size_type dot = leaf.rfind(EXTCHAR);
  if (dot == string::npos) return leaf;
  return string(leaf, 0, dot);
}

//--------------------------------------------------------------------------
// Resolve one path against another:
//   If new path is absolute, return new path
//   If relative, make absolute path relative to dirname of old path
Path Path::resolve(const Path& new_path) const
{
  if (new_path.is_absolute()) return new_path;
  string dn = dirname();
  string nn = new_path.str();

  // Strip ../ from new path and move up
  while (nn.size() > 3 && nn[0] == '.' && nn[1] == '.' && nn[2] == SEPCHAR)
  {
    nn = string(nn, 3);
    dn = Path(dn).dirname();
  }

  return Path(dn, nn);
}

//--------------------------------------------------------------------------
// Does the file exist?
bool Path::exists() const
{
  struct stat64 sb;  // Don't fail on large files
  return !stat64(c_str(), &sb);
}

//--------------------------------------------------------------------------
// Is it a directory?
bool Path::is_dir() const
{
  struct stat64 sb;
  if (stat64(c_str(), &sb)) return false;
  return S_ISDIR(sb.st_mode); 
}

//--------------------------------------------------------------------------
// Is the file readable (by me)?
bool Path::readable() const
{
  int fd = open(c_str(), O_RDONLY | O_LARGEFILE);
  if (fd < 0) return false;
  close(fd);
  return true;
}

//--------------------------------------------------------------------------
// Is the file writable (by me)?
bool Path::writeable() const
{
  int fd = open(c_str(), O_RDWR | O_LARGEFILE);
  if (fd < 0) return false;
  close(fd);
  return true;
}

//--------------------------------------------------------------------------
// Get the file's length
uint64_t Path::length() const
{
  struct stat64 sb;
  return stat64(c_str(), &sb)?0:sb.st_size;
}

//--------------------------------------------------------------------------
// Delete the file/directory (directories are always deleted recursively)
// Returns whether successful
bool Path::erase() const
{
  return !unlink(c_str());
}

//--------------------------------------------------------------------------
// Rename file to new path
// New path will be resolved against old one before rename
// Note: You probably can't rename between filing systems
bool Path::rename(const Path& new_path) const
{
  Path dest = resolve(new_path);
  return !::rename(c_str(), dest.c_str());  
}

//------------------------------------------------------------------------
// << operator to write Path to ostream
ostream& operator<<(ostream& s, const Path& p) 
{ 
  s<<p.str(); return s; 
}

}} // namespaces

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
#include <utime.h>
#include <pwd.h>
#include <grp.h>
#include <sstream>

namespace ObTools { namespace File {

//--------------------------------------------------------------------------
// Constructor from directory and leaf
// If directory is empty or ends with slash already, doesn't add a slash
Path::Path(const string& dir, const string& leaf)
{
  if (dir.empty() || dir[dir.size()-1] == SEPCHAR)
    path = dir + leaf;
  else
    path = dir + SEPCHAR + leaf;
}

//--------------------------------------------------------------------------
// Constructor from existing path and leaf (combines as above)
Path::Path(const Path& _path, const string& leaf) 
{
  string dir = _path.str();
  if (dir.empty() || dir[dir.size()-1] == SEPCHAR)
    path = dir + leaf;
  else
    path = dir + SEPCHAR + leaf;
}

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
// Get the file's last-modified time (mtime)
time_t Path::last_modified() const
{
  struct stat64 sb;
  return stat64(c_str(), &sb)?0:sb.st_mtime;
}

//--------------------------------------------------------------------------
// Set the file's last-modified time (mtime)
// Also sets access time to now
// Returns whether successful
bool Path::set_last_modified(time_t t) const
{
  struct utimbuf utb;
  utb.actime = time(NULL);  // Now
  utb.modtime = t;
  return !utime(c_str(), &utb); 
}

//--------------------------------------------------------------------------
// Get the file's mode
mode_t Path::mode() const
{
  struct stat64 sb;
  return stat64(c_str(), &sb)?0:sb.st_mode;
}

//--------------------------------------------------------------------------
// Set file permissions mode (chmod)
bool Path::set_mode(mode_t mode) const
{
  return !chmod(c_str(), mode);
}

//--------------------------------------------------------------------------
// Get the file's owner
uid_t Path::owner() const
{
  struct stat64 sb;
  return stat64(c_str(), &sb)?0:sb.st_uid;
}

//--------------------------------------------------------------------------
// Get the file's group
gid_t Path::group() const
{
  struct stat64 sb;
  return stat64(c_str(), &sb)?0:sb.st_gid;
}

//--------------------------------------------------------------------------
// Get the file's owner & group
bool Path::set_ownership(uid_t owner, uid_t group) const
{
  return !chown(c_str(), owner, group);
}

//--------------------------------------------------------------------------
// Delete the file/directory (directories are always deleted recursively)
// Returns whether successful
bool Path::erase() const
{
  if (is_dir())
    return !system((string("rm -rf ")+path).c_str());
  else
    return !unlink(c_str());
}

//--------------------------------------------------------------------------
// Rename file to new path
// Note: You probably can't rename between filing systems
bool Path::rename(const Path& new_path) const
{
  return !::rename(c_str(), new_path.c_str());  
}

//--------------------------------------------------------------------------
// Ensure a directory path exists
// With parents set, acts like 'mkdir -p' and creates full path if required
// Returns whether successful
bool Path::ensure(bool parents, int mode) const
{
  // Bottom out empty paths
  if (path.empty()) return true;

  // Bottom out existing paths
  if (exists()) return true;

  if (parents)
  {
    // Split directory into leaf and parent dir and recurse to parent
    Path ppath(dirname());
    if (!ppath.ensure()) return false;
  }

  return !::mkdir(c_str(), mode);
}

//--------------------------------------------------------------------------
// Convert integer to octal string
string Path::itoo(int mode_i)
{
  ostringstream oss;
  oss << oct << mode_i;
  return oss.str();
}

//--------------------------------------------------------------------------
// Convert octal string to integer
int Path::otoi(const string& mode_s)
{
  int n=0;
  for(string::const_iterator p=mode_s.begin(); p!=mode_s.end(); p++)
    n=n*8+(*p-'0');
  return n;
}

//--------------------------------------------------------------------------
// Get user name from uid
string Path::user_id_to_name(uid_t uid)
{
  // Painful reentrant way of doing this!
  int buflen = sysconf(_SC_GETPW_R_SIZE_MAX);
  char *buf = (char *)malloc(buflen);
  if (!buf) abort();

  struct passwd user;
  struct passwd *uptr;
  int rc = getpwuid_r(uid, &user, buf, buflen, &uptr);
  if (rc || !uptr)
  {
    free(buf);
    return "UNKNOWN";
  }

  string name(uptr->pw_name);
  free(buf);
  return name;
}

//--------------------------------------------------------------------------
// Get user id from name
uid_t Path::user_name_to_id(const string& uname)
{
  // Even more painful reentrant way of doing this, given we never use
  // the name!
  int buflen = sysconf(_SC_GETPW_R_SIZE_MAX);
  char *buf = (char *)malloc(buflen);
  if (!buf) abort();

  struct passwd user;
  struct passwd *uptr;
  int rc = getpwnam_r(uname.c_str(), &user, buf, buflen, &uptr);
  free(buf);  // We don't use it

  if (rc || !uptr) return 0;
  return uptr->pw_uid;
}

//--------------------------------------------------------------------------
// Get group name from gid
string Path::group_id_to_name(gid_t gid)
{
  // Painful reentrant way of doing this!
  int buflen = sysconf(_SC_GETGR_R_SIZE_MAX);
  char *buf = (char *)malloc(buflen);
  if (!buf) abort();

  struct group group;
  struct group *gptr;
  int rc = getgrgid_r(gid, &group, buf, buflen, &gptr);
  if (rc || !gptr)
  {
    free(buf);
    return "UNKNOWN";
  }

  string name(gptr->gr_name);
  free(buf);
  return name;
}

//--------------------------------------------------------------------------
// Get group id from name
gid_t Path::group_name_to_id(const string& gname)
{
  // Even more painful reentrant way of doing this, given we never use
  // the name!
  int buflen = sysconf(_SC_GETGR_R_SIZE_MAX);
  char *buf = (char *)malloc(buflen);
  if (!buf) abort();

  struct group group;
  struct group *gptr;
  int rc = getgrnam_r(gname.c_str(), &group, buf, buflen, &gptr);
  free(buf);  // We don't use it

  if (rc || !gptr) return 0;
  return gptr->gr_gid;
}

//------------------------------------------------------------------------
// << operator to write Path to ostream
ostream& operator<<(ostream& s, const Path& p) 
{ 
  s<<p.str(); return s; 
}

}} // namespaces

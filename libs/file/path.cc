//==========================================================================
// ObTools::File: path.cc
//
// Wrapper around a file path to provide portability and ease of use of
// file manipulation
//
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-file.h"
#include "ot-text.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <utime.h>
#include <sys/time.h>
#include <sstream>
#include <fstream>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#if defined(PLATFORM_WINDOWS)
// Note: stati versions, still 32-bit time_t
#define STRUCT_STAT struct _stati64
#define STAT _wstati64
#define CPATH wide_path().c_str()
#define OPEN _wopen
#define UTIME _wutime
#define CHMOD _wchmod
#define UNLINK _wunlink
#define ACCESS _waccess
#define FUTIME _futime
#define O_LARGEFILE 0
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <ext/stdio_filebuf.h>
#include <shellapi.h>
#else
#if defined(PLATFORM_MACOS)
// No LARGEFILE, otherwise sensible
#define O_LARGEFILE 0
#endif
#define STRUCT_STAT struct stat64
#define STAT stat64
#define CPATH c_str()
#define OPEN open
#define UTIME utime
#define CHMOD chmod
#define UNLINK unlink
#define ACCESS access
#define FUTIME futimes
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <stack>
#endif

#define READ_BUF_SIZE 4096

namespace ObTools { namespace File {

//--------------------------------------------------------------------------
// Constructor from directory and leaf
// If directory is empty or ends with slash already, doesn't add a slash
// If directory is '.', just returns leaf
Path::Path(const string& dir, const string& leaf)
{
  if (dir == ".")
    path = leaf;
  else if (dir.empty() || is_sep_char(dir[dir.size()-1]))
    path = dir + leaf;
  else
    path = dir + SEPCHAR + leaf;
}

//--------------------------------------------------------------------------
// Constructor from existing path and leaf (combines as above)
Path::Path(const Path& _path, const string& leaf):
  Path(_path.str(), leaf) {}

//--------------------------------------------------------------------------
// Find whether it's an absolute path
bool Path::is_absolute() const
{
#if defined(PLATFORM_WINDOWS)
  // Allow for c:\xxx form
  return !path.empty() && (is_sep_char(path[0])
                           ||(path.size()>=3
                              && isalpha(path[0])
                              && path[1]==':'
                              && is_sep_char(path[2])));
#else
  return !path.empty() && is_sep_char(path[0]);
#endif
}

//--------------------------------------------------------------------------
// Get directory based on dirname()
Directory Path::dir() const
{
  return {dirname()};
}

//--------------------------------------------------------------------------
// Get directory: everything before last slash, if any, not including
// trailing slash.  If no slashes, returns empty path
string Path::dirname() const
{
  string::size_type slash = path.rfind(SEPCHAR);
  if (slash == string::npos)
  {
    // No normal separator - check for ALTSEPCHAR (/ in Windows) instead
    if (ALTSEPCHAR) slash = path.rfind(ALTSEPCHAR);
    if (slash == string::npos) return ".";
  }
  else
  {
    // If ALTSEPCHAR exists (/ in Windows) make sure it doesn't happen
    // after the normal one
    string::size_type alt_slash = path.rfind(ALTSEPCHAR);
    if (alt_slash != string::npos && alt_slash > slash) slash = alt_slash;
  }

  // Special case for root
  if (!slash) return "/";
  return string(path, 0, slash);
}

//--------------------------------------------------------------------------
// Get leafname: everything after last slash, if any, otherwise everything
string Path::leafname() const
{
  string::size_type slash = path.rfind(SEPCHAR);
  if (slash == string::npos)
  {
    // No normal separator - check for ALTSEPCHAR (/ in Windows) instead
    if (ALTSEPCHAR) slash = path.rfind(ALTSEPCHAR);
    if (slash == string::npos) return path;
  }
  else
  {
    // If ALTSEPCHAR exists (/ in Windows) make sure it doesn't happen
    // after the normal one
    string::size_type alt_slash = path.rfind(ALTSEPCHAR);
    if (alt_slash != string::npos && alt_slash > slash) slash = alt_slash;
  }

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
// Get the canonicalised absolute pathname
File::Path Path::realpath() const
{
#if defined(PLATFORM_WINDOWS)
  wchar_t buff[_MAX_PATH];
  if (!_wfullpath(buff, CPATH, _MAX_PATH))
    return {};
  return File::Path{Text::UTF8::encode(buff)};
#else
  char buff[PATH_MAX];
  if (!::realpath(CPATH, buff))
    return {};
  return {buff};
#endif
}

//--------------------------------------------------------------------------
// Extend a path
Path& Path::extend(const string& leaf)
{
  path += SEPCHAR + leaf;
  return *this;
}

//--------------------------------------------------------------------------
// Extend a path
Path& Path::extend(const Path& p)
{
  if (p.is_absolute())
    path += p.str();
  else
    path += SEPCHAR + p.str();
  return *this;
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
  while (nn.size() > 3 && nn[0] == '.' && nn[1] == '.' && is_sep_char(nn[2]))
  {
    if (dn == ".") return {nn};  // Stop if we reach .
    nn = string(nn, 3);
    dn = Path(dn).dirname();
  }

  return Path(dn, nn);
}


//--------------------------------------------------------------------------
// Expand special parts of path
// e.g. On windows, expand environment variables at the beginning
Path Path::expand() const
{
#if defined(PLATFORM_WINDOWS)
  if (path.size() > 2 && path[0] == '%')
  {
    auto pos = path.find('%', 1);
    if (pos != string::npos && pos > 1)
    {
      const auto var_name = path.substr(1, pos - 1);
      char buff[32767]; // Max size of environment variable
      const auto result = GetEnvironmentVariable(var_name.c_str(), buff,
                                                 sizeof(buff));
      if (result)
      {
        auto expanded = *this;
        expanded.path.replace(0, pos + 1, buff, result);
        return expanded;
      }
    }
  }
#endif
  return *this;
}

//--------------------------------------------------------------------------
// Fix a path to local directory separator type
// Use to convert '/'-separated paths (e.g. URLs) to local separator
void Path::fix_slashes()
{
#if defined(PLATFORM_WINDOWS)
  for(string::iterator p=path.begin(); p!=path.end(); ++p)
    if (*p == '/') *p = SEPCHAR;
#endif
}

//--------------------------------------------------------------------------
// Does the file exist?
bool Path::exists() const
{
  STRUCT_STAT sb;  // Don't fail on large files
  return !STAT(CPATH, &sb);
}

//--------------------------------------------------------------------------
// Is it a directory?
bool Path::is_dir() const
{
  STRUCT_STAT sb;
  if (STAT(CPATH, &sb)) return false;
  return S_ISDIR(sb.st_mode);
}

//--------------------------------------------------------------------------
// Is the file readable (by me)?
bool Path::readable() const
{
  return !ACCESS(CPATH, 04);
}

//--------------------------------------------------------------------------
// Is the file writable (by me)?
bool Path::writeable() const
{
  if (exists())
    return !ACCESS(CPATH, 02);

  // The file is non-existant so check the directory is writeable instead
  const auto& d = Directory{dirname().empty() ? "." : dirname()};
  return !ACCESS(d.CPATH, 02);
}

//--------------------------------------------------------------------------
// Get the file's length
uint64_t Path::length() const
{
  STRUCT_STAT sb;
  return STAT(CPATH, &sb)?0:sb.st_size;
}

//--------------------------------------------------------------------------
// Get the file's last-modified time (mtime)
time_t Path::last_modified() const
{
  STRUCT_STAT sb;
  return STAT(CPATH, &sb)?0:sb.st_mtime;
}

//--------------------------------------------------------------------------
// Set the file's last-modified time (mtime)
// Also sets access time to now
// Returns whether successful
bool Path::set_last_modified(time_t t) const
{
#if defined(PLATFORM_WINDOWS)
  struct _utimbuf utb;
#else
  struct utimbuf utb;
#endif
  utb.actime = time(NULL);  // Now
  utb.modtime = t;
  return !UTIME(CPATH, &utb);
}

//--------------------------------------------------------------------------
// Get the file's mode
mode_t Path::mode() const
{
  STRUCT_STAT sb;
  return STAT(CPATH, &sb)?0:sb.st_mode;
}

//--------------------------------------------------------------------------
// Set file permissions mode (chmod)
bool Path::set_mode(mode_t mode) const
{
  return !CHMOD(CPATH, mode);
}

//--------------------------------------------------------------------------
// Get the file's owner
uid_t Path::owner() const
{
#if defined(PLATFORM_WINDOWS) // Meaningless in Windows
  return 0;
#else
  STRUCT_STAT sb;
  return STAT(CPATH, &sb)?0:sb.st_uid;
#endif
}

//--------------------------------------------------------------------------
// Get the file's group
gid_t Path::group() const
{
#if defined(PLATFORM_WINDOWS) // Meaningless in Windows
  return 0;
#else
  STRUCT_STAT sb;
  return STAT(CPATH, &sb)?0:sb.st_gid;
#endif
}

//--------------------------------------------------------------------------
// Get the file's owner & group
bool Path::set_ownership(uid_t owner, uid_t group) const
{
#if defined(PLATFORM_WINDOWS) // Meaningless in Windows
  (void)owner;
  (void)group;
  return true;
#else
  return !chown(CPATH, owner, group);
#endif
}

//--------------------------------------------------------------------------
// Get the file's owner & group - string version
bool Path::set_ownership(const string& owner, const string& group) const
{
  int uid = user_name_to_id(owner);
  int gid = group_name_to_id(group);
  if (uid < 0 || gid < 0) return false;
  return set_ownership(static_cast<uid_t>(uid), static_cast<gid_t>(gid));
}

//--------------------------------------------------------------------------
// Delete the file/directory (directories are always deleted recursively)
// Returns whether successful
bool Path::erase() const
{
  if (is_dir())
  {
#if defined(PLATFORM_WINDOWS)
    // Note: Shelling out to rmdir opens a console window

    // Create string with extra null on the end, plus manual null in case
    // c_str() does something odd - we use data() instead
    wstring double_null = wide_path() + (wchar_t)0 + (wchar_t)0;
    SHFILEOPSTRUCTW op =
    {
      0, FO_DELETE, double_null.data(), 0,
      FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT,
      false, 0, L""
    };
    return !SHFileOperationW(&op);

#else
    auto result = true;
    auto dirs = stack<Path>{};
    dirs.push(*this);
    while (result && !dirs.empty())
    {
      const auto& dir = dirs.top();
      auto d = opendir(dir.c_str());
      if (!d)
      {
        result = false;
        break;
      }

      auto subdirs = false;
      // Note: readdir is meant to be thread-safe in all modern implementations
      // at least if you're not passing the dir structure between threads.
      // The re-entrant readdir_r has some issues and is deprecated now
      while (auto de = readdir(d))
      {
        switch (de->d_type)
        {
          case DT_DIR:
            {
              auto d = string{de->d_name};
              if (d != "." && d != "..")
              {
                auto sd = dir;
                sd.extend(d);
                dirs.push(sd);
                subdirs = true;
              }
            }
            break;
          case DT_LNK:
          case DT_REG:
            {
              auto f = dir;
              f.extend(de->d_name);
              unlink(f.c_str());
            }
            break;
          default:
            result = false;
            break;
        }
      }
      closedir(d);

      if (!subdirs)
      {
        rmdir(dir.c_str());
        dirs.pop();
      }
    }
    return result;
#endif
  }
  else
  {
    if (UNLINK(CPATH))
      return !exists();
    else
      return true;
  }
}

//--------------------------------------------------------------------------
// Touch the file, creating it if not already existing, setting mtime if so
// Returns whether successful
bool Path::touch(mode_t mode) const
{
  int fd = OPEN(CPATH, O_CREAT|O_WRONLY, mode);
  if (fd < 0) return false;
  if ( FUTIME(fd,NULL) ) return false;
  close(fd);
  return true;
}

//--------------------------------------------------------------------------
// Rename file to new path
// Note: You probably can't rename between filing systems
bool Path::rename(const Path& new_path) const
{
#if defined(PLATFORM_WINDOWS)
  // Try supposedly atomic MoveFileEx (NT+)
  if (MoveFileExW(CPATH, new_path.wide_path().c_str(),
                  MOVEFILE_REPLACE_EXISTING))
    return true;

  // Fall back to try non-atomic delete/rename (95/98)
  // Note:  Don't worry about wide characters, they wouldn't work in 95/98
  // anyway
  ::unlink(new_path.c_str());  // Doesn't matter if it fails

  // Fall through to usual rename (which uses MoveFile())
#endif

  return !::rename(c_str(), new_path.c_str());
}

//--------------------------------------------------------------------------
// Read the entire file into a string
// Returns whether successful.  If not, the string contains the error
bool Path::read_all(string& s) const
{
  InStream f(path);
  if (!f)
  {
    s = strerror(errno);
    return false;
  }

  char buf[READ_BUF_SIZE];
  while (!f.eof())
  {
    f.read(buf, READ_BUF_SIZE);
    s.append(buf, f.gcount());
  }

  return true;
}

//--------------------------------------------------------------------------
// Replace the entire file with a string
// Returns error string, or "" if successful
string Path::write_all(const string& s)
{
  OutStream f(path);
  if (!f) return strerror(errno);

  f.write(s.data(), s.size());
  return "";
}

//--------------------------------------------------------------------------
// Replace the entire file with a byte vector
// Returns error string, or "" if successful
string Path::write_all(const vector<unsigned char>& d)
{
  OutStream f(path);
  if (!f) return strerror(errno);

  f.write(reinterpret_cast<const char *>(&d[0]), d.size());
  return "";
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
#if defined(PLATFORM_WINDOWS) // Meaningless in Windows
  (void)uid;
  return "?";
#else
  // Painful reentrant way of doing this!
  int buflen = sysconf(_SC_GETPW_R_SIZE_MAX);
  vector<char> buf(buflen);

  struct passwd user;
  struct passwd *uptr;
  int rc = getpwuid_r(uid, &user, &buf[0], buflen, &uptr);
  if (rc || !uptr)
  {
    return "UNKNOWN";
  }

  string name(uptr->pw_name);
  return name;
#endif
}

//--------------------------------------------------------------------------
// Get user id from name
// Returns -1 if it fails
int Path::user_name_to_id(const string& uname)
{
#if defined(PLATFORM_WINDOWS) // Meaningless in Windows
  (void)uname;
  return -1;
#else
  // Even more painful reentrant way of doing this, given we never use
  // the name!
  int buflen = sysconf(_SC_GETPW_R_SIZE_MAX);
  vector<char> buf(buflen);

  struct passwd user;
  struct passwd *uptr;
  int rc = getpwnam_r(uname.c_str(), &user, &buf[0], buflen, &uptr);
  if (rc || !uptr)
  {
    return -1;
  }

  int uid = uptr->pw_uid;
  return uid;
#endif
}

//--------------------------------------------------------------------------
// Get group name from gid
string Path::group_id_to_name(gid_t gid)
{
#if defined(PLATFORM_WINDOWS) // Meaningless in Windows
  (void)gid;
  return "?";
#else
  // Painful reentrant way of doing this!
  int buflen = sysconf(_SC_GETGR_R_SIZE_MAX);
  vector<char> buf(buflen);

  struct group group;
  struct group *gptr;
  int rc = getgrgid_r(gid, &group, &buf[0], buflen, &gptr);
  if (rc || !gptr)
  {
    return "UNKNOWN";
  }

  string name(gptr->gr_name);
  return name;
#endif
}

//--------------------------------------------------------------------------
// Get group id from name
// Returns -1 if it fails
int Path::group_name_to_id(const string& gname)
{
#if defined(PLATFORM_WINDOWS) // Meaningless in Windows
  (void)gname;
  return -1;
#else
  // Even more painful reentrant way of doing this, given we never use
  // the name!
  int buflen = sysconf(_SC_GETGR_R_SIZE_MAX);
  vector<char> buf(buflen);

  struct group group;
  struct group *gptr;
  int rc = getgrnam_r(gname.c_str(), &group, &buf[0], buflen, &gptr);
  if (rc || !gptr)
  {
    return -1;
  }

  int gid = gptr->gr_gid;
  return gid;
#endif
}

#if defined(PLATFORM_WINDOWS)
//--------------------------------------------------------------------------
// Windows only - helper function to convert a UTF8 filename into a
// wide character one
wstring Path::utf8_to_wide(const string& utf8)
{
  const char *ufn = utf8.c_str();
  int ulen = utf8.size();

  // Just get size of output first
  unsigned wlen=MultiByteToWideChar(CP_UTF8,0,ufn,ulen,0,0);
  if (!wlen) return wstring();

  // Then do it properly
  wchar_t *wfn=new wchar_t[wlen];
  wlen=MultiByteToWideChar(CP_UTF8,0,ufn,ulen,wfn,wlen);
  if (!wlen)
  {
    delete[] wfn;
    return wstring();
  }

  wstring wide(wfn, wlen);
  delete[] wfn;
  return wide;
}

//--------------------------------------------------------------------------
// Windows only - helper function to convert a wide character filename
// (e.g. returned from FindNextFileW) to a UTF8 one
string Path::wide_to_utf8(const wstring& wide) const
{
  const wchar_t *wfn = wide.c_str();
  int wlen = wide.size();

  // Just get size of output first
  unsigned ulen=WideCharToMultiByte(CP_UTF8,0,wfn,wlen,0,0,0,0);
  if (!ulen) return string();

  // Then do it properly
  char *ufn=new char[ulen];
  ulen=WideCharToMultiByte(CP_UTF8,0,wfn,wlen,ufn,ulen,0,0);
  if (!ulen)
  {
    delete[] ufn;
    return string();
  }

  string utf8(ufn, ulen);
  delete[] ufn;
  return utf8;
}
#endif

//--------------------------------------------------------------------------
// << operator to write Path to ostream
ostream& operator<<(ostream& s, const Path& p)
{
  s<<p.str(); return s;
}

//--------------------------------------------------------------------------
// Sanitise leaf name
string Path::sanitise_leaf(const string& leaf)
{
  auto sanitised = leaf;
  for (auto& c: sanitised)
  {
    if (!isalnum(c) && c != '.' && c != '_' && c != '-' && c != '+')
      c = '_';
  }
  return sanitised;
}

}} // namespaces

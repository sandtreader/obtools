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
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_FILE_H
#define __OBTOOLS_FILE_H

#include <string>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <list>
#include <fstream>
#include <vector>
#include <ot-gen.h>
#include <stdexcept>
#include <memory>

#if !defined(PLATFORM_WINDOWS)
#include <glob.h>
#endif

// Define uid_t and gid_t in Windows to make build easier
#if defined(PLATFORM_WINDOWS)

// Undefine in case someone defined it as (e.g.) int (e.g. wxWindows!)
#undef uid_t
typedef int uid_t;
#undef gid_t
typedef int gid_t;

#include <sys/stat.h>
#include <fcntl.h>
#include <ext/stdio_filebuf.h>
#endif

// Fix 64-bit file operations in BSD (e.g. OS-X) - it does them natively
// and doesn't provide '64' variants
#if defined(PLATFORM_BSD)
typedef off_t off64_t;
#define lseek64 lseek
#define open64 open
#define O_LARGEFILE 0
#endif

namespace ObTools { namespace File {

// Make our lives easier without polluting anyone else
using namespace std;

class Directory;

//==========================================================================
// Path class
class Path
{
protected:
  string path;

#if defined(PLATFORM_WINDOWS)
  static const char SEPCHAR = '\\';
  static const char ALTSEPCHAR = '/';
#else
  static const char SEPCHAR = '/';
  static const char ALTSEPCHAR = 0;
#endif
  static const char EXTCHAR = '.';

  // Check if a character is a separator, allowing for both \ and / in Win32
  static bool is_sep_char(char c)
  { return c==SEPCHAR || (ALTSEPCHAR && c==ALTSEPCHAR); }

public:
  // Constructors-------------------------------------------------------------
  //------------------------------------------------------------------------
  // Default constructor (empty)
  Path() {}

  //------------------------------------------------------------------------
  // Copy constructor
  Path(const Path& _o): path(_o.path) {}

  //------------------------------------------------------------------------
  // Constructor from string
  Path(const string& _path): path(_path) {}

  //------------------------------------------------------------------------
  // Constructor from directory and leaf
  // If directory is empty or ends with slash already, doesn't add a slash
  // If directory is '.', just returns leaf
  Path(const string& dir, const string& leaf);

  //------------------------------------------------------------------------
  // Constructor from existing path and leaf (combines as above)
  Path(const Path& _path, const string& leaf);

  //------------------------------------------------------------------------
  // Validity check
  bool operator!() const { return path.empty(); }

  // Accessors ---------------------------------------------------------------
  //------------------------------------------------------------------------
  // Get path as string
  const string& str() const { return path; }

  //------------------------------------------------------------------------
  // Get path as C-string (for fopen etc.)
  const char *c_str() const { return path.c_str(); }

  //------------------------------------------------------------------------
  // Extend a path
  Path& extend(const string& leaf);
  Path& extend(const Path& path);

  // Methods to split paths --------------------------------------------------
  //------------------------------------------------------------------------
  // Find whether it's an absolute path
  bool is_absolute() const;

  //------------------------------------------------------------------------
  // Get directory based on dirname()
  Directory dir() const;

  //------------------------------------------------------------------------
  // Get directory: everything before last slash, if any, not including
  // trailing slash.  If no slashes, returns "."
  string dirname() const;

  //------------------------------------------------------------------------
  // Get leafname: everything after last slash, if any, otherwise everything
  string leafname() const;

  //------------------------------------------------------------------------
  // Get extension: Part of leafname following dot, if any
  string extension() const;

  //------------------------------------------------------------------------
  // Get basename: leafname with extension (if any) removed
  string basename() const;

  //------------------------------------------------------------------------
  // Get the canonicalised absolute pathname
  Path realpath() const;

  //------------------------------------------------------------------------
  // Fix a path to local directory separator type
  // Use to convert '/'-separated paths (e.g. URLs) to local separator
  void fix_slashes();

  // Methods to resolve paths ----------------------------------------------
  //------------------------------------------------------------------------
  // Resolve one path against another:
  //   If new path is absolute, return new path
  //   If relative, make absolute path relative to dirname of old path
  Path resolve(const Path& new_path) const;

  //------------------------------------------------------------------------
  // Expand special parts of path
  // e.g. On windows, expand environment variables at the beginning
  Path expand() const;

  // Methods to get file information ---------------------------------------
  //------------------------------------------------------------------------
  // Does the file exist?
  virtual bool exists() const;

  //------------------------------------------------------------------------
  // Is it a directory?
  bool is_dir() const;

  //------------------------------------------------------------------------
  // Is the file readable (by me)?
  bool readable() const;

  //------------------------------------------------------------------------
  // Is the file writable (by me)?
  bool writeable() const;

  //------------------------------------------------------------------------
  // Get the file's length
  uint64_t length() const;

  //------------------------------------------------------------------------
  // Get the file's last-modified time (mtime)
  time_t last_modified() const;

  //------------------------------------------------------------------------
  // Set the file's last-modified time (mtime)
  // Also sets access time to now
  // Returns whether successful
  bool set_last_modified(time_t t) const;

  //------------------------------------------------------------------------
  // Get the file's mode
  mode_t mode() const;

  //------------------------------------------------------------------------
  // Set file permissions mode (chmod)
  bool set_mode(mode_t mode) const;

  // String version
  bool set_mode(const string& mode) const { return set_mode(otoi(mode)); }

  //------------------------------------------------------------------------
  // Get the file's owner
  uid_t owner() const;

  //------------------------------------------------------------------------
  // Get the file's group
  gid_t group() const;

  //------------------------------------------------------------------------
  // Get the file's owner & group
  bool set_ownership(uid_t owner, uid_t group) const;

  // String version
  bool set_ownership(const string& owner, const string& group) const;

  //------------------------------------------------------------------------
  // Delete the file/directory (directories are always deleted recursively)
  // Returns whether successful
  bool erase() const;

  //------------------------------------------------------------------------
  // Touch the file, creating it if not already existing, setting mtime if so
  // Returns whether successful
  bool touch(mode_t mode=0644) const;

  //------------------------------------------------------------------------
  // Rename file to new path
  // Note: You probably can't rename between filing systems
  bool rename(const Path& new_path) const;

  //------------------------------------------------------------------------
  // Read the entire file into a string
  // Returns whether successful.  If not, the string contains the error
  bool read_all(string& s) const;

  //------------------------------------------------------------------------
  // Replace the entire file with a string
  // Returns error string, or "" if successful
  string write_all(const string& s);

  //------------------------------------------------------------------------
  // Replace the entire file with byte vector
  // Returns error string, or "" if successful
  string write_all(const vector<unsigned char>& d);

  //------------------------------------------------------------------------
  // Handy octal conversion functions for file modes
  // Convert integer to octal string
  static string itoo(int mode_i);

  // Convert octal string to integer
  static int otoi(const string& mode_s);

  //------------------------------------------------------------------------
  // Handy user/group id functions for ownership
  // Get user name from uid
  static string user_id_to_name(uid_t uid);

  // Get user id from name
  // Returns -1 if failed
  static int user_name_to_id(const string& uname);

  // Get group name from gid
  static string group_id_to_name(gid_t gid);

  // Get group id from name
  // Returns -1 if failed
  static int group_name_to_id(const string& gname);

#if defined(PLATFORM_WINDOWS)
  //------------------------------------------------------------------------
  // Windows only - helper function to convert a UTF8 filename into a
  // wide character one
  static wstring utf8_to_wide(const string& utf8);

  //------------------------------------------------------------------------
  // Get wide path
  wstring wide_path() const { return utf8_to_wide(path); }

  //------------------------------------------------------------------------
  // Windows only - helper function to convert a wide character filename
  // (e.g. returned from FindNextFileW) to a UTF8 one
  string wide_to_utf8(const wstring& wide) const;
#endif

  //------------------------------------------------------------------------
  // Equality operator
  bool operator==(const Path& b) const
  {
    return path == b.path;
  }

  //------------------------------------------------------------------------
  // Sanitise leaf name
  static string sanitise_leaf(const string& leaf);

  //------------------------------------------------------------------------
  // Virtual destructor
  virtual ~Path() {}
};

//--------------------------------------------------------------------------
// << operator to write Path to ostream
ostream& operator<<(ostream& s, const Path& p);

//==========================================================================
// Directory class
// Adds directory iterator to basic Path
class Directory: public Path
{
public:
  //------------------------------------------------------------------------
  // Constructors, as Path (q.v.)
  Directory(): Path() {}
  Directory(const Path& _o): Path(_o) {}
  Directory(const string& _path): Path(_path) {}
  Directory(const string& dir, const string& leaf): Path(dir, leaf) {}
  Directory(const Path& _path, const string& leaf): Path(_path, leaf) {}

  //------------------------------------------------------------------------
  // Ensure a directory path exists
  // With parents set, acts like 'mkdir -p' and creates full path if required
  // Returns whether successful
  bool ensure(bool parents=false, int mode=0777) const;

  //------------------------------------------------------------------------
  // Get list of directory contents, as leaf strings
  // pattern takes a glob pattern (see Text::pattern_match)
  // If all is set, hidden/dotfiles are returned (including . and ..)
  // Return whether successful (directory readable)
  // Fills in leaves if so
  bool inspect(list<string>& leaves, const string& pattern="*",
               bool all=false) const;

  //------------------------------------------------------------------------
  // Get list of directory contents, as full paths prefixed by directory path
  // Other parameters as above
  // Returns whether successful (directory readable)
  // Fills in paths if so
  bool inspect(list<Path>& paths, const string& pattern="*",
               bool all=false) const;

  //--------------------------------------------------------------------------
  // Get list of directory contents, as full paths prefixed by directory path
  // Recursing to subdirectories
  // Other parameters as above
  // Returns whether successful (directory readable)
  // Fills in paths if so
  bool inspect_recursive(list<Path>& paths, const string& pattern="*",
                         bool all=false) const;

  //------------------------------------------------------------------------
  // Does the directory exist and is it actually dir?
  bool exists() const;

  //------------------------------------------------------------------------
  // Is the directory empty?
  bool empty() const;

  //------------------------------------------------------------------------
  // Extend a path
  Directory& extend(const string& leaf);
  Directory& extend(const Path& path);

  //--------------------------------------------------------------------------
  // Resolve a path against a directory
  //   If new path is absolute, return new path
  //   If relative, make absolute path relative to within the directory
  // Note difference with Path::resolve is that this works from the directory
  // path itself, not its parent
  Path resolve(const Path& new_path) const;

  //------------------------------------------------------------------------
  // Expand special parts of path
  // e.g. On windows, expand environment variables at the beginning
  Directory expand() const { return Path::expand(); }
};

//==========================================================================
// Stream classes - provides unified view of file streams which still works
// with Windows wide-character filenames
#if defined(PLATFORM_WINDOWS)
// Windows version - they are streams created from underlying _wopen FD
class InStream: public istream
{
  int fd;
  __gnu_cxx::stdio_filebuf<char> filebuf;

 public:
  //------------------------------------------------------------------------
  // Constructor
  InStream(const string& fn,
           ios::openmode mode = ios::in | ios::binary):
    fd(_wopen(Path::utf8_to_wide(fn).c_str(),
              O_RDONLY | ((mode & ios::binary)?O_BINARY:0))),
    filebuf(fd, mode)
  { if (fd>=0) istream::init(&filebuf); else setstate(failbit); }

  //------------------------------------------------------------------------
  // Extra close method to make it look like an ifstream
  void close() { ::close(fd); }
};

class OutStream: public ostream
{
  int fd;
  __gnu_cxx::stdio_filebuf<char> filebuf;

 public:
  //------------------------------------------------------------------------
  // Constructor
  OutStream(const string& fn,
           ios::openmode mode = ios::out | ios::trunc | ios::binary):
    fd(_wopen(Path::utf8_to_wide(fn).c_str(),
              O_RDWR | O_CREAT |((mode & ios::binary)?O_BINARY:0)
                               |((mode & ios::trunc)?O_TRUNC:0)
                               |((mode & ios::app)?O_APPEND:0),
              S_IWRITE | S_IREAD)),
    filebuf(fd, mode)
  { if (fd>=0) ostream::init(&filebuf); else setstate(failbit); }

  //------------------------------------------------------------------------
  // Extra close method to make it look like an ifstream
  void close() { ::close(fd); }
};

#else
// Unix/OSX versions - they are normal fstreams
class InStream: public ifstream
{
 public:
  //------------------------------------------------------------------------
  // Constructor
  InStream(const string& fn,
           ios::openmode mode = ios::in | ios::binary):
    ifstream(fn.c_str(), mode) {}
};

class OutStream: public ofstream
{
 public:
  //------------------------------------------------------------------------
  // Constructor
  OutStream(const string& fn,
           ios::openmode mode = ios::out | ios::trunc | ios::binary):
    ofstream(fn.c_str(), mode) {}
};

#endif

//==========================================================================
// Buffered Output Stream
// Has a similar, but reduced, interface to ofstream, and provides simple
// buffering
// Notes:
// - the first write doesn't get buffered due to libstd++ implementation
// - the buffer size cannot be set whilst a file is open
class BufferedOutFileBuf: public filebuf
{
private:
  vector<char> buffer;

protected:
  //------------------------------------------------------------------------
  // Handle characters one at a time
  streamsize xsputn(const char *s, streamsize n);

public:
  //------------------------------------------------------------------------
  // Constructor
  BufferedOutFileBuf(uint64_t size);

  //------------------------------------------------------------------------
  // Resize buffer
  void resize(uint64_t size);

  //------------------------------------------------------------------------
  // Get size of buffer
  uint64_t size() const
  {
    return buffer.size();
  }

  //------------------------------------------------------------------------
  // Virtual destructor
  ~BufferedOutFileBuf()
  {
    // Make sure to close (and therefore write) before freeing buffer
    close();
  }
};

class BufferedOutStream: public ostream
{
private:
  BufferedOutFileBuf file_buf;

public:
  //------------------------------------------------------------------------
  // Constructors
  BufferedOutStream();
  BufferedOutStream(const string& filename,
                    uint64_t buffer_size,
                    ios_base::openmode mode = ios_base::out
                                            | ios_base::trunc
                                            | ios_base::binary);

  //------------------------------------------------------------------------
  // Set buffer size
  // Note: will not work if called whilst file open
  void set_buffer_size(uint64_t buffer_size)
  {
    file_buf.resize(buffer_size);
  }

  //------------------------------------------------------------------------
  // Get buffer size
  uint64_t get_buffer_size() const
  {
    return file_buf.size();
  }

  //------------------------------------------------------------------------
  // Test for file being open
  bool is_open() const
  {
    return file_buf.is_open();
  }

  //------------------------------------------------------------------------
  // Open a file
  void open(const char *filename, ios_base::openmode mode = ios_base::in
                                                          | ios_base::out)
  {
    if (!file_buf.open(filename, mode))
      setstate(ios_base::failbit);
    else
      clear();
  }

  //------------------------------------------------------------------------
  // Close file
  void close()
  {
    if (!file_buf.close())
      setstate(ios_base::failbit);
  }
};

//==========================================================================
// Multi Output File Stream
class MultiOutFileBuf: public streambuf
{
private:
  vector<unique_ptr<filebuf> >& file_bufs;

protected:
  //------------------------------------------------------------------------
  // Put sequence of characters
  streamsize xsputn(const char *s, streamsize n);

  //------------------------------------------------------------------------
  // Put character on overflow
  int overflow(int c = EOF);

  //------------------------------------------------------------------------
  // Set internal position pointer to relative position
  // Implemented for sake of MultiOutStream::tellp() functionality
  streampos seekoff(streamoff off, ios_base::seekdir way,
                    ios_base::openmode which = ios_base::in | ios_base::out);

public:
  //------------------------------------------------------------------------
  // Constructor
  MultiOutFileBuf(vector<unique_ptr<filebuf> >& _file_bufs):
    file_bufs(_file_bufs)
  {}
};

class MultiOutStream: public ostream
{
private:
  vector<unique_ptr<filebuf> > file_bufs;
  MultiOutFileBuf file_buf;

public:
  //------------------------------------------------------------------------
  // Constructors
  MultiOutStream();

  //------------------------------------------------------------------------
  // Test for file being open
  bool is_open() const;

  //------------------------------------------------------------------------
  // Open a file
  void open(const char *filename, ios_base::openmode mode = ios_base::in
                                                           | ios_base::out);

  //------------------------------------------------------------------------
  // Open a file with return of success or not
  bool open_back(const char *filename, ios_base::openmode mode = ios_base::in
                                                           | ios_base::out);

  //------------------------------------------------------------------------
  // Close file
  void close();
};

//==========================================================================
// Buffered Multi Output File Stream
class BufferedMultiOutFileBuf: public streambuf
{
private:
  vector<unique_ptr<BufferedOutFileBuf>>& file_bufs;

protected:
  //------------------------------------------------------------------------
  // Put sequence of characters
  streamsize xsputn(const char *s, streamsize n);

  //------------------------------------------------------------------------
  // Put character on overflow
  int overflow(int c = EOF);

  //------------------------------------------------------------------------
  // Set internal position pointer to relative position
  // Implemented for sake of MultiOutStream::tellp() functionality
  streampos seekoff(streamoff off, ios_base::seekdir way,
                    ios_base::openmode which = ios_base::in | ios_base::out);

public:
  //------------------------------------------------------------------------
  // Constructor
  BufferedMultiOutFileBuf(vector<unique_ptr<BufferedOutFileBuf>>& _file_bufs):
    file_bufs{_file_bufs}
  {}
};

class BufferedMultiOutStream: public ostream
{
private:
  vector<unique_ptr<BufferedOutFileBuf>> file_bufs;
  BufferedMultiOutFileBuf file_buf;
  uint64_t buffer_size = 1 << 10; // "Measurement would reveal the best choice"

public:
  //------------------------------------------------------------------------
  // Constructors
  BufferedMultiOutStream(uint64_t _buffer_size);

  //------------------------------------------------------------------------
  // Set buffer size
  // Note: will not work if called whilst file open
  void set_buffer_size(uint64_t buffer_size);

  //------------------------------------------------------------------------
  // Get buffer size
  uint64_t get_buffer_size() const
  {
    return buffer_size;
  }

  //------------------------------------------------------------------------
  // Test for file being open
  bool is_open() const;

  //------------------------------------------------------------------------
  // Open a file
  void open(const char *filename, ios_base::openmode mode = ios_base::in
                                                           | ios_base::out);

  //------------------------------------------------------------------------
  // Open a file with return of success or not
  bool open_back(const char *filename, ios_base::openmode mode = ios_base::in
                                                           | ios_base::out);

  //------------------------------------------------------------------------
  // Close file
  void close();
};

//==========================================================================
// Glob class
// Perform actions on multiple files specified by a glob

#if !defined(PLATFORM_WINDOWS)
class Glob
{
private:
  glob_t result;

public:
  //------------------------------------------------------------------------
  // Error exception
  class Error: public runtime_error
  {
  public:
    Error(const string& error):
      runtime_error(error)
    {}
  };

  typedef char ** const_iterator;

  //------------------------------------------------------------------------
  // Constructor
  Glob(const string& pattern);

  //------------------------------------------------------------------------
  // Erase files / directories
  bool erase() const;

  //------------------------------------------------------------------------
  // Beginning iterator
  const_iterator begin() const;

  //------------------------------------------------------------------------
  // Ending iterator
  const_iterator end() const;

  //------------------------------------------------------------------------
  // Destructor
  ~Glob();
};
#endif

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_FILE_H

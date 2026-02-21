# CLAUDE.md - ObTools::ReGen Library

## Overview

`ObTools::ReGen` provides code regeneration support — merging auto-generated code with user-edited sections using marked blocks. Lives under `namespace ObTools::ReGen`.

**Header:** `ot-regen.h`
**Dependencies:** none

## Key Classes

| Class | Purpose |
|-------|---------|
| `MarkedFile` | Reads files with marker-delimited blocks |
| `MasterFile` | Master (generated) file with merge support |
| `Block` | Collection of lines from a marked block |
| `BlockLine` | Single line with type info |
| `rofstream` | Regenerating output file stream |
| `regenbuf` | Custom streambuf for regeneration |

## Line Types

```cpp
enum LineType {
  LINE_NORMAL, LINE_OPEN, LINE_CLOSE,
  LINE_USER_START, LINE_USER_END
};
```

## Merge Flags

```cpp
enum {
  MERGE_DELETE_ORPHANS = 1,  // remove blocks no longer in master
  MERGE_SUPPRESS_NEW  = 2   // suppress new blocks from master
};
```

## MarkedFile

```cpp
MarkedFile(istream& in, const char *mark = "//~");
bool read_line();
string& line_text();
LineType line_type();
string line_tag();
```

## MasterFile

```cpp
MasterFile(istream& in, const char *mark = "//~");
void dump(ostream& sout);
void merge(MarkedFile& ufile, ostream& sout, int flags = 0);
```

## rofstream (Regenerating Output Stream)

```cpp
rofstream(string fn, const char *marker = "//~", int flags = 0);
void close();
~rofstream();  // auto-closes
```

Writes to `rofstream` as if to `ofstream`, but on close merges with existing file to preserve user-edited sections.

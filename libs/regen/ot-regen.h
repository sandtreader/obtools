//==========================================================================
// ObTools::ReGen: ot-regen.h
//
// Interface for ObTools Regenerator library
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#ifndef __OBTOOLS_REGEN_H
#define __OBTOOLS_REGEN_H

#include <string>
#include <list>
#include <map>
#include <iostream>
using namespace std;

namespace ObTools { namespace ReGen
{

//==========================================================================
//
enum LineType
{
  LINE_NORMAL,              // Ordinary unmarked line
  LINE_OPEN,                // Block open, with tag
  LINE_CLOSE,               // Block close
  LINE_USER_START,          // User text starts
  LINE_USER_END             // User text ends
};

//==========================================================================
//Basic file of lines with markers
class MarkedFile
{
protected:
  istream& sin;             // Stream we're reading from
  const char *marker;       // Marker string
  string line;              // Last line read, not including EOL
  
public:
  //------------------------------------------------------------------------
  // Constructor
  MarkedFile(istream &in, const char *mark="//~"):sin(in),marker(mark) {}

  //------------------------------------------------------------------------
  // Read a line - returns whether successful
  bool read_line();

  //------------------------------------------------------------------------
  // Get text of last read line - not including EOL
  string& line_text() { return line; }

  //------------------------------------------------------------------------
  // Find type of last read line
  LineType line_type();

  //------------------------------------------------------------------------
  // Get tag of last read line
  string line_tag();
};

//==========================================================================
//Block of text from master file 
struct BlockLine
{
  LineType type;
  string text;

  //------------------------------------------------------------------------
  // Constructor
  BlockLine(LineType lt, const string& t): type(lt),text(t) {}
};

struct Block
{
  bool used;
  list<BlockLine *> lines;

  //------------------------------------------------------------------------
  // Constructor/Destructor
  Block(): used(false) {}
  ~Block();

  //------------------------------------------------------------------------
  // Add a line
  void add_line(LineType lt, const string& t);

  //------------------------------------------------------------------------
  // Dump to output stream
  void dump(ostream& sout);
};

//==========================================================================
//Master file - reads map of blocks and merges user files
enum 
{
  MERGE_DELETE_ORPHANS = 1,// Remove blocks from user code no longer in master
  MERGE_SUPPRESS_NEW = 2   // Suppress new blocks from master
};

class MasterFile: private MarkedFile
{
private:
  list<Block *> blocks;            // List of blocks (for GC)
  map<string, Block *> blockmap;   // Map of blocks by tag

  Block *find_block(const string& tag);

public:
  //------------------------------------------------------------------------
  // Constructor/Destructor
  MasterFile(istream &in, const char *mark="//~");
  ~MasterFile();

  //------------------------------------------------------------------------
  // Dump blocks to given stream
  void dump(ostream& sout);

  //------------------------------------------------------------------------
  // Merge with given user file, to given stream
  void merge(MarkedFile &ufile, ostream& sout, int flags=0);
};

//==========================================================================
//Regenerating output file stream - replacement for ofstream which 
//'magically' regenerates over existing files

//regenerating streambuf
class regenbuf: public streambuf
{
private:
  string fn;
  string buffer;
  bool closed;
  const char *marker;
  int flags;

protected:
  // Streambuf overflow - handles characters
  int overflow(int); 
   
public:
  // Constructor/destructor
  regenbuf(string _fn, const char *_marker="//~", int flags=0);
  ~regenbuf();

  // Close stream - does the actual write to the file
  void close();
};

//Regenerating output file stream 
class rofstream: public ostream
{
public:
  // Constructor - like ofstream, also take MasterFile marker and flags
  rofstream(string _fn, const char *_marker="//~", int _flags=0): 
    ostream(new regenbuf(_fn, _marker, _flags)) {}

  // Destructor
  ~rofstream() { close(); delete rdbuf(); }

  // Close - pass on to regenbuf to do
  void close() { static_cast<regenbuf *>(rdbuf())->close(); }
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_REGEN_H




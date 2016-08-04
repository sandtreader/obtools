//==========================================================================
// ObTools::ReGen: mfile.cc
//
// Marked file - reads input file with marker lines
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-regen.h"
using namespace ObTools::ReGen;

#define MARK_OPEN   '['
#define MARK_CLOSE  ']'
#define MARK_USTART '^'
#define MARK_UEND   'v'

//--------------------------------------------------------------------------
// Read a line - returns whether successful
bool MarkedFile::read_line()
{
  if (!sin) return false;
  getline(sin, line);
  // Double check, to prevent bogus empty last line
  if (!sin && !line.size()) return false;
  return true;
}

//--------------------------------------------------------------------------
// Find type of last read line
LineType MarkedFile::line_type()
{
  string::size_type pos;

  // Locate and skip marker
  pos = line.find(marker);
  if (pos == string::npos) return LINE_NORMAL;
  pos += 3;

  // Ignore if no following character
  if (pos >= line.size()) return LINE_NORMAL;

  char c = line[pos];
  switch (c)
  {
    case MARK_OPEN:   return LINE_OPEN;
    case MARK_CLOSE:  return LINE_CLOSE;
    case MARK_USTART: return LINE_USER_START;
    case MARK_UEND:   return LINE_USER_END;
    default:          return LINE_NORMAL;  // Ignore unrecognised
  }
}

//--------------------------------------------------------------------------
// Get tag of last read line
// Tag is remainder of line after marker and open char, with leading and
// trailing WS removed
string MarkedFile::line_tag()
{
  string::size_type pos, end;

  // Locate and skip marker and opening type char (assumed)
  pos = line.find(marker);
  if (pos == string::npos) return "";
  pos+=4;

  // Ignore if no following text
  if (pos >= line.size()) return "";

  // Walk forward over whitespace
  pos = line.find_first_not_of(" \t", pos);
  if (pos == string::npos) return "";

  // Find end, skipping trailing whitespace
  end = line.find_last_not_of(" \t");
  if (end == string::npos || end<=pos) return "";

  // Extract this
  return line.substr(pos, end-pos+1);
}



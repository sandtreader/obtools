//==========================================================================
// ObTools::Web: mime-headers.cc
//
// Parser/Generator for MIME headers
//
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-web.h"
#include "ot-text.h"

namespace ObTools { namespace Web {

//==========================================================================
// MIMEHeaderParser

//--------------------------------------------------------------------------
// Get a line
// Returns true if read OK - even if blank
bool MIMEHeaderParser::getline(string& s)
{
  unsigned int count = 0;

  while (!in.fail())
  {
    int c = in.get();

    switch (c)
    {
      case EOF:  // Error - should end cleanly with blank line
	return false;

      case '\r':
	// Skip remaining LF if present
	c = in.get();
	if (c!=EOF && c!='\n') in.putback(c);
	return true;

      case '\n': // Shouldn't happen, but being liberal for Unix files
	return true;

      default:
	if (++count < MAX_HEADER)  // DOS protection
	  s+=c;
	else
	  return false;            // Bomb out 
    }
  }

  return false;
}

//--------------------------------------------------------------------------
// Parse a stream
// Returns whether successful
// Skips the blank line delimiter, leaving stream ready to read message
// body (if any)
bool MIMEHeaderParser::parse()
{
  // Read lines
  while (!in.fail())
  {
    string line;
    if (!getline(line)) return false;
    
    if (line.empty()) 
      return true;

    // Check for :
    string::size_type colon = line.find(':');

    if (colon != string::npos)
    {
      string name(line, 0, colon);
      string value(line, colon+1);

      // Check for following LWS - indicates continuation header
      for(;;)
      {
	int c = in.get();
	if (c==' ' || c=='\t')
	{
	  string extra;
	  if (!getline(extra)) return false;

	  // DoS protection - just in case someone is 'clever' enough
	  // to send folded headers, each of which is within the length,
	  // we need to check the total length
	  if (value.size() + extra.size() > MAX_HEADER) return false;

	  value += ' '; 
	  value += extra;
	}
	else 
	{
	  in.putback(c);
	  break;
	}
      }

      // Lower-case name for comparison
      name = Text::tolower(name);

      // Strip leading and trailing whitespace and flatten internal space
      value = Text::canonicalise_space(value);

      // If anything sensible left, add it to the XML structure
      if (!name.empty() && !value.empty())
	root.add(name, value);
    }
  }

  return false;
}

//==========================================================================
// MIMEHeaderGenerator

//--------------------------------------------------------------------------
// Generates the stream
// Returns whether successful (can only fail if stream fails)
// Includes the blank line delimiter, leaving stream ready to write message
// body (if any)
bool MIMEHeaderGenerator::generate()
{
  // Iterate over each child of root
  OBTOOLS_XML_FOREACH_CHILD(e, root)
    string name = e.name;
    string value = e.content;

    // Uppercase first letter to be tidy
    name[0] = toupper(name[0]);

    // Check stream is OK
    if (out.fail()) return false;

    // Output header
    out << name << ": ";

    // Split value for long lines
    while (value.size() > MAX_LINE)
    {
      // Look for a likely breakpoint - try commas first
      string::size_type split = value.rfind(',', MAX_LINE);
      
      // Failing that, try space
      if (split == string::npos) split = value.rfind(' ', MAX_LINE);

      if (split != string::npos)
      {
	// Split it here
	string frag(value, 0, split);
	value.erase(0, split+1);

	// Output first fragment, with continuation
	out << frag << "\r\n ";
      }
      else break; // Give up - they'll have to have it long
    }

    // Output remainder and CRLF
    out << value << "\r\n";
  OBTOOLS_XML_ENDFOR

  // Output final blank line
  if (out.fail()) return false;
  out << "\r\n";

  return true;
}


}} // namespaces




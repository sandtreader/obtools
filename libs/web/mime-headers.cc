//==========================================================================
// ObTools::Web: mime-headers.cc
//
// Parser/Generator for MIME headers
//
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-web.h"
#include "ot-text.h"
#include <time.h>

namespace ObTools { namespace Web {

//------------------------------------------------------------------------
// Add a current date header to RFC 822 standard
void MIMEHeaders::put_date(const string& header)
{
  // Add date
  time_t now = time(NULL);
  char buf[64];
  strftime(buf, 64, "%a, %b %d %Y %H:%M:%S GMT", gmtime(&now));
  put(header, buf);
}

//--------------------------------------------------------------------------
// Get a line
// Returns true if read OK - even if blank
bool MIMEHeaders::getline(istream& in, string& s)
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
// Get all headers of name 'name'
list<string> MIMEHeaders::get_all(const string& name) const
{
  list<string> l;
  for(ObTools::XML::ConstElementIterator p(xml.get_children(name)); p; ++p)
  {
    const ObTools::XML::Element& e=*p;
    string value = e.content;
    if (!value.empty()) l.push_back(value);
  }

  return l;
}

//--------------------------------------------------------------------------
// Split multi-value headers at commas
// Reads all headers of name 'name', and splits at delimiter to give a
// flattened list of values
list<string> MIMEHeaders::get_all_splitting(const string& name,
                                            char delimiter) const
{
  list<string> l;
  for(ObTools::XML::ConstElementIterator p(xml.get_children(name)); p; ++p)
  {
    const ObTools::XML::Element& e=*p;
    string value = e.content;

    // Loop over all values in each header
    for(;;)
    {
      // Check for delimiter, and split if so
      string::size_type delim = value.find(delimiter);

      if (delim == string::npos) break;

      // Split here, store it and chop for next round
      string sub(value, 0, delim);
      sub = Text::canonicalise_space(sub);
      if (!sub.empty()) l.push_back(sub);
      value.erase(0, delim+1);
    }

    // Add (remaining) value to the list (too)
    value = Text::canonicalise_space(value);
    if (!value.empty()) l.push_back(value);
  }

  return l;
}

//--------------------------------------------------------------------------
// Split a header value (e.g. from get or get_all) into a prime value
// and parameters delineated by ';'
// Parameters without a value are given the value '1'
// The value is modified to be without parameters

// Useful for Content-Type (HTTP), Transport (RTSP) etc.
// e.g.
//   Content-Type: text/html; charset=ISO-8859-1; pure
//
// Leaves 'text/html' in value, and property list:
//   charset     ISO-8859-1
//   pure        1
Misc::PropertyList MIMEHeaders::split_parameters(string& value)
{
  Misc::PropertyList pl;
  string::size_type p = 0;
  string remaining;

  // Loop over parameters
  for(;;)
  {
    string element;

    // Look for ';'
    string::size_type semi = value.find(';', p);
    if (semi == string::npos)
      element = string(value, p);
    else
      element = string(value, p, semi-p);

    element = Text::canonicalise_space(element);

    if (!element.empty())
    {
      // Treat first specially - capture into remaining
      if (!p)
        remaining = element;
      else
      {
        // Split at = if present
        string::size_type eq = element.find('=');
        if (eq == string::npos)  // No equal - set to 1
          pl.add(element, "1");
        else
        {
          // Split
          string pn(element, 0, eq);
          string pv(element, eq+1);
          pn = Text::canonicalise_space(pn);
          pv = Text::canonicalise_space(pv);
          if (!pn.empty()) pl.add(pn, pv);
        }
      }
    }

    // Move to next
    if (semi != string::npos)
      p = semi+1;
    else
      break;
  }

  value = remaining;
  return pl;
}

//--------------------------------------------------------------------------
// Parse headers from a stream
// Returns whether successful
// Skips the blank line delimiter, leaving stream ready to read message
// body (if any)
bool MIMEHeaders::read(istream& in)
{
  // Clear existing
  xml.clear_children();

  // Read lines
  while (!in.fail())
  {
    string line;
    if (!getline(in, line)) return false;

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
          if (!getline(in, extra)) return false;

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
        xml.add(name, value);
    }
  }

  return false;
}

//--------------------------------------------------------------------------
// Generates headers to a stream
// Returns whether successful (can only fail if stream fails)
// Includes the blank line delimiter, leaving stream ready to write message
// body (if any)
bool MIMEHeaders::write(ostream& out) const
{
  // Iterate over each child of root xml
  OBTOOLS_XML_FOREACH_CHILD(e, xml)
    string name = e.name;
    string value = e.content;

    // Uppercase first letter and any letters after - to be conformant
    bool first = true;
    for(string::size_type i=0; i<name.size(); i++)
    {
      if (first) name[i] = toupper(name[i]);
      first = (name[i] == '-');
    }

    // Check stream is OK
    if (out.fail()) return false;

    // Output header
    out << name << ": ";

    // Split value for long lines
    while (value.size() > MAX_LINE)
    {
      // Look for a likely breakpoint - try commas first
      string::size_type split = value.rfind(',', MAX_LINE);

      if (split != string::npos)
      {
        // Move over it, leaving it on first line
        split++;

        // This next character must be a space or tab, otherwise we can't split
        // here safely
        if (value[split] != ' ' && value[split] != '\t')
          split = string::npos;
      }

      if (split == string::npos)
      {
        // Failing that, try space
        split = value.rfind(' ', MAX_LINE);
      }

      if (split != string::npos)
      {
        // Split it here
        string frag(value, 0, split);

        // Erase including the split space
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

//------------------------------------------------------------------------
// >> operator to read MIMEHeaders from istream
// e.g. cin >> url;
istream& operator>>(istream& s, MIMEHeaders& mh)
{
  mh.read(s);  // !!! Check for failure?
  return s;
}

//------------------------------------------------------------------------
// << operator to write MIMEHeaders to ostream
// e.g. cout << url;
ostream& operator<<(ostream& s, const MIMEHeaders& mh)
{
  mh.write(s);
  return s;
}


}} // namespaces




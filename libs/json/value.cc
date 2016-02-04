//==========================================================================
// ObTools::JSON: value.cc
//
// JSON value operations, including writing
//
// Copyright (c) 2016 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-json.h"
#include <iomanip>

namespace ObTools { namespace JSON {

//------------------------------------------------------------------------
// Escape the string and write to the output
void Value::write_string_to(ostream& out) const
{
  out << '"';

  vector<wchar_t> unicode;
  Text::UTF8::decode(s, unicode);

  for(vector<wchar_t>::iterator p=unicode.begin(); p!=unicode.end(); ++p)
  {
    wchar_t c = *p;
    switch (c)
    {
      case '\\': out << "\\\\"; break;
      case '"':  out << "\\\""; break;
      case '\b': out << "\\b"; break;
      case '\f': out << "\\f"; break;
      case '\n': out << "\\n"; break;
      case '\r': out << "\\r"; break;
      case '\t': out << "\\t"; break;
      default:
        if (c > 0x7f)
        {
          if (c < 0x10000)
            out << "\\u" << setw(4) << setfill('0') << hex << c;
          else
            out << "\\u++++";
        }
        else
          out << static_cast<char>(c);
    }
  }
  out << '"';
}

//------------------------------------------------------------------------
// Write an object value to the output
void Value::write_object_to(ostream& out, bool pretty, int indent) const
{
  // Whether to pretty print on multiple lines - optimise for {}
  bool multiline = pretty && !o.empty();

  out << '{';
  if (multiline) out << '\n';

  for(map<string, Value>::const_iterator p = o.begin(); p!=o.end();)
  {
    if (pretty) for(int i=0; i<indent+2; i++) out << ' ';
    out << "\"" << p->first << "\":";

    const Value& v = p->second;
    if (pretty)
    {
      // 'ANSI' bracing style if sub-object is OBJECT or ARRAY and non-empty
      if ((v.type == Value::OBJECT && !v.o.empty())
       || (v.type == Value::ARRAY && !v.a.empty()))
      {
        out << '\n';
        for(int i=0; i<indent+2; i++) out << ' ';
      }
      else out << ' ';  // on same line
    }
    v.write_to(out, pretty, indent+2);
    if (++p != o.end()) out << ',';
    if (pretty) out << '\n';
  }

  if (multiline) for(int i=0; i<indent; i++) out << ' ';
  out << "}";
  if (multiline && !indent) out << '\n';  // Tidy last line
}

//------------------------------------------------------------------------
// Write an array value to the output
void Value::write_array_to(ostream& out, bool pretty, int indent) const
{
  // Only go multi-line if we contain OBJECTs or ARRAYs which are non-empty
  // and hence require multi-line themselves
  bool multiline = false;
  if (pretty)
  {
    for(vector<Value>::const_iterator p = a.begin(); p!=a.end(); ++p)
    {
      const Value& v = *p;
      if ((v.type == Value::OBJECT && !v.o.empty())
          ||(v.type == Value::ARRAY && !v.a.empty()))
      {
        multiline = true;
        break;
      }
    }
  }

  out << "[";
  if (multiline) out << '\n';

  for(vector<Value>::const_iterator p = a.begin(); p!=a.end();)
  {
    if (multiline) for(int i=0; i<indent+2; i++) out << ' ';
    else if (pretty) out << ' ';

    const Value& v = *p;
    v.write_to(out, pretty, indent+2);
    if (++p != a.end())
    {
      out << ',';
      if (multiline) out << '\n';
    }
    else if (multiline)
      out << '\n';
    else if (pretty)
      out << ' ';
  }

  if (multiline) for(int i=0; i<indent; i++) out << ' ';
  out << "]";
  if (multiline && !indent) out << '\n';  // Tidy last line
}

//------------------------------------------------------------------------
// Write the value to the given stream
// Set 'pretty' for multi-line, indented pretty-print, clear for optimal
void Value::write_to(ostream& out, bool pretty, int indent) const
{
  switch (type)
  {
    case NULL_:  out << "null";        break;
    case NUMBER: out << n;             break;
    case STRING: write_string_to(out); break;
    case OBJECT: write_object_to(out, pretty, indent); break;
    case ARRAY:  write_array_to(out, pretty, indent);  break;
    case TRUE:   out << "true";        break;
    case FALSE:  out << "false";       break;
  }
}

//------------------------------------------------------------------------
// >> operator to write to ostream
ostream& operator<<(ostream& s, const Value& v)
{
  v.write_to(s, true);  // Pretty print, 0 indent
  return s;
}

}} // namespaces

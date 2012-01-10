//==========================================================================
// ObTools::AMF: value.cc
//
// AMF value structure
//
// Copyright (c) 2012 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-amf.h"
#include "ot-misc.h"
#include "ot-time.h"
#include "ot-text.h"

namespace ObTools { namespace AMF {

//------------------------------------------------------------------------
// Pretty print the message
ostream& Value::log(ostream& sout, const string& indent) const
{
  switch (type)
  {
    case UNDEFINED: sout << "undefined"; break;
    case NULLV:     sout << "null"; break;
    case FALSE:     sout << "false"; break;
    case TRUE:      sout << "true"; break;

    case INTEGER:   sout << "integer: " << n; break;
    case DOUBLE:    sout << "double: " << d; break;
    case STRING:    sout << "string: '" << text << "'"; break;
    case XML_DOC:   sout << "xml-doc:\n" << text; break;
    case DATE:
    {
      sout << "double: " << d << " (";
      Time::Stamp t("19700101T000000Z");
      sout << (t+Time::Duration(d/1000)).iso() << ")";
      break;
    }

    case ARRAY:
    {
      sout << "array";
      if (dense_array.size())
      {
        sout << " dense(" << dense_array.size() << "):\n";
        for(unsigned int i=0; i<dense_array.size(); i++)
        {
          sout << indent+"  ";
          dense_array[i].log(sout, indent+"  ");
          sout << endl;
        }
      }

      if (assoc_array.size())
      {
        sout << " associative:\n";
        for(map<string, Value>::const_iterator p = assoc_array.begin();
            p != assoc_array.end(); ++p)
        {
          sout << indent+"  " << p->first << " = ";
          p->second.log(sout, indent+"  ");
          sout << endl;
        }
      }

      break;
    }

    case OBJECT:
    {
      sout << "object:\n";
      for(map<string, Value>::const_iterator p = assoc_array.begin();
          p != assoc_array.end(); ++p)
      {
        sout << indent+"  " << p->first << " = ";
        p->second.log(sout, indent+"  ");
        sout << endl;
      }

      break;
    }

    case XML:   sout << "xml:\n" << text; break;

    case BYTE_ARRAY:
    {
      sout << "byte-array: " << text.size() << " bytes\n";
      Misc::Dumper dumper(sout);
      dumper.dump(text.data(), text.size());
    }
  }

  return sout;
}

//------------------------------------------------------------------------
// Read from a channel in the given format
void Value::read(Channel::Reader& chan, Format format)
  throw (Channel::Error)
{
  type = (Type)chan.read_byte();
  Type original_type = type;

  // If AMF0, map to AMF3 types
  if (format == FORMAT_AMF0)
  {
    switch (type)
    {
      case 0: type = DOUBLE; break;   // Number
      case 1: type = chan.read_byte()?TRUE:FALSE; break;  // Boolean - read it
      case 2: type = STRING; break;
      case 3: type = OBJECT; break;
      case 5: type = NULLV; break;
      case 6: type = UNDEFINED; break;
        // case 7: ??
      case 8: type = ARRAY; break;    // ECMA (associative) array
        // case 9: ??
      case 10: type = ARRAY; break;   // Strict (dense) array
      case 11: type = DATE; break;
      case 12: type = STRING; break;  // Long string

      case 15: type = XML_DOC; break;
      default: throw Channel::Error(99,"Unknown AMF0 marker "+Text::itos(type));
    }
  }

  // Now read based on (mapped) type
  switch (type)
  {
    case UNDEFINED:
    case NULLV:
    case FALSE:
    case TRUE:
      // Nothing more to do
      break;

    case STRING:
    case XML_DOC:
    case XML:
    case BYTE_ARRAY:
    {
      int len = chan.read_nbo_16();
      // !!! Check for AMF0 long string, AMF3 reference
      chan.read(text, len);
      break;
    }

    case INTEGER:
    {
      // !!! Check for AMF3 29-bit horror
      n = chan.read_nbo_32();
      break;
    }

    case DOUBLE:
    case DATE:
    {
      d = chan.read_nbo_double();
      break;
    }

    case ARRAY:
    {
      // !!! AMF3 version
      uint32_t count = chan.read_nbo_32();
      while (count--)
      {
        int len = chan.read_nbo_16();
        string name;
        // !!! Check for AMF3 reference
        chan.read(name, len);
        Value v;
        v.read(chan, format);
        set(name, v);
      }
      break;
    }

    case OBJECT:
    {
      // !!! AMF3 version
      for(;;)
      {
        int len = chan.read_nbo_16();
        if (!len) break;

        string name;
        // !!! Check for AMF3 reference
        chan.read(name, len);
        Value v;
        v.read(chan, format);
        set(name, v);
      }
      break;
    }

    default: throw Channel::Error(99, "Unknown AMF marker "+Text::itos(type));
  }
}

//------------------------------------------------------------------------
// << operator to write a value to ostream
ostream& operator<<(ostream& s, const Value& v)
{
  return v.log(s);
}

}} // namespaces

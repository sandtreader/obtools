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
// Comparator
bool Value::operator==(const Value& o) const
{
  if (type != o.type) return false;

  switch (type)
  {
    case INTEGER:
      if (n != o.n) return false;
      break;

    case DOUBLE:
    case DATE:
      if (d != o.d) return false;
      break;

    case STRING:
    case XML_DOC:
    case XML:
    case BYTE_ARRAY:
      if (text != o.text) return false;
      break;

    case ARRAY:
    case OBJECT:
      if (assoc_array != o.assoc_array) return false;
      if (dense_array != o.dense_array) return false;
      break;

    default:;  // Nothing to compare
  }

  return true;
}

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
  throw (Exception, Channel::Error)
{
  type = (Type)chan.read_byte();
  Type original_type = type;

  // If AMF0, map to AMF3 types
  if (format == FORMAT_AMF0)
  {
    switch (type)
    {
      case 0x00: type = DOUBLE; break;   // Number
      case 0x01: type = chan.read_byte()?TRUE:FALSE; break; // Bool - read it
      case 0x02: type = STRING; break;
      case 0x03: type = OBJECT; break;
        // 0x04 - movieclip, reserved
      case 0x05: type = NULLV; break;
      case 0x06: type = UNDEFINED; break;
        // 0x07: Reference??
      case 0x08: type = ARRAY; break;    // ECMA (associative) array
        // 0x09: Object end should never be seen seperately
      case 0x0A: type = ARRAY; break;   // Strict (dense) array
      case 0x0B: type = DATE; break;
      case 0x0C: type = STRING; break;  // Long string
        // 0x0D - unsupported?
        // 0x0E - recordset, not supported
      case 0x0F: type = XML_DOC; break;
      default: throw Exception("Unknown AMF0 marker "+Text::itos(type));
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
      // 16 or 32 bit length
      int len = (format == FORMAT_AMF0
                 && (original_type == 12 || original_type == 15))?
        chan.read_nbo_32():chan.read_nbo_16();
      // !!! Check for AMF3 reference
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
      uint32_t count = chan.read_nbo_32();

      if (format == FORMAT_AMF0 && original_type == 10) // Strict array
      {
        // Assume strict just uses count
        while (count--)
        {
          Value v;
          v.read(chan, format);
          add(v);
        }

        break;
      }

      // ECMA array - ignore count and fall through to OBJECT
    }
    // !Falling

    case OBJECT:
    {
      // !!! AMF3 version
      for(;;)
      {
        int len = chan.read_nbo_16();
        if (!len)
        {
          if (format == FORMAT_AMF0)
          {
            // Read and check object end marker
            uint8_t end = chan.read_byte();
            if (end != 0x09)
              throw Exception("Bad object end marker");
          }
          break;
        }

        string name;
        // !!! Check for AMF3 reference
        chan.read(name, len);
        Value v;
        v.read(chan, format);
        set(name, v);
      }
      break;
    }

    default: throw Exception("Unknown AMF marker "+Text::itos(type));
  }
}

//------------------------------------------------------------------------
// Get a value from an associative array/object
Value Value::get(const string& name) throw (Exception)
{
  if (type != ARRAY && type != OBJECT)
    throw Exception("Not an object or array");

  map<string, Value>::iterator p = assoc_array.find(name);
  if (p == assoc_array.end())
    throw Exception("Object/array property "+name+" not found");

  return p->second;
}

//------------------------------------------------------------------------
// Get a value from an associative array/object, checking for a specific type
Value Value::get(const string& name, Type type, const string& type_name)
  throw (Exception)
{
  Value v = get(name);
  if (v.type != type)
    throw Exception("Object/array property "+name
                        +" is not type "+type_name);
  return v;
}

//------------------------------------------------------------------------
// Get a boolean value from an associative array/object
bool Value::get_boolean(const string& name) throw (Exception)
{
  Value v = get(name);
  if (v.type == TRUE) return true;
  if (v.type == FALSE) return false;
  throw Exception("Object/array property "+name+" is not TRUE or FALSE");
}

//------------------------------------------------------------------------
// Write to a channel in the given format
void Value::write(Channel::Writer& chan, Format format)
  throw (Exception, Channel::Error)
{
  if (format == FORMAT_AMF0)
  {
    switch (type)
    {
      case UNDEFINED:
        chan.write_byte(0x06);
        break;

      case NULLV:
        chan.write_byte(0x05);
        break;

      case FALSE:
        chan.write_byte(0x01);
        chan.write_byte(0x00);
        break;

      case TRUE:
        chan.write_byte(0x01);
        chan.write_byte(0xFF);
        break;

      case STRING:
      case XML_DOC:
      {
        // Depends if we need more than 65535 bytes
        bool big = text.size() > 65535;
        chan.write_byte(type==STRING?(big?0x0C:0x02):0x0F);
        if (big || type==XML_DOC)  // XML always 32-bit
          chan.write_nbo_32(text.size());
        else
          chan.write_nbo_16(text.size());
        chan.write(text);
        break;
      }

      case INTEGER:
        // No INTEGER in AMF0 - cast to double
        chan.write_byte(0x00);
        chan.write_nbo_double((double)n);
        break;

      case DOUBLE:
      case DATE:
        chan.write_byte(type==DOUBLE?0x00:0x0B);
        chan.write_nbo_double(d);
        break;

      case ARRAY:
        // Depends whether dense or associative
        if (assoc_array.size())
        {
          chan.write_byte(0x08);
          chan.write_nbo_32(assoc_array.size());
          for(map<string, Value>::iterator p = assoc_array.begin();
              p!=assoc_array.end(); ++p)
          {
            // Write name
            chan.write_nbo_16(p->first.size());
            chan.write(p->first);

            // and value
            p->second.write(chan, format);
          }

          // !AMF0 spec doesn't say it but every parser/generate seems to
          // assume it - end marker required
          chan.write_nbo_16(0);
          chan.write_byte(0x09);  // object-end-marker
        }
        else
        {
          chan.write_byte(0x0A);
          chan.write_nbo_32(dense_array.size());
          for(vector<Value>::iterator p = dense_array.begin();
              p!=dense_array.end(); ++p)
            p->write(chan, format);
        }
        break;

      case OBJECT:
        chan.write_byte(0x03);
        for(map<string, Value>::iterator p = assoc_array.begin();
            p!=assoc_array.end(); ++p)
        {
          // Write name
          chan.write_nbo_16(p->first.size());
          chan.write(p->first);

          // and value
          p->second.write(chan, format);
        }

        // End marker
        chan.write_nbo_16(0);
        chan.write_byte(0x09);  // object-end-marker
        break;

      default:;
        throw Exception("AMF0 writing of type "+Text::itos(type)
                        +" not supported");
        break;
    }
  }
  else throw Exception("AMF3 writing not yet supported!");
}

//------------------------------------------------------------------------
// << operator to write a value to ostream
ostream& operator<<(ostream& s, const Value& v)
{
  return v.log(s);
}

}} // namespaces

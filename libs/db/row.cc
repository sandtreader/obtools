//==========================================================================
// ObTools::DB: row.cc
//
// Implementation of result row
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-db.h"

namespace ObTools { namespace DB {

//------------------------------------------------------------------------
//Get value of field of given name, or default if not found
string Row::get(string fieldname, const string& def) const
{
  map<string,string>::const_iterator p=fields.find(fieldname);
  if (p!=fields.end())
    return p->second;
  else
    return def;
}

//------------------------------------------------------------------------
//Get integer value of field of given name, or default if not found
int Row::get_int(string fieldname, int def) const
{
  map<string,string>::const_iterator p=fields.find(fieldname);
  if (p!=fields.end()) return atoi(p->second.c_str());

  return def;
}

//------------------------------------------------------------------------
//Get boolean value of field of given name, or default if not found
bool Row::get_bool(string fieldname, bool def) const
{
  map<string,string>::const_iterator p=fields.find(fieldname);
  if (p!=fields.end())
  {
    char c=0;
    if (!p->second.empty()) c=p->second[0];
      
    switch(c)
    {
      case 'T': case 't':
      case 'Y': case 'y':
	return true;

      default:
	return false;
    }
  }

  return def;
}


}} // namespaces

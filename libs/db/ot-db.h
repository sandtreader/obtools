//==========================================================================
// ObTools::DB: ot-db.h
//
// Public definitions for ObTools::DB Library
// General C++ database wrapper
// 
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#ifndef __OBTOOLS_DB_H
#define __OBTOOLS_DB_H

#include <map>
#include <iostream>

namespace ObTools { namespace DB { 

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Database row (same for all drivers)
class Row
{
private:
  map<string, string> fields;

public:
  //------------------------------------------------------------------------
  //Add name/value pair(Drivers only)
  void add(const string& fieldname, const string& value)
  { fields[fieldname] = value; }

  //------------------------------------------------------------------------
  //Finds whether the row contains a value for the given fieldname
  bool has(string fieldname) const
  { return fields.find(fieldname) != fields.end(); }

  //------------------------------------------------------------------------
  //Get value of field of given name, or default if not found
  string get(string fieldname, const string& def="") const;

  //------------------------------------------------------------------------
  //Handy [] operator
  // e.g. foo = row["id"];
  string operator[](string fieldname) const
  { return get(fieldname); }
  
  //------------------------------------------------------------------------
  //Get integer value of field of given name, or default if not found
  int get_int(string fieldname, int def=0) const;

  //------------------------------------------------------------------------
  //Get boolean value of field of given name, or default if not found
  bool get_bool(string fieldname, bool def=false) const;
};

//==========================================================================
// Database result set (abstract)
class ResultSet
{
public:
  //------------------------------------------------------------------------
  //Get number of rows in result set
  virtual int count()=0;

  //------------------------------------------------------------------------
  //Get next row from result set
  //Whether another was found - if so, writes into row
  virtual bool fetch(Row& row)=0;

  //------------------------------------------------------------------------
  //Get first value of next row from result set
  //Whether another was found - if so, writes into value
  virtual bool fetch(string& value)=0;

  //------------------------------------------------------------------------
  //Virtual destructor
  virtual ~ResultSet() {}
};

//==========================================================================
// Database result auto-ptr wrapper
// Allows inheritance of ResultSet while preserving single abstract interface
class Result
{
private:
  ResultSet *rset;

  // Helper to allow copy-by-value (see auto_ptr for source of this hack)
  struct Ref
  {
    ResultSet *rset;
    explicit Ref(ResultSet *r): rset(r) {}
  };

public:
  //------------------------------------------------------------------------
  //Constructors 
  Result(): rset(0) {}               // Invalid result
  Result(ResultSet *r): rset(r) {}   // Valid result

  //------------------------------------------------------------------------
  //Copy constructor and assignment operators
  // - detach rset from old one - note non-const
  Result(Result& r): rset(r.rset) { r.rset=0; }
  Result& operator=(Result& r) 
  { 
    if (rset && rset!=r.rset) delete rset;
    rset=r.rset; r.rset=0;
    return *this; 
  }

  //------------------------------------------------------------------------
  //Ditto from 'Ref' helper - combination of the two is claimed to allow
  //assignment by value - e.g.
  // Result res = my_func_returning_result();
  Result(Ref r): rset(r.rset) { }
  Result& operator=(Ref& r) 
  { 
    if (rset && rset!=r.rset) delete rset;
    rset=r.rset; 
    return *this; 
  }
  operator Ref() { ResultSet *t = rset; rset=0; return Ref(t); }

  //------------------------------------------------------------------------
  //Handy ! operator to check for (in)validity
  bool operator!() const { return !rset; }

  //------------------------------------------------------------------------
  //Get number of rows in result set
  int count() { return rset?rset->count():0; }

  //------------------------------------------------------------------------
  //Get next row from result set
  //Whether another was found - if so, writes into row
  bool fetch(Row& row) { return rset?rset->fetch(row):false; }

  //------------------------------------------------------------------------
  //Get first value of next row from result set
  //Whether another was found - if so, writes into value
  bool fetch(string& value) { return rset?rset->fetch(value):false; }

  //------------------------------------------------------------------------
  //Destructor - delete result set
  ~Result() { if (rset) delete rset; }
};

//==========================================================================
// Database connection (abstract)
class Connection
{
protected:
  bool valid;

public:
  //------------------------------------------------------------------------
  //Constructor
  Connection(): valid(false) {}

  //------------------------------------------------------------------------
  //Handy ! operator to check for (in)validity
  bool operator!() const { return !valid; }

  //==========================================================================
  // Virtual functions implemented by driver subclass

  //------------------------------------------------------------------------
  //Execute a command, not expecting any result (e.g. INSERT, UPDATE, DELETE)
  //Returns whether successful
  virtual bool exec(const string& sql)=0;

  //------------------------------------------------------------------------
  //Execute a query and get result (e.g. SELECT)
  //Returns result - check this for validity
  virtual Result query(const string& sql)=0;

  //------------------------------------------------------------------------
  //Get integer ID of last INSERT
  virtual int inserted_id()=0;

  //------------------------------------------------------------------------
  //Virtual destructor
  virtual ~Connection() {}

  //==========================================================================
  // Helper functions implemented in connection.cc

  //------------------------------------------------------------------------
  //Execute a query and get first (only) row
  //Returns whether successful - row is filled in if so
  bool query(const string& sql, Row& row);

  //------------------------------------------------------------------------
  //Execute a query and get single (only) value from first (only) row
  //Returns whether successful - value is filled in if so
  bool query(const string& sql, string& value);

  //------------------------------------------------------------------------
  //Execute a query and get single (only) value from first (only) row
  //Returns value or default if not found
  string query_string(const string& sql, const string& def="");

  //------------------------------------------------------------------------
  //Execute a query and get single (only) integer value from first (only) row
  //Returns value or default if not found
  int query_int(const string& sql, int def=0);

  //------------------------------------------------------------------------
  //Execute a query and get single (only) integer value from first (only) row
  //Returns value or default if not found
  bool query_bool(const string& sql, bool def=false);
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_DB_H




//==========================================================================
// ObTools::DB: transaction.cc
//
// Transaction holder
//
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-db.h"

namespace ObTools { namespace DB {

//------------------------------------------------------------------------
//Constructor 
Transaction::Transaction(Connection& _conn): conn(_conn), committed(false)
{
  if (!conn.exec("START TRANSACTION")) committed = true;  // Make commit fail
}

//------------------------------------------------------------------------
//Commit - returns whether commit command ran OK
bool Transaction::commit()
{
  if (committed) return false;
  return (committed = conn.exec("COMMIT"));
}

//------------------------------------------------------------------------
//Destructor
Transaction::~Transaction()
{
  if (!committed) conn.exec("ROLLBACK");
}



}} // namespaces

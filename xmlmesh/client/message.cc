//==========================================================================
// ObTools::XMLBus: message.cc
//
// Implementation of XMLBus message
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-xmlbus.h"
#include "ot-mt.h"
#include <sstream>

namespace ObTools { namespace XMLBus {

//--------------------------------------------------------------------------
//Static globals for ID allocation - threadsafe

static MT::Mutex id_mutex;
static int id_serial;

//--------------------------------------------------------------------------
//Constructor - allocates ID 
Message::Message(const string& _subject, const string& _content):
  subject(_subject), content(_content)
{
  MT::Lock lock(id_mutex);
  ostringstream ids;
  ids << "OTM-" << ++id_serial;
  id = ids.str();
}


}} // namespaces






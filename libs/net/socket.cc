//==========================================================================
// ObTools::Net: socket.cc
//
// C++ wrapper for BSD sockets
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-net.h"

namespace ObTools { namespace Net {

//--------------------------------------------------------------------------
// Default destructor - just close
Socket::~Socket()
{
  close(fd);
}


}} // namespaces




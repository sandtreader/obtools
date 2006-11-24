//==========================================================================
// ObTools::Crypto: library.cc
//
// Overall library initialisation/shutdown
//
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-crypto.h"
#include "openssl/evp.h"

namespace ObTools { namespace Crypto {

//------------------------------------------------------------------------
// Constructor
Library::Library()
{
  if (should_initialise()) OpenSSL_add_all_algorithms();
}

//------------------------------------------------------------------------
// Destructor
Library::~Library()
{
  // Clean up unless no-one has initialised before 
  if (!should_initialise()) EVP_cleanup();
}

}} // namespaces


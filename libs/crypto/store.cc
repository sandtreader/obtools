//==========================================================================
// ObTools::Crypto: store.cc
//
// X509 certificate store handling and certificate verification
//
// Copyright (c) 2008 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include <stdlib.h>
#include <sstream>
#include <iomanip>
#include <ctype.h>
#include "ot-chan.h"
#include "ot-crypto.h"

namespace ObTools { namespace Crypto {

//------------------------------------------------------------------------
// Constructor:
// ca_file should refer to a PEM format containing a list of trusted CAs
// ca_dir should refer to a directory containing certificate files with 
// hashed names (see OpenSSL docs)
// Either one or the other is optional, but not both
CertificateStore::CertificateStore(const string& ca_file, 
				   const string& ca_dir)
{
  store = X509_STORE_new();

  const char *file = ca_file.c_str();
  if (!*file) file = 0;

  const char *dir = ca_dir.c_str();
  if (!*dir) dir = 0;

  X509_STORE_load_locations(store, file, dir);
}

//------------------------------------------------------------------------
// Verify a certificate
bool CertificateStore::verify(const Certificate& cert)
{
  X509_STORE_CTX ctx;
  MT::Lock lock(mutex);

  // Verify certificate only, no other chain
  bool result = X509_STORE_CTX_init(&ctx, store, cert.get_x509(), 0)
             && X509_verify_cert(&ctx);

  X509_STORE_CTX_cleanup(&ctx);
  return result;
}

//------------------------------------------------------------------------
// Destructor
CertificateStore::~CertificateStore()
{
  X509_STORE_free(store);
}


}} // namespaces


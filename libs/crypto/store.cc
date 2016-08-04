//==========================================================================
// ObTools::Crypto: store.cc
//
// X509 certificate store handling and certificate verification
//
// Copyright (c) 2008 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
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
CertificateStore::CertificateStore(const string& ca_file,
                                   const string& ca_dir)
{
  store = X509_STORE_new();

  const char *file = ca_file.c_str();
  if (!*file) file = 0;

  const char *dir = ca_dir.c_str();
  if (!*dir) dir = 0;

  if (file || dir) X509_STORE_load_locations(store, file, dir);
}

//------------------------------------------------------------------------
// Add a pre-loaded certificateb
// Note: certicate must remain alive during lifetime of store
bool CertificateStore::add(Certificate *cert)
{
  MT::Lock lock(mutex);
  return X509_STORE_add_cert(store, cert->get_x509())?true:false;
}

//------------------------------------------------------------------------
// Add a CRL file and enable CRL checking in the store
// If 'all' is set, the entire chain is checked against the CRL
bool CertificateStore::add_crl(const string& crl_file, bool all)
{
  X509_LOOKUP *lookup = X509_STORE_add_lookup(store, X509_LOOKUP_file());
  if (!lookup) return false;

  if (X509_load_crl_file(lookup, crl_file.c_str(), X509_FILETYPE_PEM) != 1)
    return false;

  X509_STORE_set_flags(store, X509_V_FLAG_CRL_CHECK
                            | (all?X509_V_FLAG_CRL_CHECK_ALL:0));
  return true;
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


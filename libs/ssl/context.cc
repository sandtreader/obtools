//==========================================================================
// ObTools::SSL: context.cc
//
// C++ wrapper for SSL application context
//
// Copyright (c) 2008 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-ssl.h"
#include "ot-log.h"

namespace ObTools { namespace SSL {

//--------------------------------------------------------------------------
// Constructor: Allocates context
Context::Context()
{
  // Initialise library if not already done
  static bool initialised = false;
  if (!initialised)
  {
    SSL_library_init();
    SSL_load_error_strings();
    initialised = true;
  }

  // Provide all the options
  ctx = SSL_CTX_new(SSLv23_method());

  if (!ctx) log_errors("Can't create SSL context");
}

//--------------------------------------------------------------------------
// Use the given certificate
void Context::use_certificate(Crypto::Certificate& cert)
{
  if (ctx) SSL_CTX_use_certificate(ctx, cert.get_x509());
}

//--------------------------------------------------------------------------
// Use a certificate from a PEM-format string
// Returns whether valid
bool Context::use_certificate(const string& pem)
{
  if (!ctx) return false;

  Crypto::Certificate cert(pem);
  if (!cert) return false;

  use_certificate(cert);
  return true;
}

//--------------------------------------------------------------------------
// Use the given RSA private key
void Context::use_private_key(Crypto::RSAKey& rsa)
{
  if (ctx) SSL_CTX_use_RSAPrivateKey(ctx, rsa.rsa);
}

//--------------------------------------------------------------------------
// Use a private key from a PEM-format string with optional pass-phrase
// Returns whether valid
bool Context::use_private_key(const string& pem, const string& pass_phrase)
{
  if (!ctx) return false;

  Crypto::RSAKey rsa(pem, true, pass_phrase);
  if (!rsa.valid) return false;

  use_private_key(rsa);
  return true;
}

//--------------------------------------------------------------------------
// Create a new SSL connection from the context on the given fd
OpenSSL *Context::create_connection(int fd)
{
  if (!ctx) return 0;

  OpenSSL *ssl = SSL_new(ctx);
  if (ssl)
  {
    // Attach to fd
    if (!SSL_set_fd(ssl, fd))
    {
      log_errors("Can't attach SSL to fd");
      return 0;
    }

    return ssl;
  }

  log_errors("Can't create SSL connection structure");
  return 0;
}

//--------------------------------------------------------------------------
// Destructor: Deallocates context
Context::~Context()
{
  if (ctx) SSL_CTX_free(ctx);
}

//--------------------------------------------------------------------------
// Static:  Log SSL errors
void Context::log_errors(const string& text)
{
  Log::Streams log;
  log.error << "SSL: " << text << endl;

  unsigned long err;
  while (err = ERR_get_error())
  {
    char buf[120];
    log.error << ERR_error_string(err, buf) << endl;
  }
}

}} // namespaces




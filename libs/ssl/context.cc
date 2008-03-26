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
    OpenSSL::SSL_library_init();
    OpenSSL::SSL_load_error_strings();
    initialised = true;
  }

  // Provide all the options
  ctx = OpenSSL::SSL_CTX_new(OpenSSL::SSLv23_method());

  if (!ctx) log_errors("Can't create SSL context");
}

//--------------------------------------------------------------------------
// Use the given certificate
void Context::use_certificate(Crypto::Certificate& cert)
{
  if (ctx) SSL_CTX_use_certificate(ctx, cert.get_x509());
}

//--------------------------------------------------------------------------
// Use the given RSA private key
void Context::use_private_key(Crypto::RSAKey& rsa)
{
  if (ctx) SSL_CTX_use_RSAPrivateKey(ctx, rsa.rsa);
}

//--------------------------------------------------------------------------
// Create a new SSL connection from the context on the given fd
OpenSSL::SSL *Context::create_connection(int fd)
{
  if (!ctx) return 0;

  OpenSSL::SSL *ssl = OpenSSL::SSL_new(ctx);
  if (ssl)
  {
    // Attach to fd
    if (!OpenSSL::SSL_set_fd(ssl, fd))
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
  if (ctx) OpenSSL::SSL_CTX_free(ctx);
}

//--------------------------------------------------------------------------
// Static:  Log SSL errors
void Context::log_errors(const string& text)
{
  Log::Streams log;
  log.error << "SSL: " << text << endl;

  unsigned long err;
  while (err = OpenSSL::ERR_get_error())
  {
    char buf[120];
    log.error << OpenSSL::ERR_error_string(err, buf) << endl;
  }
}

}} // namespaces




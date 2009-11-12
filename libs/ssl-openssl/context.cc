//==========================================================================
// ObTools::SSL: context.cc
//
// C++ wrapper for SSL application context
//
// Copyright (c) 2008 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-ssl-openssl.h"
#include "ot-log.h"
#include "ot-text.h"

namespace ObTools { namespace SSL_OpenSSL {

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
// Enable peer certificate verification
// Set 'force' to require them, otherwise optional
void Context::enable_peer_verification(bool force)
{
  if (ctx) 
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER 
		            | (force?SSL_VERIFY_FAIL_IF_NO_PEER_CERT:0),
		       0);
}

//--------------------------------------------------------------------------
// Use given verify locations (list of trusted CAs)
// ca_file should refer to a PEM format containing a list of trusted CAs
// ca_dir should refer to a directory containing certificate files with hashed
// names (see OpenSSL docs)
// Either one or the other is optional, but not both
void Context::set_verify_paths(const string& ca_file, const string& ca_dir)
{
  const char *file = ca_file.empty()?0:ca_file.c_str();
  const char *dir  = ca_dir.empty() ?0:ca_dir.c_str();

  if (ctx && SSL_CTX_load_verify_locations(ctx, file, dir) != 1)
    log_errors("Can't load verify locations");
}

//--------------------------------------------------------------------------
// Use default verify paths
void Context::set_default_verify_paths()
{
  if (ctx && SSL_CTX_set_default_verify_paths(ctx) != 1)
    log_errors("Can't set default verify paths");
}

//--------------------------------------------------------------------------
// Set session ID context
void Context::set_session_id_context(const string& s)
{
  if (ctx) SSL_CTX_set_session_id_context(ctx, (const unsigned char *)s.data(),
					  s.length());
}

//--------------------------------------------------------------------------
// Create a new SSL connection from the context on the given fd, and 
// accept() it
Connection *Context::accept_connection(int fd)
{
  if (!ctx) return 0;

  OpenSSL *ssl = SSL_new(ctx);
  if (ssl)
  {
    // Attach to fd
    if (!SSL_set_fd(ssl, fd))
    {
      log_errors("Can't attach SSL to fd");
      SSL_free(ssl);
      return 0;
    }

    int ret = SSL_accept(ssl);
    if (ret < 1)
    {
      log_errors("Failed to accept SSL");
      SSL_free(ssl);
      return 0;
    }

    return new Connection(ssl);
  }

  log_errors("Can't create SSL connection structure");
  return 0;
}

//--------------------------------------------------------------------------
// Create a new SSL connection from the context on the given fd and
// connect() it
Connection *Context::connect_connection(int fd)
{
  if (!ctx) return 0;

  OpenSSL *ssl = SSL_new(ctx);
  if (ssl)
  {
    // Attach to fd
    if (!SSL_set_fd(ssl, fd))
    {
      log_errors("Can't attach SSL to fd");
      SSL_free(ssl);
      return 0;
    }

    int ret = SSL_connect(ssl);
    if (ret < 1)
    {
      log_errors("Failed to connect SSL");
      SSL_free(ssl);
      return 0;
    }

    return new Connection(ssl);
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
  while ((err = ERR_get_error()) != 0)
  {
    char buf[120];
    log.error << ERR_error_string(err, buf) << endl;
  }
}

//--------------------------------------------------------------------------
// Static:  Create from an <ssl> configuration element
// Returns context, or 0 if disabled or failed
Context *Context::create(XML::Element& ssl_e)
{
  if (!ssl_e.get_attr_bool("enabled")) return 0;

  XML::XPathProcessor xpath(ssl_e);
  Log::Streams log;

  // Get SOAP RSA pass-phrase first, if required
  string ssl_pass_phrase;
  if (xpath.get_value_bool("private-key/@encrypted"))
  {
    log.summary << "SSL RSA key pass phrase required\n";
    cout << "\n** Enter pass phrase for RSA private key: ";
    cin >> ssl_pass_phrase;
  }

  // Get private key, strip blank lines, indent
  string key = xpath.get_value("private-key");
  key = Text::strip_blank_lines(key);
  key = Text::remove_indent(key, Text::get_common_indent(key));

  // Test the key
  Crypto::RSAKey rsa(key, true, ssl_pass_phrase);
  if (!rsa.valid)
  {
    log.error << "Invalid RSA private key or pass phrase - giving up\n";
    return 0;
  }

  log.summary << "RSA key loaded OK\n";

  Context *ssl_ctx = new Context();
  ssl_ctx->use_private_key(rsa);

  // Get certificate
  string cert = xpath.get_value("certificate");
  cert = Text::strip_blank_lines(cert);
  cert = Text::remove_indent(cert, Text::get_common_indent(cert));

  if (ssl_ctx->use_certificate(cert))
  {
    log.summary << "SSL context initialised OK\n";
  }
  else
  {
    log.error << "Can't use SSL certificate - disabling\n";
    delete ssl_ctx;
      return 0;
  }

  // Enable verification if requested
  if (xpath.get_value_bool("verify/@enabled"))
  {
    bool mandatory = xpath.get_value_bool("verify/@mandatory");
    ssl_ctx->enable_peer_verification(mandatory);

    // Load CA file/directory
    ssl_ctx->set_verify_paths(xpath.get_value("verify/root/file"),
			      xpath.get_value("verify/root/directory"));

    // Optionally load defaults
    if (xpath.get_value_bool("verify/root/@defaults"))
      ssl_ctx->set_default_verify_paths();
  }

  // Set up session ID context
  ssl_ctx->set_session_id_context(xpath.get_value("session/@context", "pst"));

  return ssl_ctx;
}


}} // namespaces




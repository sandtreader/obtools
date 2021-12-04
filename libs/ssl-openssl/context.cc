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
void Context::use_certificate(Crypto::Certificate& cert, bool is_extra)
{
  if (ctx)
  {
    if (is_extra)
      SSL_CTX_add_extra_chain_cert(ctx, cert.get_x509());
    else
      SSL_CTX_use_certificate(ctx, cert.get_x509());

    Log::Detail log;
    log << "Loaded " << (is_extra?"extra":"main") << " certificate for "
        << cert.get_cn() << endl;
  }
}

//--------------------------------------------------------------------------
// Use a certificate from a PEM-format string
// Returns whether valid
bool Context::use_certificate(const string& pem, bool is_extra)
{
  if (!ctx) return false;

  Crypto::Certificate cert(pem);
  if (!cert) return false;

  use_certificate(cert, is_extra);
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
// Get the index for picking the Context* from an SSL context
static int get_ssl_ctx_index()
{
  static int index = SSL_CTX_get_ex_new_index(0, 0, 0, 0, 0);
  return index;
}

//--------------------------------------------------------------------------
// Verify callback function to check common name
static int verify_common_name_callback(int preverify_ok,
                                       X509_STORE_CTX *x509_ctx)
{
  if (!preverify_ok)
    return 0;

  // If we're somewhere on the chain that isn't the peer, we aren't interested
  int depth = X509_STORE_CTX_get_error_depth(x509_ctx);
  if (depth)
    return 1;

  if (x509_ctx)
  {
    ::SSL *ssl = reinterpret_cast<::SSL *>(X509_STORE_CTX_get_ex_data(
          x509_ctx, SSL_get_ex_data_X509_STORE_CTX_idx()));
    SSL_CTX *ssl_ctx = SSL_get_SSL_CTX(ssl);
    Context *ctx = reinterpret_cast<Context *>(SSL_CTX_get_ex_data(ssl_ctx,
                                                 get_ssl_ctx_index()));
    if (ctx)
    {
      X509 *current_cert = X509_STORE_CTX_get_current_cert(x509_ctx);
      char buff[256];
      X509_NAME_get_text_by_NID(X509_get_subject_name(current_cert),
                                NID_commonName, buff, sizeof(buff));
      buff[sizeof(buff) - 1] = '\0';
      return (ctx->verify_common_name == buff);
    }
  }
  return 0;
}

//--------------------------------------------------------------------------
// Enable peer certificate verification
// Set 'force' to require them, otherwise optional
void Context::enable_peer_verification(bool force, bool common_name)
{
  if (ctx)
  {
    if (common_name)
    {
      SSL_CTX_set_ex_data(ctx, get_ssl_ctx_index(), this);
    }
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER
                            | (force?SSL_VERIFY_FAIL_IF_NO_PEER_CERT:0),
                       (common_name?verify_common_name_callback:0));
  }
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
// Use given file of CA certs as list of CA's to request from clients
void Context::set_client_ca_file(const string& ca_file)
{
  const char *file = ca_file.c_str();
  STACK_OF(X509_NAME) *cert_names;
  cert_names = SSL_load_client_CA_file(file);
  if (cert_names)
   SSL_CTX_set_client_CA_list(ctx, cert_names);
 else
   log_errors("Can't load client CA file");
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
  if (ctx) SSL_CTX_set_session_id_context(ctx,
                          reinterpret_cast<const unsigned char *>(s.data()),
                          s.length());
}

//--------------------------------------------------------------------------
// Create a new SSL connection from the context on the given fd, and
// accept() it
Connection *Context::accept_connection(int fd)
{
  if (!ctx) return 0;

  ::SSL *ssl = SSL_new(ctx);
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
      // Use temporary socket object to get address
      Net::TCPSocket socket(fd);
      Net::EndPoint remote = socket.remote();
      socket.detach_fd();
      log_errors("Failed to accept SSL from "+remote.host.get_dotted_quad());
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

  ::SSL *ssl = SSL_new(ctx);
  if (ssl)
  {
    // Attach to fd
    if (!SSL_set_fd(ssl, fd))
    {
      log_errors("Can't attach SSL to fd");
      SSL_free(ssl);
      return 0;
    }

    // Set SNI hostname, if known
    if (!sni_hostname.empty())
      SSL_set_tlsext_host_name(ssl, sni_hostname.c_str());

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
  log.error << "SSL: " << text << " - ";

  unsigned long err;
  while ((err = ERR_get_error()) != 0)
  {
    char buf[120];
    log.error << ERR_error_string(err, buf) << ". ";
  }

  log.error << endl;
}

//--------------------------------------------------------------------------
// Static:  Configure verification from an <ssl> configuration element
void Context::configure_verification(Context *ssl_ctx,
                                     const XML::Element& ssl_e)
{
  XML::ConstXPathProcessor xpath(ssl_e);

  // Enable verification if requested
  if (xpath.get_value_bool("verify/@enabled"))
  {
    bool mandatory = xpath.get_value_bool("verify/@mandatory");
    ssl_ctx->verify_common_name = xpath.get_value("verify/@common-name");

    ssl_ctx->enable_peer_verification(mandatory,
                                      !ssl_ctx->verify_common_name.empty());

    // Load CA file/directory
    ssl_ctx->set_verify_paths(xpath.get_value("verify/root/file"),
                              xpath.get_value("verify/root/directory"));

    // Optionally load defaults
    if (xpath.get_value_bool("verify/root/@defaults"))
      ssl_ctx->set_default_verify_paths();

    // Load list of acceptable client CAs
    string client_ca_file = xpath.get_value("verify/client/ca-file");
    if (!client_ca_file.empty())
      ssl_ctx->set_client_ca_file(client_ca_file);
  }
}

//--------------------------------------------------------------------------
// Static:  Create from an <ssl> configuration element
// Returns context, or 0 if disabled or failed
Context *Context::create(const XML::Element& ssl_e, string pass_phrase)
{
  if (!ssl_e.get_attr_bool("enabled")) return 0;

  XML::ConstXPathProcessor xpath(ssl_e);
  Log::Streams log;

  // Get SOAP RSA pass-phrase first, if required
  if (xpath.get_value_bool("private-key/@encrypted") && pass_phrase.empty())
  {
    log.summary << "SSL RSA key pass phrase required\n";
    cout << "\n** Enter pass phrase for RSA private key: ";
    cin >> pass_phrase;
  }

  Context *ssl_ctx = new Context();

  // Get private key, strip blank lines, indent
  string key = xpath.get_value("private-key");
  key = Text::strip_blank_lines(key);
  key = Text::remove_indent(key, Text::get_common_indent(key));
  if (!key.empty())
  {
    // Test the key
    Crypto::RSAKey rsa(key, true, pass_phrase);
    if (!rsa.valid)
    {
      log.error << "Invalid RSA private key or pass phrase - giving up\n";
      return 0;
    }

    log.summary << "RSA key loaded OK\n";
    ssl_ctx->use_private_key(rsa);
  }

  // Get certificates
  auto certs = xpath.get_elements("certificate");
  bool is_extra = false;
  for(auto cert_e: certs)
  {
    string cert = **cert_e;
    cert = Text::strip_blank_lines(cert);
    cert = Text::remove_indent(cert, Text::get_common_indent(cert));

    if (!ssl_ctx->use_certificate(cert, is_extra))
    {
      log.error << "Can't use SSL certificate - disabling\n";
      delete ssl_ctx;
      return 0;
    }

    is_extra = true;
  }

  configure_verification(ssl_ctx, ssl_e);

  // Set up session ID context
  ssl_ctx->set_session_id_context(xpath.get_value("session/@context", "pst"));

  log.summary << "SSL context initialised OK\n";
  return ssl_ctx;
}

//--------------------------------------------------------------------------
// Static:  Create from an <ssl> configuration element with no key or cert
// Returns context, or 0 if disabled or failed
Context *Context::create_anonymous(const XML::Element& ssl_e)
{
  if (!ssl_e.get_attr_bool("enabled")) return 0;

  XML::ConstXPathProcessor xpath(ssl_e);
  Log::Streams log;

  Context *ssl_ctx = new Context();

  configure_verification(ssl_ctx, ssl_e);

  // Set up session ID context
  ssl_ctx->set_session_id_context(xpath.get_value("session/@context", "pst"));

  return ssl_ctx;
}


}} // namespaces




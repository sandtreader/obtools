//==========================================================================
// ObTools::SSL: ot-ssl-openssl.h
//
// Public definitions for ObTools::SSL
// SSL/TLS Socket functions - wrapper around libssl
// 
// Copyright (c) 2009 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_SSL_OPENSSL_H
#define __OBTOOLS_SSL_OPENSSL_H

#include "ot-ssl.h"
#include "ot-crypto.h"

// This is rather ugly...  We want to use SSL as a namespace, but
// OpenSSL defines it as a struct.  Hence we redefine SSL here to 
// expand to OpenSSL for the duration of the OpenSSL header 
#define SSL OpenSSL
#include <openssl/ssl.h>
#undef SSL

#include <openssl/err.h>

namespace ObTools { namespace SSL_OpenSSL { 

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Abstract SSL connection
class Connection: public SSL::Connection
{
private:
  OpenSSL *ssl;

public:
  //--------------------------------------------------------------------------
  // Constructor, takes OpenSSL (=SSL) object
  Connection(OpenSSL *_ssl): ssl(_ssl) {}

  //--------------------------------------------------------------------------
  // Raw stream read wrapper
  ssize_t cread(void *buf, size_t count);

  //--------------------------------------------------------------------------
  // Raw stream write wrapper
  ssize_t cwrite(const void *buf, size_t count);

  //--------------------------------------------------------------------------
  // Get peer's X509 common name
  string get_peer_cn();

  //--------------------------------------------------------------------------
  // Destructor
  ~Connection();
};

//==========================================================================
// SSL application context
class Context: public SSL::Context
{
  SSL_CTX *ctx;  // OpenSSL library context

public:
  string verify_common_name;

  //--------------------------------------------------------------------------
  // Constructor: Allocates context
  Context();

  //--------------------------------------------------------------------------
  // Use the given certificate
  void use_certificate(Crypto::Certificate& cert);

  //--------------------------------------------------------------------------
  // Use a certificate from a PEM-format string
  // Returns whether valid
  bool use_certificate(const string& pem);

  //--------------------------------------------------------------------------
  // Use the given RSA private key
  void use_private_key(Crypto::RSAKey& rsa);

  //--------------------------------------------------------------------------
  // Use a private key from a PEM-format string, with optional pass-phrase
  // Returns whether valid
  bool use_private_key(const string& pem, const string& pass_phrase="");

  //--------------------------------------------------------------------------
  // Enable peer certificate verification
  // Set 'force' to require them, otherwise optional
  // Set 'common_name' to require the CN to match verify_common_name
  void enable_peer_verification(bool force = false, bool common_name = false);

  //--------------------------------------------------------------------------
  // Use given verify locations (list of trusted CAs)
  // ca_file should refer to a PEM format containing a list of trusted CAs
  // ca_dir should refer to a directory containing certificate files with 
  // hashed names (see OpenSSL docs)
  // Either one or the other is optional, but not both
  void set_verify_paths(const string& ca_file="", const string& ca_dir="");

  //--------------------------------------------------------------------------
  // Use default verify paths
  void set_default_verify_paths();

  //--------------------------------------------------------------------------
  // Use given file of CA certs as list of CA's to request from clients
  void set_client_ca_file(const string& ca_file);

  //--------------------------------------------------------------------------
  // Set session ID context
  void set_session_id_context(const string& s);

  //--------------------------------------------------------------------------
  // Create a new SSL connection from the context and bind it to the given fd
  // and accept() it
  Connection *accept_connection(int fd);

  //--------------------------------------------------------------------------
  // Create a new SSL connection from the context and bind it to the given fd
  // and connect() it
  Connection *connect_connection(int fd);

  //--------------------------------------------------------------------------
  // Destructor: Deallocates context
  ~Context();

  //--------------------------------------------------------------------------
  // Static:  Log SSL errors
  static void log_errors(const string& text);

  //--------------------------------------------------------------------------
  // Static:  Set verification options from an <ssl> configuration element
  static void configure_verification(Context *ssl_ctx, XML::Element& ssl_e);

  //--------------------------------------------------------------------------
  // Static:  Create from an <ssl> configuration element
  // Returns context, or 0 if disabled or failed
  static Context *create(XML::Element& ssl_e, string pass_phrase = "");

  //--------------------------------------------------------------------------
  // Static:  Create from an <ssl> configuration element with no key or cert
  // Returns context, or 0 if disabled or failed
  static Context *create_anonymous(XML::Element& ssl_e);
};


//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_SSL_H

















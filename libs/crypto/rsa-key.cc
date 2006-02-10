//==========================================================================
// ObTools::Crypto: rsa-key.cc
//
// RSA key handling, to/from PEM format
//
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include <stdlib.h>
#include <sstream>
#include <iomanip>
#include <ctype.h>
#include "ot-chan.h"
#include "ot-crypto.h"

namespace OpenSSL
{
#include "openssl/bio.h"
#include "openssl/buffer.h"
#include "openssl/pem.h"
}

namespace ObTools { namespace Crypto {

//------------------------------------------------------------------------
// Create a new key from random data
// Must seed PRNG first 
void RSAKey::create(int size, int exponent)
{
  rsa = OpenSSL::RSA_generate_key(size, exponent, 0, 0);
  if (rsa) valid = true;
}

//------------------------------------------------------------------------
// Read from stream - reads PEM format to EOF
// If force_private is set, reads a private key PEM block even if a
// public key is wanted (use for testing both ends with a single key)
void RSAKey::read(istream& sin, bool force_private)
{
  string key;
  // Read all we can into the string
  while (sin) key+=sin.get();
  read(key, force_private);
}

//------------------------------------------------------------------------
// Write to stream - writes PEM format
// If force_public is set, writes a public key PEM block even if
// a private key is held - use to generate new public/private pairs 
void RSAKey::write(ostream& sout, bool force_public) const
{
  sout << str(force_public);
}

//------------------------------------------------------------------------
// Read from string - reads PEM format
// If force_private is set, reads a private key even if public (see above)
void RSAKey::read(const string& text, bool force_private)
{
  int length = text.size()+1; // C string

  // Create 'BIO'
  OpenSSL::BIO *bio = OpenSSL::BIO_new(OpenSSL::BIO_s_mem());
  if (!bio) return;

  // Create memory buffer
  OpenSSL::BUF_MEM *buf = OpenSSL::BUF_MEM_new();
  if (!buf) return;
  OpenSSL::BUF_MEM_grow(buf, length);
  memcpy(buf->data, text.c_str(), length);

  // Attach to BIO (auto free of buf)
  OpenSSL::BIO_set_mem_buf(bio, buf, BIO_CLOSE);

  // Read key
  if (is_private || force_private)
    rsa = OpenSSL::PEM_read_bio_RSAPrivateKey(bio, 0, 0, 0);
  else
    rsa = OpenSSL::PEM_read_bio_RSA_PUBKEY(bio, 0, 0, 0);

  // Clean up
  OpenSSL::BIO_free(bio);

  if (rsa) valid = true;
}

//------------------------------------------------------------------------
// Convert to PEM format string
// force_public forces public key output from private key (see above)
string RSAKey::str(bool force_public) const
{
  if (!valid) return "INVALID!";

  // Create 'BIO'
  OpenSSL::BIO *bio = OpenSSL::BIO_new(OpenSSL::BIO_s_mem());
  if (!bio) return "";

  // Write key
  if (is_private && !force_public)
    OpenSSL::PEM_write_bio_RSAPrivateKey(bio, rsa, 0, 0, 0, 0, 0);
  else
    OpenSSL::PEM_write_bio_RSA_PUBKEY(bio, rsa);

  // Grr. BIO_flush is a macro so we have to hack the namespaces...
  {
    using namespace OpenSSL;
    BIO_flush(bio);
  }

  // Get buffer
  OpenSSL::BUF_MEM *buf;
  OpenSSL::BIO_get_mem_ptr(bio, &buf);
  string key(buf->data, buf->length);

  OpenSSL::BIO_free(bio);  // buf goes too
  return key;
}
  
//------------------------------------------------------------------------
// >> operator to read key from istream
istream& operator>>(istream& s, RSAKey& k)
{
  k.read(s);
  return s;
}

//------------------------------------------------------------------------
// << operator to write key to ostream
ostream& operator<<(ostream& s, const RSAKey& k)
{
  k.write(s);
  return s;
}

//------------------------------------------------------------------------
// Destructor
RSAKey::~RSAKey()
{
  if (rsa) OpenSSL::RSA_free(rsa);
}

}} // namespaces


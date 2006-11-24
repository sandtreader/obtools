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

#include "openssl/bio.h"
#include "openssl/buffer.h"
#include "openssl/pem.h"

namespace ObTools { namespace Crypto {

//------------------------------------------------------------------------
// Create a new key from random data
// Must seed PRNG first 
void RSAKey::create(int size, int exponent)
{
  rsa = RSA_generate_key(size, exponent, 0, 0);
  if (rsa) valid = true;
}

//------------------------------------------------------------------------
// Read from stream - reads PEM format to EOF
// If force_private is set, reads a private key PEM block even if a
// public key is wanted (use for testing both ends with a single key)
void RSAKey::read(istream& sin, const string& pass_phrase,
		  bool force_private)
{
  string key;
  // Read all we can into the string
  while (sin) key+=sin.get();
  read(key, pass_phrase, force_private);
}

//------------------------------------------------------------------------
// Write to stream - writes PEM format
// If force_public is set, writes a public key PEM block even if
// a private key is held - use to generate new public/private pairs 
void RSAKey::write(ostream& sout, const string& pass_phrase,
		   bool force_public) const
{
  sout << str(pass_phrase, force_public);
}

//------------------------------------------------------------------------
// Read from string - reads PEM format, with pass phrase
// If force_private is set, reads a private key even if public (see above)
void RSAKey::read(const string& text, 
		  const string& pass_phrase,
		  bool force_private)
{
  int length = text.size()+1; // C string

  // Create 'BIO'
  BIO *bio = BIO_new(BIO_s_mem());
  if (!bio) return;

  // Create memory buffer
  BUF_MEM *buf = BUF_MEM_new();
  if (!buf) return;
  BUF_MEM_grow(buf, length);
  memcpy(buf->data, text.c_str(), length);

  // Attach to BIO (auto free of buf)
  BIO_set_mem_buf(bio, buf, BIO_CLOSE);

  // Get passphrase string
  const char *pp = 0;
  if (!pass_phrase.empty()) pp = pass_phrase.c_str();

  // Read key
  if (is_private || force_private)
    rsa = PEM_read_bio_RSAPrivateKey(bio, 0, 0, (void *)pp);
  else
    rsa = PEM_read_bio_RSA_PUBKEY(bio, 0, 0, 0);

  // Clean up
  BIO_free(bio);

  if (rsa) valid = true;
}

//------------------------------------------------------------------------
// Convert to PEM format string with pass phrase
// force_public forces public key output from private key (see above)
string RSAKey::str(const string& pass_phrase, bool force_public) const
{
  if (!valid) return "INVALID!";

  // Create 'BIO'
  BIO *bio = BIO_new(BIO_s_mem());
  if (!bio) return "";

  // Get passphrase string
  const char *pp = 0;
  const EVP_CIPHER *enc = 0;

  if (!pass_phrase.empty())
  {
    enc = EVP_des_ede3_cbc();
    pp = pass_phrase.c_str();
  }

  // Write key
  if (is_private && !force_public)
    PEM_write_bio_RSAPrivateKey(bio, rsa, enc, 0, 0, 0, (void *)pp);
  else
    PEM_write_bio_RSA_PUBKEY(bio, rsa);

  BIO_flush(bio);

  // Get buffer
  BUF_MEM *buf;
  BIO_get_mem_ptr(bio, &buf);
  string key(buf->data, buf->length);

  BIO_free(bio);  // buf goes too
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
  if (rsa) RSA_free(rsa);
}

}} // namespaces


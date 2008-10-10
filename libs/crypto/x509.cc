//==========================================================================
// ObTools::Crypto: x509.cc
//
// X509 certificate handling, to/from PEM format, member access
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

#include "openssl/bio.h"
#include "openssl/buffer.h"
#include "openssl/pem.h"

// PEM delimiters
#define PEM_CERT_START "-----BEGIN CERTIFICATE-----\n"
#define PEM_CERT_END "-----END CERTIFICATE-----\n"

namespace ObTools { namespace Crypto {

//------------------------------------------------------------------------
// Read from stream - reads PEM or DER format to EOF
void Certificate::read(istream& sin)
{
  string text;
  // Read all we can into the string
  while (sin) text+=sin.get();
  read(text);
}

//------------------------------------------------------------------------
// Write to stream - writes PEM format
void Certificate::write(ostream& sout) const
{
  sout << str();
}

//------------------------------------------------------------------------
// Read from string - reads PEM or DER format
void Certificate::read(const string& text)
{
  int length = text.size();
  const unsigned char *data = (const unsigned char *)text.data();

  // Scan for top-bit set characters indicating DER format
  bool der = false;
  for(int i=0; i<length; i++)
  {
    if (data[i] & 0x80)
    {
      der = true;
      break;
    }
  }

  // If PEM, fix up text to ensure delimiters are present
  string fixed;
  if (!der && text.find(PEM_CERT_START) == string::npos)
  {
    fixed = PEM_CERT_START + text + PEM_CERT_END;
    length = fixed.size();
    data = (const unsigned char *)fixed.data();
  }

  // Create 'BIO'
  BIO *bio = BIO_new(BIO_s_mem());
  if (!bio) return;

  // Create memory buffer
  BUF_MEM *buf = BUF_MEM_new();
  if (!buf) return;
  BUF_MEM_grow(buf, length);
  memcpy(buf->data, data, length);

  // Attach to BIO (auto free of buf)
  BIO_set_mem_buf(bio, buf, BIO_CLOSE);

  // Read certificate
  if (der)
    x509 = d2i_X509_bio(bio, 0);
  else
    x509 = PEM_read_bio_X509(bio, 0, 0, 0);

  // Clean up
  BIO_free(bio);
}

//------------------------------------------------------------------------
// Convert to PEM format string 
string Certificate::str() const
{
  if (!x509) return "INVALID!";

  // Create 'BIO'
  BIO *bio = BIO_new(BIO_s_mem());
  if (!bio) return "";

  // Write certificate
  PEM_write_bio_X509(bio, x509);
  BIO_flush(bio);

  // Get buffer
  BUF_MEM *buf;
  BIO_get_mem_ptr(bio, &buf);
  string pem(buf->data, buf->length);

  BIO_free(bio);  // buf goes too
  return pem;
}

//------------------------------------------------------------------------
// Get common name
string Certificate::get_cn() const
{
  if (!x509) return "INVALID!";

  char buf[256];
  buf[0] = 0;
  X509_NAME *subject = X509_get_subject_name(x509);
  if (subject) X509_NAME_get_text_by_NID(subject, NID_commonName, buf, 256);
  return string(buf);
}

//------------------------------------------------------------------------
// Get RSA public key into the given RSAKey
// Returns whether successful, also sets key validity
bool Certificate::get_public_key(RSAKey& key) const
{
  key.valid = false;  // Be pessimistic
  if (!x509) return false;

  EVP_PKEY *evp_pkey = X509_get_pubkey(x509);
  if (!evp_pkey) return false;

  key.rsa = EVP_PKEY_get1_RSA(evp_pkey);
  if (!key.rsa) return false;

  key.is_private = false;
  key.valid = true;
  return true;
}

//------------------------------------------------------------------------
// >> operator to read cert from istream
istream& operator>>(istream& s, Certificate& c)
{
  c.read(s);
  return s;
}

//------------------------------------------------------------------------
// << operator to write certificate to ostream
ostream& operator<<(ostream& s, const Certificate& c)
{
  c.write(s);
  return s;
}

//------------------------------------------------------------------------
// Destructor
Certificate::~Certificate()
{
  if (x509) X509_free(x509);
}

}} // namespaces


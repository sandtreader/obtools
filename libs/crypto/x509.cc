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

namespace ObTools { namespace Crypto {

//------------------------------------------------------------------------
// Read from stream - reads PEM format to EOF
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
// Read from string - reads PEM format
void Certificate::read(const string& text)
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

  // Read certificate
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
  X509_NAME_get_text_by_NID(X509_get_subject_name(x509), NID_commonName,
			    buf, 256);
  return string(buf);
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


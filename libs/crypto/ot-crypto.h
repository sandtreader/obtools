//==========================================================================
// ObTools::Crypto: ot-crypto.h
//
// Public definitions for ObTools::Crypto
// C++ wrappers for crypto functions, (currently) using OpenSSL
// 
// Copyright (c) 2006 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

// Include ot-chan.h first to enable Channel functions

#ifndef __OBTOOLS_CRYPTO_H
#define __OBTOOLS_CRYPTO_H

#include <stdint.h>
#include <string>
#include <map>
#include <string.h>

// This is rather ugly...  We want to use SSL as a namespace, but
// OpenSSL defines it as a struct.  Hence we redefine SSL here to 
// expand to OpenSSL for the duration of the OpenSSL headers 
#define SSL OpenSSL
#include <openssl/aes.h>
#include <openssl/des.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>
#undef SSL

#include "ot-mt.h"

namespace ObTools { namespace Crypto { 

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Overall library initialiser singleton
// Keep one of these alive for the entire operation of the program
class Library
{
private:
  // One-shot initialiser - returns true only first time called
  static bool should_initialise()
  { static bool i = false; if (i) return false; i = true; return true; }

public:
  //------------------------------------------------------------------------
  // Constructor
  Library();

  //------------------------------------------------------------------------
  // Destructor
  ~Library();
};

//==========================================================================
// AES key (aes-key.cc)
// 16, 24 or 32 byte key, also used for IV
class AESKey
{
public:
  unsigned char key[32];          // Key
  enum Size
  {
    BITS_128 = 128,
    BITS_192 = 192,
    BITS_256 = 256,
  } size;
  bool is_key;                    // True if a key, false if an IV
  bool valid;                     // Whether key/schedule is valid

  //------------------------------------------------------------------------
  // Default constructor
  AESKey(Size _size = BITS_128, bool _is_key = true):
    size(_size), is_key(_is_key), valid(false)
  {
    memset(&key, 0, sizeof(key));
  }

  //------------------------------------------------------------------------
  // Constructor from data block
  AESKey(const unsigned char *data, Size _size):
    size(_size), is_key(true)
  {
    read(data);
  }

  //------------------------------------------------------------------------
  // Constructor from string - requires hex characters
  AESKey(const string& text, Size _size):
    size(_size), is_key(true)
  {
    read(text);
  }

  //------------------------------------------------------------------------
  // Destructor
  // Trashes the key!
  ~AESKey();

  //------------------------------------------------------------------------
  // Create a new key from random data
  // Must seed PRNG first
  void create();

  //------------------------------------------------------------------------
  // Read from data block
  void read(const unsigned char *data);

  //------------------------------------------------------------------------
  // Write to data block
  void write(unsigned char *data) const;

  //------------------------------------------------------------------------
  // Read from stream - reads hex characters
  void read(istream& sin);

  //------------------------------------------------------------------------
  // Read from stream as binary
  void read_binary(istream& sin);

  //------------------------------------------------------------------------
  // Write to stream - writes hex characters
  void write(ostream& sout) const;

  //------------------------------------------------------------------------
  // Write to stream as binary
  void write_binary(ostream& sout) const;

  //------------------------------------------------------------------------
  // Read from string - reads hex characters
  void read(const string& text);

  //------------------------------------------------------------------------
  // Set from passphrase
  void set_from_passphrase(const string& text);

  //------------------------------------------------------------------------
  // Set from integer - big-endian, zero padded
  void set_from_int(uint64_t n);

  //------------------------------------------------------------------------
  // Read from string as base64
  // Returns whether successful
  bool set_from_base64(const string& s);

  //------------------------------------------------------------------------
  // Convert to string (hex characters)
  string str() const;

  //------------------------------------------------------------------------
  // Convert to a base64 string
  string str_base64() const;

#if defined(__OBTOOLS_CHAN_H)
  // Provide channel versions without forcing use of ot-chan.h

  //------------------------------------------------------------------------
  // Read from channel (8 binary bytes)
  void read(Channel::Reader& reader) throw (Channel::Error);

  //------------------------------------------------------------------------
  // Write to channel (8 binary bytes)
  void write(Channel::Writer& writer) const throw (Channel::Error);
#endif
};

//------------------------------------------------------------------------
// >> operator to read key from istream
istream& operator>>(istream& s, AESKey& k);

//------------------------------------------------------------------------
// << operator to write key to ostream
ostream& operator<<(ostream& s, const AESKey& k);

//==========================================================================
// AES crypto object
// Uses ECB (no IV), CBC (with IV) or CTR (set on object) according to
// whether IV set
class AES
{
public:
  // Key
  AESKey key;

  // IV
  AESKey iv;  // Remains invalid if not used

  // Use CTR mode
  bool ctr;

  //------------------------------------------------------------------------
  // Default constructor
  AES(): iv(AESKey::BITS_128, false), ctr(false) {}

#undef set_key // Annoyingly defined by des headers
  //------------------------------------------------------------------------
  // Set key
  void set_key(const AESKey& _key) { key = _key; }

  //------------------------------------------------------------------------
  // Set an IV
  void set_iv(const AESKey& _iv) { iv = _iv; }

  //------------------------------------------------------------------------
  // Set key
  void set_ctr(bool _ctr) { ctr = _ctr; }

  //------------------------------------------------------------------------
  // Get current IV (in case it needs to be snapshotted for (e.g.) test
  AESKey& get_iv() { return iv; }

  //------------------------------------------------------------------------
  // Encrypt/decrypt a block in place
  // If block is not padded to 8 bytes, the remainder (up to 7) bytes
  // WILL NOT BE ENCRYPTED
  // Encrypts if 'encryption' is set (default), otherwise decrypts
  // IV is modified if set
  // Returns whether successful (keys set up correctly)
  bool encrypt(unsigned char *data, int length, bool encryption = true);

  //------------------------------------------------------------------------
  // Decrypt a block in place - shorthand for above
  bool decrypt(unsigned char *data, int length)
  { return encrypt(data, length, false); }

  //------------------------------------------------------------------------
  // Sugared versions with binary strings and PKCS5 padding
  bool encrypt(const string& plaintext, string& ciphertext_p);

  //------------------------------------------------------------------------
  // Sugared version of decrypt with binary strings and PKCS5 unpadding
  bool decrypt(const string& ciphertext, string& plaintext_p);
};

//==========================================================================
// DES key (des-key.cc)
// 8-byte key, also used for IV 
class DESKey
{
private:
  void load();

public:
  DES_cblock key;            // Original 8-byte key
  DES_key_schedule schedule; // Expanded key for optimised processing
  bool is_key;                    // True if a key, false if an IV
  bool valid;                     // Whether key/schedule is valid
  
  //------------------------------------------------------------------------
  // Default constructor
  DESKey(bool _is_key = true): is_key(_is_key), valid(false) {
    memset(key,0,sizeof(key));
  }

  //------------------------------------------------------------------------
  // Copy constructor - just copies base key and rebuilds schedule
  DESKey(const DESKey& k);

  //------------------------------------------------------------------------
  // Assignment operator - ditto
  DESKey& operator=(const DESKey& k);

  //------------------------------------------------------------------------
  // Constructor from data block - requires 8 bytes of data
  DESKey(const unsigned char *data):is_key(true) { read(data); }

  //------------------------------------------------------------------------
  // Constructor from string - requires 16 hex characters
  DESKey(const string& text):is_key(true) { read(text); }

  //------------------------------------------------------------------------
  // Create a new key from random data
  // Must seed PRNG first 
  void create();

  //------------------------------------------------------------------------
  // Read from data block - requires 8 bytes of data 
  void read(const unsigned char *data);

  //------------------------------------------------------------------------
  // Write to data block - writes 8 bytes of data 
  void write(unsigned char *data) const;

  //------------------------------------------------------------------------
  // Read from stream - reads 16 hex characters
  void read(istream& sin);

  //------------------------------------------------------------------------
  // Write to stream - writes 16 hex characters
  void write(ostream& sout) const;

  //------------------------------------------------------------------------
  // Read from string - reads 16 hex characters
  void read(const string& text);

  //------------------------------------------------------------------------
  // Convert to string
  string str() const;

#if defined(__OBTOOLS_CHAN_H)
  // Provide channel versions without forcing use of ot-chan.h
  
  //------------------------------------------------------------------------
  // Read from channel (8 binary bytes)
  void read(Channel::Reader& reader) throw (Channel::Error);

  //------------------------------------------------------------------------
  // Write to channel (8 binary bytes)
  void write(Channel::Writer& writer) const throw (Channel::Error);
#endif
};

//------------------------------------------------------------------------
// >> operator to read key from istream
istream& operator>>(istream& s, DESKey& k);

//------------------------------------------------------------------------
// << operator to write key to ostream
ostream& operator<<(ostream& s, const DESKey& k);

//==========================================================================
// DES crypto object
// Uses ECB (1 key, no IV) and CBC (1-3 keys, with IV) according to
// number of keys and whether IV set 
class DES
{
public:
  static const int MAX_KEYS = 3;

  // Keys: 1-3
  int nkeys;
  DESKey keys[MAX_KEYS];

  // IV
  DESKey iv;  // Remains invalid if not used

  //------------------------------------------------------------------------
  // Default constructor
  DES(): nkeys(0), iv(false) {}

  //------------------------------------------------------------------------
  // Add a key
  void add_key(const DESKey& k) { if (nkeys<MAX_KEYS) keys[nkeys++] = k; }

  //------------------------------------------------------------------------
  // Set an IV
  void set_iv(const DESKey& _iv) { iv = _iv; }

  //------------------------------------------------------------------------
  // Get current IV (in case it needs to be snapshotted for (e.g.) test
  DESKey& get_iv() { return iv; }

  //------------------------------------------------------------------------
  // Encrypt/decrypt a block in place
  // If block is not padded to 8 bytes, the remainder (up to 7) bytes 
  // WILL NOT BE ENCRYPTED
  // Encrypts if 'encryption' is set (default), otherwise decrypts
  // IV is modified if set
  // Returns whether successful (keys set up correctly)
  bool encrypt(unsigned char *data, int length, bool encryption = true);

  //------------------------------------------------------------------------
  // Decrypt a block in place - shorthand for above
  bool decrypt(unsigned char *data, int length)
  { return encrypt(data, length, false); }
};

//==========================================================================
// RSA key (rsa-key.cc)
// N-byte private or public key
class RSAKey
{
private:
  // Copy and assignment are banned!
  RSAKey(const RSAKey&) {}
  RSAKey& operator=(const RSAKey&) { return *this; }

public:
  RSA *rsa;

  bool is_private; // Private or public?
  bool valid;      // Whether key is valid
  
  //------------------------------------------------------------------------
  // Default constructor
  RSAKey(bool _private = false):
    rsa(0), is_private(_private), valid(false) {}

  //------------------------------------------------------------------------
  // Constructor from string - PEM format with pass-phrase
  RSAKey(const string& text, bool _private, const string& pass_phrase):
    rsa(0), is_private(_private), valid(false) { read(text, pass_phrase); }

  // Stub for backwards linkage compatibility
  RSAKey(const string& text, bool _private):
    rsa(0), is_private(_private), valid(false) { read(text); }

  //------------------------------------------------------------------------
  // Set from string for already created ones, with pass-phrase
  void set(const string& text, bool _private, const string& pass_phrase)
  { is_private = _private; read(text, pass_phrase); }

  // Backwards compatibility stub
  void set(const string& text, bool _private)
  { set(text, _private, ""); }

  //------------------------------------------------------------------------
  // Create a new key from random data
  // Seed PRNG first!
  void create(int size=1024, int exponent=65537);

  //------------------------------------------------------------------------
  // Read from stream - reads PEM format, with pass phrase
  // If force_private is set, reads a private key PEM block even if a
  // public key is wanted (use for testing both ends with a single key)
  void read(istream& sin, const string& pass_phrase, 
	    bool force_private = false);

  // Backwards compatibility stub
  void read(istream& sin, bool force_private = false);

  //------------------------------------------------------------------------
  // Write to stream - writes PEM format, with pass phrase
  // If force_public is set, writes a public key PEM block even if
  // a private key is held - use to generate new public/private pairs 
  void write(ostream& sout, const string& pass_phrase, 
	     bool force_public = false) const;

  // Backwards compatibility stub
  void write(ostream& sout, bool force_public = false) const;

  //------------------------------------------------------------------------
  // Read from string - reads PEM format, with pass phrase
  // If force_private is set, reads a private key even if public (see above)
  void read(const string& text, const string& pass_phrase,
	    bool force_private = false);

  //------------------------------------------------------------------------
  // Read from DER
  // If force_private is set, reads a private key even if public (see above)
  void read_der(const unsigned char* der, int length,
                bool force_private = false);

  // Backwards compatibility stub
  void read(const string& text, bool force_private = false);
    
  //------------------------------------------------------------------------
  // Convert to string (PEM format) with pass_phrase
  // force_public forces public key output from private key (see above)
  string str(const string& pass_phrase, bool force_public = false) const;

  // Backwards compatibility stub
  string str(bool force_public = false) const;

  //------------------------------------------------------------------------
  // Check for (in)validity
  bool operator!() const { return !valid; }

  //------------------------------------------------------------------------
  // Destructor
  ~RSAKey();
};

//------------------------------------------------------------------------
// >> operator to read key from istream (PEM format)
istream& operator>>(istream& s, RSAKey& k);

//------------------------------------------------------------------------
// << operator to write key to ostream (PEM format)
ostream& operator<<(ostream& s, const RSAKey& k);

//==========================================================================
// RSA crypto object
class RSA
{
public:
  RSAKey key;

  //------------------------------------------------------------------------
  // Default constructor
  RSA(bool is_private=false): key(is_private) {}

  //------------------------------------------------------------------------
  // Get ciphertext size
  int cipher_size();

  // Backwards compatibility for mis-spelling
  int cypher_size() { return cipher_size(); }

  //------------------------------------------------------------------------
  // Get maximum plaintext size
  int max_plaintext();

  //------------------------------------------------------------------------
  // Encrypt a block 
  // Length may be up to max_plaintext() bytes
  // Returns whether successful (key set up correctly)
  // 'to' must be writable to cipher_size() bytes
  bool encrypt(const unsigned char *from, int length, unsigned char *to);

  //------------------------------------------------------------------------
  // Decrypt a block 
  // Returns decrypted length of block
  // Assumes 'from' data is always cypher_size() bytes
  // 'to' must be writable to cipher_size() bytes 
  // (unless you _really_ understand padding!)
  int decrypt(const unsigned char *from, unsigned char *to);
};

//==========================================================================
// PKCS5 padding support 
class PKCS5
{
 public:
  //------------------------------------------------------------------------
  // Pad a block of data to given length multiple
  // Returns copied and padded malloc'ed data block, and modifies length
  // to length of padded block
  static unsigned char *pad(const unsigned char *data, int& length, 
			    int multiple);

  //------------------------------------------------------------------------
  // Pad a block of data to given length multiple, in place
  // Data must be at least multiple bytes longer than length
  // Returns length of padded data
  static int pad_in_place(unsigned char *data, int length, int multiple);

  //------------------------------------------------------------------------
  // Unpad a block of data 
  // Returns original length of block - data is not copied or modified
  static int original_length(const unsigned char *data, int length);
};

//==========================================================================
// SHA1 digest/hash support
// Either use as an object for repeated partial blocks, or use the static
// digest() to do an entire block
class SHA1
{
 private:
  SHA_CTX sha_ctx;
  bool finished;

  static string hex20(unsigned char *b);

 public:
  static const int DIGEST_LENGTH = 20;

  //------------------------------------------------------------------------
  // Constructor
  SHA1();

  //------------------------------------------------------------------------
  // Update digest with a block of data
  void update(const void *data, size_t length);

  //------------------------------------------------------------------------
  // Get result - writes DIGEST_LENGTH bytes to result
  void get_result(unsigned char *result);

  //------------------------------------------------------------------------
  // Get result as a hex string
  string get_result();

  //------------------------------------------------------------------------
  // Destructor
  ~SHA1();

  //------------------------------------------------------------------------
  // Static: Get hash of block of data.  Writes DIGEST_LENGTH bytes to result
  static void digest(const void *data, size_t length,
		     unsigned char *result);

  //------------------------------------------------------------------------
  // Ditto, but returning hex string
  static string digest(const void *data, size_t length);

  //------------------------------------------------------------------------
  // C++-friendly version of the above
  string digest(const string& text)
  { return digest(text.data(), text.length()); }
};

//==========================================================================
// HMAC hash support
// Either use as an object for repeated partial blocks, or use the static
// digest() to do an entire block
// Base class which can be inherited from for specific hash functions.
class HMAC
{
private:
  HMAC_CTX hmac_ctx;
  bool finished;
  const unsigned int digest_length;

public:
  //------------------------------------------------------------------------
  // Constructor
  HMAC(const void *key, int key_len, const EVP_MD *md,
       unsigned int digest_length);

  //------------------------------------------------------------------------
  // Return length of digest
  unsigned int get_digest_length() const { return digest_length; }

  //------------------------------------------------------------------------
  // Update digest with a block of data
  void update(const unsigned char *data, size_t length);

  //------------------------------------------------------------------------
  // Get result - writes get_digest_length() bytes to result
  void get_result(unsigned char *result);

  //------------------------------------------------------------------------
  // Get hash of block of data.  Writes get_digest_length() bytes to result
  void digest(const unsigned char *data, size_t length,
              unsigned char *result);

  //------------------------------------------------------------------------
  // Virtual Destructor
  virtual ~HMAC();
};

//==========================================================================
// HMAC SHA 256
class HMACSHA256: public HMAC
{
public:
  //------------------------------------------------------------------------
  // Constructor
  HMACSHA256(const void *key, int key_len):
    HMAC(key, key_len, EVP_sha256(), 32) {}

  //------------------------------------------------------------------------
  // Virtual Destructor
  virtual ~HMACSHA256() {}
};

//==========================================================================
// X509 support
class Certificate
{
private:
  X509 *x509;

public:
  //------------------------------------------------------------------------
  // Default constructor
  Certificate(): x509(0) {}

  //------------------------------------------------------------------------
  // Constructor from existing X509 structure
  Certificate(X509 *_x509): x509(_x509) {}

  //------------------------------------------------------------------------
  // Constructor from PEM or DER format string
  Certificate(const string& text): x509(0) { read(text); }

  //------------------------------------------------------------------------
  // Read from stream - reads PEM or DER format, depending whether string
  // contains binary characters
  void read(istream& sin);

  //------------------------------------------------------------------------
  // Write to stream - writes PEM format
  void write(ostream& sout) const;

  //------------------------------------------------------------------------
  // Read from string - reads PEM format
  void read(const string& text);

  //------------------------------------------------------------------------
  // Convert to string (PEM format) 
  string str() const;

  //------------------------------------------------------------------------
  // Check if it's valid
  bool operator!() const { return !x509; }

  //------------------------------------------------------------------------
  // Get raw X509 structure (e.g. for SSL).  Can be 0.
  X509 *get_x509() const { return x509; }

  //------------------------------------------------------------------------
  // Get common name
  string get_cn() const;

  //------------------------------------------------------------------------
  // Get RSA public key into the given RSAKey
  // Returns whether successful, also sets key validity
  bool get_public_key(RSAKey& key) const;

  //------------------------------------------------------------------------
  // Destructor
  ~Certificate();
};

//------------------------------------------------------------------------
// >> operator to read cert from istream
istream& operator>>(istream& s, Certificate& c);

//------------------------------------------------------------------------
// << operator to write certificate to ostream
ostream& operator<<(ostream& s, const Certificate& c);

//==========================================================================
// X509 Certificate Store
// Mutexed for multi-thread use
class CertificateStore
{
  X509_STORE *store;
  MT::Mutex mutex;      // Not clear that X509_STORE is re-entrant

 public:
  //------------------------------------------------------------------------
  // Constructor:
  // ca_file should refer to a PEM format containing a list of trusted CAs
  // ca_dir should refer to a directory containing certificate files with 
  // hashed names (see OpenSSL docs)
  CertificateStore(const string& ca_file="", const string& ca_dir="");

  //------------------------------------------------------------------------
  // Add a pre-loaded certificate
  // Note: certicate must remain alive during lifetime of store
  bool add(Certificate *cert);

  //------------------------------------------------------------------------
  // Add a CRL file and enable CRL checking in the store
  // If 'all' is set, the entire chain is checked against the CRL
  bool add_crl(const string& crl_file, bool all=false);

  //------------------------------------------------------------------------
  // Verify a certificate
  bool verify(const Certificate& cert);

  //------------------------------------------------------------------------
  // Destructor
  ~CertificateStore();
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_CRYPTO_H

















//==========================================================================
// ObTools::Crypto: ot-crypto.h
//
// Public definitions for ObTools::Crypto
// C++ wrappers for crypto functions, (currently) using OpenSSL
// 
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

// Include ot-chan.h first to enable Channel functions

#ifndef __OBTOOLS_CRYPTO_H
#define __OBTOOLS_CRYPTO_H

#include <string>
#include <map>

#include <openssl/des.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>

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
  DESKey(bool _is_key = true): is_key(_is_key), valid(false) {}

  //------------------------------------------------------------------------
  // Copy constructor - just copies base key and rebuilds schedule
  DESKey(const DESKey& k);

  //------------------------------------------------------------------------
  // Assignment operator - ditto
  DESKey& operator=(const DESKey& k);

  //------------------------------------------------------------------------
  // Constructor from data block - requires 8 bytes of data
  DESKey(const unsigned char *data) { read(data); }

  //------------------------------------------------------------------------
  // Constructor from string - requires 16 hex characters
  DESKey(const string& text) { read(text); }

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

  // Backwards compatibility stub
  void read(const string& text, bool force_private = false);
    
  //------------------------------------------------------------------------
  // Convert to string (PEM format) with pass_phrase
  // force_public forces public key output from private key (see above)
  string str(const string& pass_phrase, bool force_public = false) const;

  // Backwards compatibility stub
  string str(bool force_public = false) const;

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
  // Get cyphertext size
  int cypher_size();

  //------------------------------------------------------------------------
  // Get maximum plaintext size
  int max_plaintext();

  //------------------------------------------------------------------------
  // Encrypt a block 
  // Length may be up to max_plaintext() bytes
  // Returns whether successful (key set up correctly)
  // 'to' must be writable to cypher_size() bytes
  bool encrypt(const unsigned char *from, int length, unsigned char *to);

  //------------------------------------------------------------------------
  // Decrypt a block 
  // Returns decrypted length of block
  // Assumes 'from' data is always cypher_size() bytes
  // 'to' must be writable to cypher_size() bytes 
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
  // Unpad a block of data 
  // Returns original length of block - data is not copied or modified
  static int PKCS5::original_length(const unsigned char *data, int length);
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

};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_CRYPTO_H

















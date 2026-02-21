# CLAUDE.md - ObTools::Crypto Library

## Overview

`ObTools::Crypto` provides cryptographic operations via OpenSSL: AES/DES symmetric encryption, RSA asymmetric encryption, SHA1/SHA256 hashing, HMAC, X.509 certificates, PKCS7 padding, and elliptic curve key pairs. Lives under `namespace ObTools::Crypto`.

**Header:** `ot-crypto.h`
**Dependencies:** `ot-mt`, `ot-misc`, `ot-text`, `ot-gather`, `ext-crypto`
**Platforms:** posix, web

## Key Classes

| Class | Purpose |
|-------|---------|
| `Library` | OpenSSL library initialisation (construct once) |
| `AESKey` / `AES` | AES encryption (128/192/256-bit, CBC/CTR) |
| `DESKey` / `DES` | DES/3DES encryption |
| `RSAKey` / `RSA` | RSA asymmetric encryption |
| `SHA1` / `SHA256` | Secure hash algorithms |
| `HMAC` / `HMACSHA1` / `HMACSHA256` | Hash-based message authentication |
| `Certificate` / `CertificateStore` | X.509 certificate handling |
| `KeyPair` | Elliptic curve key pairs (secp256k1, ed25519) |
| `PKCS7` | PKCS7 padding |
| `Hash` namespace | Standalone hash functions (ripemd160, sha256, sha512, sha3_256, keccak256) |

## AESKey

```cpp
enum Size { BITS_128=128, BITS_192=192, BITS_256=256 };

AESKey(Size size=BITS_128, bool is_key=true);
AESKey(const unsigned char *data, Size size=BITS_128);
AESKey(const string& hex_text, Size size=BITS_128);

void create();                              // generate random key
void set_from_passphrase(const string& text);
void set_from_int(uint64_t n);
bool set_from_base64(const string& s);
string str() const;                         // hex
string str_base64() const;
void read(const string& hex);  void write(ostream&) const;
```

## AES

```cpp
AES();
void set_key(const AESKey& key);
void set_iv(const AESKey& iv);
void set_ctr(bool ctr);                     // use CTR mode

// Block operations
bool encrypt(unsigned char *data, int length, bool encryption=true);
bool decrypt(unsigned char *data, int length);

// String operations (with PKCS7 padding)
bool encrypt(const string& plaintext, string& ciphertext);
bool decrypt(const string& ciphertext, string& plaintext);
bool encrypt(vector<unsigned char>& data);
bool decrypt(vector<unsigned char>& data);

// Gather buffer (CTR mode)
bool encrypt(Gather::Buffer& buffer);
bool decrypt(Gather::Buffer& buffer);
```

## RSAKey

```cpp
RSAKey(bool is_private=false);
RSAKey(const string& pem, bool is_private, const string& passphrase="");
void create(int size=1024, int exponent=65537);
void read(const string& pem, const string& passphrase="", bool force_private=false);
string str(const string& passphrase="", bool force_public=false) const;
string der(bool force_public=false) const;
bool operator!() const;
```

## RSA

```cpp
RSA(bool is_private=false);
int cipher_size();  int max_plaintext();
bool encrypt(const unsigned char *from, int length, unsigned char *to);
int decrypt(const unsigned char *from, unsigned char *to);
```

## SHA1 / SHA256

Both share the same API pattern:

```cpp
SHA1();  SHA256();
void update(const void *data, size_t length);
string get_result();     // binary digest

static string digest(const string& text);       // one-shot binary
static string digest_hex(const string& text);   // one-shot hex
static void digest(const void *data, size_t len, unsigned char *result);
// SHA256 additionally:
static vector<byte> digest(const vector<byte>& data);
```

## HMAC / HMACSHA1 / HMACSHA256

```cpp
HMACSHA1(const string& key);   HMACSHA256(const string& key);
string digest(const string& text);       // binary
string digest_hex(const string& text);   // hex

// Static one-shot
static string sign(const string& key, const string& data);
static string sign_hex(const string& key, const string& data);
```

## Certificate / CertificateStore

```cpp
Certificate();  Certificate(const string& pem);
string get_cn() const;
bool get_public_key(RSAKey& key) const;
string str() const;  bool operator!() const;

CertificateStore(const string& ca_file="", const string& ca_dir="");
bool add(Certificate *cert);
bool add_crl(const string& crl_file, bool all=false);
bool verify(const Certificate& cert);
```

## KeyPair (abstract)

```cpp
static unique_ptr<KeyPair> create_secp256k1(const vector<byte>& key);
static unique_ptr<KeyPair> create_secp256k1_pub(const vector<byte>& key);
static unique_ptr<KeyPair> create_ed25519(const vector<byte>& key);
static unique_ptr<KeyPair> create_ed25519_pub(const vector<byte>& key);

bool is_valid() const;
virtual vector<byte> sign(const vector<byte>& message) const = 0;
virtual bool verify(const vector<byte>& message, const vector<byte>& signature) const = 0;
```

## Hash Namespace

```cpp
vector<byte> Hash::ripemd160(const vector<byte>& data);
vector<byte> Hash::sha256(const vector<byte>& data);
vector<byte> Hash::sha512(const vector<byte>& data);
vector<byte> Hash::sha3_256(const vector<byte>& data);
vector<byte> Hash::keccak256(const vector<byte>& data);  // OpenSSL 3.2+
```

## PKCS7

```cpp
static unsigned char *PKCS7::pad(const unsigned char *data, int& length, int multiple);
static void PKCS7::pad(vector<unsigned char>& data, int multiple);
static int PKCS7::original_length(const unsigned char *data, int length);
static void PKCS7::unpad(vector<unsigned char>& data);
```

## File Layout

```
ot-crypto.h           - Public header
library.cc            - OpenSSL library init
aes.cc / aes-key.cc   - AES implementation
des.cc / des-key.cc   - DES implementation
rsa.cc / rsa-key.cc   - RSA implementation
sha1.cc / sha256.cc   - Hash implementations
hmac.cc               - HMAC implementation
hash.cc               - Hash namespace functions
x509.cc               - Certificate handling
store.cc              - CertificateStore
pkcs7.cc              - PKCS7 padding
keypair-ec.cc          - EC key pairs
keypair-ed.cc          - Ed25519 key pairs
test-aes.cc            - AES tests (gtest)
test-aes-string.cc     - AES string tests
test-hash.cc           - Hash tests
test-hmac.cc           - HMAC tests
test-keypair-ec.cc     - EC key pair tests
test-keypair-ed.cc     - Ed25519 tests
test-pkcs7-vector.cc   - PKCS7 tests
```

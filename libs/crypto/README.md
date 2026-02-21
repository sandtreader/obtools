# ObTools::Crypto

A cryptographic operations library for C++17 built on OpenSSL. Provides AES/DES symmetric encryption, RSA asymmetric encryption, SHA1/SHA256 hashing, HMAC authentication, X.509 certificates, elliptic curve key pairs, and PKCS7 padding.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Features

- **AES encryption**: 128/192/256-bit keys, CBC and CTR modes, PKCS7 padding
- **DES/3DES encryption**: single and triple-key DES
- **RSA encryption**: key generation, PEM/DER import/export, encrypt/decrypt
- **Hashing**: SHA1, SHA256, RIPEMD160, SHA512, SHA3-256, Keccak-256
- **HMAC**: HMAC-SHA1 and HMAC-SHA256 message authentication
- **X.509**: certificate parsing, verification, certificate stores with CRL
- **Key pairs**: secp256k1 and ed25519 elliptic curve keys with sign/verify
- **PKCS7**: block padding and unpadding

## Dependencies

- `ot-mt`, `ot-misc`, `ot-text`, `ot-gather` - Core ObTools libraries
- `ext-crypto` - OpenSSL libcrypto

## Quick Start

```cpp
#include "ot-crypto.h"
using namespace ObTools;

// Initialise OpenSSL (once, at program start)
Crypto::Library lib;
```

### AES Encryption

```cpp
// Generate random 256-bit key and IV
Crypto::AESKey key(Crypto::AESKey::BITS_256);
key.create();
Crypto::AESKey iv(Crypto::AESKey::BITS_128, false);
iv.create();

Crypto::AES aes;
aes.set_key(key);
aes.set_iv(iv);

// Encrypt/decrypt strings (PKCS7 padded)
string ciphertext, plaintext;
aes.encrypt("Hello, World!", ciphertext);
aes.decrypt(ciphertext, plaintext);  // "Hello, World!"

// Key from passphrase
Crypto::AESKey passkey(Crypto::AESKey::BITS_256);
passkey.set_from_passphrase("my secret");

// Key serialisation
string hex = key.str();
string b64 = key.str_base64();
```

### CTR Mode

```cpp
Crypto::AES aes;
aes.set_key(key);
aes.set_iv(iv);
aes.set_ctr(true);

// Encrypt/decrypt binary vectors
vector<unsigned char> data = /* ... */;
aes.encrypt(data);
aes.decrypt(data);
```

### RSA Encryption

```cpp
// Generate key pair
Crypto::RSAKey key(true);  // private
key.create(2048);

// Export
string private_pem = key.str();
string public_pem = key.str("", true);  // force_public=true
string der_bytes = key.der(true);

// Import
Crypto::RSAKey imported(pem_string, true, "passphrase");

// Encrypt/decrypt
Crypto::RSA rsa(true);
rsa.key = /* loaded key */;

unsigned char ciphertext[256];
rsa.encrypt(plaintext_data, plaintext_len, ciphertext);

unsigned char decrypted[256];
int len = rsa.decrypt(ciphertext, decrypted);
```

### SHA256 Hashing

```cpp
// One-shot
string hash = Crypto::SHA256::digest("Hello");          // binary
string hex = Crypto::SHA256::digest_hex("Hello");        // hex string
vector<byte> vhash = Crypto::SHA256::digest(byte_vec);   // binary vector

// Incremental
Crypto::SHA256 sha;
sha.update(data1, len1);
sha.update(data2, len2);
string result = sha.get_result();
```

### HMAC Signing

```cpp
// One-shot
string sig = Crypto::HMACSHA256::sign("secret-key", "message");
string hex = Crypto::HMACSHA256::sign_hex("secret-key", "message");

// Incremental
Crypto::HMACSHA256 hmac("secret-key");
hmac.update(data, len);
string result = hmac.digest("additional data");
```

### X.509 Certificates

```cpp
// Load certificate
Crypto::Certificate cert(pem_string);
if (!cert) { /* invalid */ }
string cn = cert.get_cn();

// Extract public key
Crypto::RSAKey pub_key;
cert.get_public_key(pub_key);

// Verify with certificate store
Crypto::CertificateStore store("/etc/ssl/certs/ca-certificates.crt");
if (store.verify(cert)) { /* trusted */ }

// Add CRL
store.add_crl("/path/to/crl.pem");
```

### Elliptic Curve Key Pairs

```cpp
// secp256k1
auto kp = Crypto::KeyPair::create_secp256k1(private_key_bytes);
vector<byte> sig = kp->sign(message_bytes);

auto pub = Crypto::KeyPair::create_secp256k1_pub(public_key_bytes);
bool ok = pub->verify(message_bytes, sig);

// ed25519
auto ed = Crypto::KeyPair::create_ed25519(private_key_bytes);
vector<byte> sig2 = ed->sign(message_bytes);
```

### Hash Functions

```cpp
vector<byte> data = /* ... */;
auto h1 = Crypto::Hash::sha256(data);
auto h2 = Crypto::Hash::sha512(data);
auto h3 = Crypto::Hash::ripemd160(data);
auto h4 = Crypto::Hash::sha3_256(data);
auto h5 = Crypto::Hash::keccak256(data);  // OpenSSL 3.2+
```

### PKCS7 Padding

```cpp
// Pad vector to AES block size (16)
vector<unsigned char> data = /* ... */;
Crypto::PKCS7::pad(data, 16);

// Unpad
Crypto::PKCS7::unpad(data);

// Get original length from padded data
int orig = Crypto::PKCS7::original_length(data.data(), data.size());
```

## Build

```
NAME      = ot-crypto
TYPE      = lib
DEPENDS   = ot-mt ot-misc ot-text ot-gather ext-crypto
PLATFORMS = posix web
```

## License

Copyright (c) 2003 Paul Clark. MIT License.

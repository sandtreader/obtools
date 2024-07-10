//==========================================================================
// ObTools::Crypto: keypair.cc
//
// Key Pair (Edwards curve) implementation
//
// Copyright (c) 2024 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================


#include "ot-crypto.h"
#if OPENSSL_VERSION_MAJOR >= 3
#include <openssl/param_build.h>
#include <openssl/err.h>

namespace ObTools { namespace Crypto {

class KeyPairEd: public KeyPair
{
public:
  bool verify(const vector<byte>& message, const vector<byte>& signature) const;
  vector<byte> sign(const vector<byte>& message) const;
  KeyPairEd(decltype(evp_key)& evp_key): KeyPair{evp_key} {}
};

//--------------------------------------------------------------------------
// General Edwards curve constructors
unique_ptr<KeyPair> KeyPair::create_ed(int type, const vector<byte>& key)
{
  auto evp_key = unique_ptr<EVP_PKEY, void (*)(EVP_PKEY *)>{
      EVP_PKEY_new_raw_private_key(
        static_cast<int>(type), nullptr,
        reinterpret_cast<const unsigned char *>(key.data()), key.size()),
      EVP_PKEY_free};
  return unique_ptr<KeyPair>{new KeyPairEd{evp_key}};
}

unique_ptr<KeyPair> KeyPair::create_ed_pub(int type, const vector<byte>& key)
{
  auto offset = (key.size() == 34) ? 2 : 0; // Skip over CBOR encapsulation
  auto evp_key = unique_ptr<EVP_PKEY, void (*)(EVP_PKEY *)>{
      EVP_PKEY_new_raw_public_key(
        static_cast<int>(type), nullptr,
        reinterpret_cast<const unsigned char *>(key.data())+offset,
        key.size()-offset),
      EVP_PKEY_free};
  return unique_ptr<KeyPair>{new KeyPairEd{evp_key}};
}

//--------------------------------------------------------------------------
// Constructor wrappers
unique_ptr<KeyPair> KeyPair::create_ed25519(const vector<byte>& key)
{
  return create_ed(NID_ED25519, key);
}
unique_ptr<KeyPair> KeyPair::create_ed25519_pub(const vector<byte>& key)
{
  return create_ed_pub(NID_ED25519, key);
}

//--------------------------------------------------------------------------
// Verify
bool KeyPairEd::verify(const vector<byte>& message,
                     const vector<byte>& signature) const
{
  auto mdctx = unique_ptr<EVP_MD_CTX, void (*)(EVP_MD_CTX *)>{
      EVP_MD_CTX_new(), EVP_MD_CTX_free};
  if (EVP_DigestVerifyInit(
        mdctx.get(), nullptr, nullptr, nullptr, evp_key.get()) != 1)
    throw runtime_error("Failed to initiliase EVP digest verifier");
  return EVP_DigestVerify(mdctx.get(),
      reinterpret_cast<const unsigned char *>(signature.data()),
      signature.size(),
      reinterpret_cast<const unsigned char *>(message.data()),
      message.size()) == 1;
}

//--------------------------------------------------------------------------
// Sign
vector<byte> KeyPairEd::sign(const vector<byte>& message) const
{
  auto mdctx = unique_ptr<EVP_MD_CTX, void (*)(EVP_MD_CTX *)>{
      EVP_MD_CTX_new(), EVP_MD_CTX_free};
  if (EVP_DigestSignInit(
        mdctx.get(), nullptr, nullptr, nullptr, evp_key.get()) != 1)
    throw runtime_error("Failed to initiliase EVP digest signer");
  auto signature = vector<byte>(EVP_PKEY_size(evp_key.get()));
  auto signature_len = signature.size();
  if (EVP_DigestSign(mdctx.get(),
      reinterpret_cast<unsigned char *>(signature.data()),
      &signature_len,
      reinterpret_cast<const unsigned char *>(message.data()),
      message.size()) != 1)
    throw runtime_error("Failed to sign");
  signature.resize(signature_len);
  return signature;
}

}} // namespaces

#endif // OpenSSL >= 3

//==========================================================================
// ObTools::Crypto: evp.cc
//
// EVP implementation
//
// Copyright (c) 2024 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-crypto.h"
#if OPENSSL_VERSION_MAJOR >= 3

namespace ObTools { namespace Crypto { namespace EVP {

//--------------------------------------------------------------------------
// Verify signature
bool verify(const EVPKey& key, const vector<byte>& message,
            const vector<byte>& signature)
{
  if (key.type == EVPKey::Type::X25519 || key.type == EVPKey::Type::Ed25519 ||
      key.type == EVPKey::Type::X448 || key.type == EVPKey::Type::Ed448)
  {
    // EVP_PKEY_verify_init does not seem to support these types
    auto mdctx = unique_ptr<EVP_MD_CTX, void (*)(EVP_MD_CTX *)>{
        EVP_MD_CTX_new(), EVP_MD_CTX_free};
    if (EVP_DigestVerifyInit(
          mdctx.get(), nullptr, nullptr, nullptr, key.evp_key.get()) != 1)
      throw runtime_error("Failed to initiliase EVP digest verifier");
    return EVP_DigestVerify(mdctx.get(),
        reinterpret_cast<const unsigned char *>(signature.data()),
        signature.size(),
        reinterpret_cast<const unsigned char *>(message.data()),
        message.size()) == 1;
  }
  auto ctx = unique_ptr<EVP_PKEY_CTX, void (*)(EVP_PKEY_CTX *)>{
      EVP_PKEY_CTX_new(key.evp_key.get(), nullptr), EVP_PKEY_CTX_free};
  if (EVP_PKEY_verify_init(ctx.get()) != 1)
    throw runtime_error("Failed to initiliase EVP verifier");
  return EVP_PKEY_verify(ctx.get(),
      reinterpret_cast<const unsigned char *>(signature.data()),
      signature.size(),
      reinterpret_cast<const unsigned char *>(message.data()),
      message.size()) == 1;
}

//--------------------------------------------------------------------------
// Hash data
vector<byte> hash(HashType type, const vector<byte>& data)
{
  auto hash = vector<byte>(EVP_MAX_MD_SIZE);
  auto hash_size = unsigned{0};
  const auto hash_func = EVP_MD_fetch(nullptr, type, nullptr);
  EVP_Digest(
      reinterpret_cast<const void *>(data.data()), data.size(),
      reinterpret_cast<unsigned char *>(hash.data()), &hash_size,
      hash_func, nullptr);
  hash.resize(hash_size);
  return hash;
}

}}} // namespaces

#endif // OpenSSL >= 3

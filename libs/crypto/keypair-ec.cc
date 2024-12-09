//==========================================================================
// ObTools::Crypto: keypair-ec.cc
//
// Key Pair (elliptic curve) implementation
//
// Copyright (c) 2024 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================


#include "ot-crypto.h"
#if OPENSSL_VERSION_MAJOR >= 3
#include <openssl/param_build.h>
#include <openssl/err.h>

namespace ObTools { namespace Crypto {

const auto secp256k1 = string("secp256k1");

class KeyPairEC: public KeyPair
{
public:
  bool verify(const vector<byte>& message, const vector<byte>& signature) const;
  vector<byte> sign(const vector<byte>& message) const;
  KeyPairEC(decltype(evp_key)& evp_key): KeyPair{evp_key} {}
};

//--------------------------------------------------------------------------
// General elliptive curve constructor
unique_ptr<KeyPair> KeyPair::create_ec(const string& curve,
                                       const vector<byte>& key)
{
  auto param_bld = unique_ptr<OSSL_PARAM_BLD, void (*)(OSSL_PARAM_BLD *)>{
      OSSL_PARAM_BLD_new(), OSSL_PARAM_BLD_free};
  if (!param_bld)
    throw runtime_error("Failed to create param builder");
  OSSL_PARAM_BLD_push_utf8_string(param_bld.get(), "group", curve.c_str(), 0);
  auto priv = unique_ptr<BIGNUM, void (*)(BIGNUM *)>{
      BN_bin2bn(reinterpret_cast<const unsigned char *>(key.data()),
                key.size(), nullptr),
      BN_free};
  OSSL_PARAM_BLD_push_BN(param_bld.get(), "priv", priv.get());
  auto params = unique_ptr<OSSL_PARAM, void (*)(OSSL_PARAM *)>{
      OSSL_PARAM_BLD_to_param(param_bld.get()), OSSL_PARAM_free};
  auto pkey_ctx = unique_ptr<EVP_PKEY_CTX, void (*)(EVP_PKEY_CTX *)>{
      EVP_PKEY_CTX_new_from_name(nullptr, "EC", nullptr), EVP_PKEY_CTX_free};
  if (!pkey_ctx)
    throw runtime_error("Failed to create key context");
  if (EVP_PKEY_fromdata_init(pkey_ctx.get()) <= 0)
    throw runtime_error("Failed to init fromdata");
  auto pkey = unique_ptr<EVP_PKEY, void (*)(EVP_PKEY *)>{
      EVP_PKEY_new(), EVP_PKEY_free};
  auto pkey_p = pkey.get();
  if (EVP_PKEY_fromdata(
        pkey_ctx.get(), &pkey_p, EVP_PKEY_KEYPAIR, params.get()) <= 0)
    throw runtime_error("Failed to create key from data");
  return unique_ptr<KeyPair>{new KeyPairEC{pkey}};
}

unique_ptr<KeyPair> KeyPair::create_ec_pub(const string& curve,
                                           const vector<byte>& key)
{
  auto param_bld = unique_ptr<OSSL_PARAM_BLD, void (*)(OSSL_PARAM_BLD *)>{
      OSSL_PARAM_BLD_new(), OSSL_PARAM_BLD_free};
  if (!param_bld)
    throw runtime_error("Failed to create param builder");
  OSSL_PARAM_BLD_push_utf8_string(param_bld.get(), "group", curve.c_str(), 0);
  OSSL_PARAM_BLD_push_octet_string(
      param_bld.get(), "pub", key.data(), key.size());
  auto params = unique_ptr<OSSL_PARAM, void (*)(OSSL_PARAM *)>{
      OSSL_PARAM_BLD_to_param(param_bld.get()), OSSL_PARAM_free};
  auto pkey_ctx = unique_ptr<EVP_PKEY_CTX, void (*)(EVP_PKEY_CTX *)>{
      EVP_PKEY_CTX_new_from_name(nullptr, "EC", nullptr), EVP_PKEY_CTX_free};
  if (!pkey_ctx)
    throw runtime_error("Failed to create key context");
  if (EVP_PKEY_fromdata_init(pkey_ctx.get()) <= 0)
    throw runtime_error("Failed to init fromdata");
  auto pkey = unique_ptr<EVP_PKEY, void (*)(EVP_PKEY *)>{
      EVP_PKEY_new(), EVP_PKEY_free};
  auto pkey_p = pkey.get();
  if (EVP_PKEY_fromdata(
        pkey_ctx.get(), &pkey_p, EVP_PKEY_PUBLIC_KEY, params.get()) <= 0)
    throw runtime_error("Failed to create key from data");
  return unique_ptr<KeyPair>{new KeyPairEC{pkey}};
}

//--------------------------------------------------------------------------
// Constructor wrappers
unique_ptr<KeyPair> KeyPair::create_secp256k1(const vector<byte>& key)
{
  return create_ec(secp256k1, key);
}
unique_ptr<KeyPair> KeyPair::create_secp256k1_pub(const vector<byte>& key)
{
  return create_ec_pub(secp256k1, key);
}

//--------------------------------------------------------------------------
// Verify
bool KeyPairEC::verify(const vector<byte>& message,
                       const vector<byte>& signature) const
{
  auto ctx = unique_ptr<EVP_PKEY_CTX, void (*)(EVP_PKEY_CTX *)>{
      EVP_PKEY_CTX_new(evp_key.get(), nullptr), EVP_PKEY_CTX_free};
  if (EVP_PKEY_verify_init(ctx.get()) != 1)
    throw runtime_error("Failed to initiliase EVP verifier");
  return EVP_PKEY_verify(ctx.get(),
      reinterpret_cast<const unsigned char *>(signature.data()),
      signature.size(),
      reinterpret_cast<const unsigned char *>(message.data()),
      message.size()) == 1;
}

//--------------------------------------------------------------------------
// Sign
vector<byte> KeyPairEC::sign(const vector<byte>& message) const
{
  auto ctx = unique_ptr<EVP_PKEY_CTX, void (*)(EVP_PKEY_CTX *)>{
      EVP_PKEY_CTX_new(evp_key.get(), nullptr), EVP_PKEY_CTX_free};
  if (EVP_PKEY_sign_init(ctx.get()) != 1)
    throw runtime_error("Failed to initiliase EVP signer");
  auto signature = vector<byte>(EVP_PKEY_size(evp_key.get()));
  auto signature_len = signature.size();
  if (EVP_PKEY_sign(ctx.get(),
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

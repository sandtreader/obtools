//==========================================================================
// ObTools::Crypto: evp-key.cc
//
// EVP Key implementation
//
// Copyright (c) 2024 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================


#include "ot-crypto.h"
#if OPENSSL_VERSION_MAJOR >= 3
#include <openssl/param_build.h>
#include <openssl/err.h>

namespace ObTools { namespace Crypto {

//--------------------------------------------------------------------------
// Initialise an EVP Key
unique_ptr<EVP_PKEY, void (*)(EVP_PKEY *)> EVPKey::init_key(
    Type type, const vector<byte>& key)
{
  if (type == Type::SECP256K1) {
    auto param_bld = unique_ptr<OSSL_PARAM_BLD, void (*)(OSSL_PARAM_BLD *)>{
        OSSL_PARAM_BLD_new(), OSSL_PARAM_BLD_free};
    if (!param_bld)
      throw runtime_error("Failed to create param builder");
    OSSL_PARAM_BLD_push_utf8_string(param_bld.get(), "group", "secp256k1", 0);
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
    return pkey;
  }
  return {EVP_PKEY_new_raw_public_key(
      static_cast<int>(type), nullptr,
      reinterpret_cast<const unsigned char *>(key.data()), key.size()),
      EVP_PKEY_free};
}

}} // namespaces

#endif // OpenSSL >= 3

//==========================================================================
// ObTools::Crypto: evp.cc
//
// EVP implementation
//
// Copyright (c) 2024 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-crypto.h"

namespace ObTools { namespace Crypto { namespace EVP {

//--------------------------------------------------------------------------
// Verify signature
bool verify(const EVPKey& key, const vector<byte>& message,
    const vector<byte>& signature)
{
  auto mdctx = unique_ptr<EVP_MD_CTX, void (*)(EVP_MD_CTX *)>{
      EVP_MD_CTX_new(), EVP_MD_CTX_free};
  // Note: 3rd arg should probably be set based on key type, but for Ed25519
  // it must be null
  if (EVP_DigestVerifyInit(
        mdctx.get(), nullptr, nullptr, nullptr, key.evp_key.get()) != 1)
    throw runtime_error("Failed to initiliase EVP digest verifier");
  return EVP_DigestVerify(mdctx.get(),
      reinterpret_cast<const unsigned char *>(signature.data()),
      signature.size(), reinterpret_cast<const unsigned char *>(message.data()),
      message.size()) == 1;
}

}}} // namespaces

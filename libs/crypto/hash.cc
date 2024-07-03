//==========================================================================
// ObTools::Crypto: hash.cc
//
// Hash implementations
//
// Copyright (c) 2024 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-crypto.h"
#if OPENSSL_VERSION_MAJOR >= 3

namespace ObTools { namespace Crypto { namespace Hash {

//--------------------------------------------------------------------------
// General hash function
vector<byte> hash(const string &type, const vector<byte>& data)
{
  auto hash = vector<byte>(EVP_MAX_MD_SIZE);
  auto hash_size = unsigned{0};
  const auto hash_func = EVP_MD_fetch(nullptr, type.c_str(), nullptr);
  EVP_Digest(
      reinterpret_cast<const void *>(data.data()), data.size(),
      reinterpret_cast<unsigned char *>(hash.data()), &hash_size,
      hash_func, nullptr);
  hash.resize(hash_size);
  return hash;
}

//--------------------------------------------------------------------------
// Wrapper functions
vector<byte> ripemd160(const vector<byte>& data)
{
  return hash("ripemd160", data);
}
vector<byte> sha512(const vector<byte>& data)
{
  return hash("SHA512", data);
}
#if OPENSSL_VERSION_MAJOR > 3 || \
    (OPENSSL_VERSION_MAJOR == 3 && OPENSSL_VERSION_MINOR >= 2)
vector<byte> keccak256(const vector<byte>& data)
{
  return hash("KECCAK-256", data);
}
#endif


}}} // namespaces

#endif // OpenSSL >= 3

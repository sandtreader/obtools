//==========================================================================
// ObTools::Merkle: hash-sha256.cc
//
// Hash function for SHA256
//
// Copyright (c) 2024 Paul Clark
//==========================================================================

#include "ot-merkle.h"
#include "ot-crypto.h"

using namespace std;
using namespace ObTools;
using namespace ObTools::Merkle::Hash;

SHA256::hash_t SHA256::hash_func(const Node<SHA256::hash_t, void>& left,
                                 const Node<SHA256::hash_t, void>& right)
{
  const auto left_hash = left.get_hash();
  const auto right_hash = right.get_hash();

  Crypto::SHA256 sha256;
  sha256.update(left_hash.data(), left_hash.size());
  sha256.update(right_hash.data(), right_hash.size());

  SHA256::hash_t result(Crypto::SHA256::DIGEST_LENGTH);
  sha256.get_result(result.data());
  return result;
}


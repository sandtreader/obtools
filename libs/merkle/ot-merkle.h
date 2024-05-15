//==========================================================================
// ObTools::Merkle: ot-merkle.h
//
// Public definitions for ObTools::Merkle
//
// Merkle tree tools
//
// Copyright (c) 2024 Paul Clark.
//==========================================================================

#ifndef __OBTOOLS_MERKLE_H
#define __OBTOOLS_MERKLE_H

#include <memory>
#include <vector>

namespace ObTools { namespace Merkle {

// Make our lives easier without polluting anyone else
using namespace std;

template <typename T>
using HashFunc = T (*)(const T&, const T&);

//==========================================================================
// Merkle Tree Node abstract class
template <typename T>
class Node
{
public:
  virtual T get_hash() const = 0;
  virtual ~Node() {};
};

//==========================================================================
// Merkle Tree Leaf
template <typename T>
class Leaf: public Node<T>
{
private:
  T hash;

public:
  Leaf(const T& _hash):
    hash(_hash)
  {}

  T get_hash() const
  {
    return hash;
  }
};

//==========================================================================
// Merkle Tree Branch
template <typename T>
class Branch: public Node<T>
{
private:
  HashFunc<T> hash_func;
  unique_ptr<Node<T>> left;
  unique_ptr<Node<T>> right;

public:
  Branch(
    HashFunc<T> _hash_func,
    unique_ptr<Node<T>>& _left,
    unique_ptr<Node<T>>& _right
  ):
    hash_func{_hash_func}, left{std::move(_left)}, right{std::move(_right)}
  {}
  Branch(Branch&& b):
    hash_func{b.hash_func}, left{std::move(b.left)}, right{std::move(b.right)}
  {}

  T get_hash() const
  {
    if (right)
      return hash_func(left->get_hash(), right->get_hash());
    else
      return left->get_hash();
  }
};

//==========================================================================
// Merkle Tree
template<typename T>
class Tree: public Node<T> {
private:
  unique_ptr<Node<T>> root;

  static vector<unique_ptr<Node<T>>> get_parent_nodes(
    const HashFunc<T> hash_func,
    vector<unique_ptr<Node<T>>>& nodes)
  {
    vector<unique_ptr<Node<T>>> parents;
    for (auto i = 0u; i < nodes.size() / 2; ++i)
      parents.push_back(make_unique<Branch<T>>(hash_func, nodes[2 * i], nodes[2 * i + 1]));
    return parents;
  }

  static unique_ptr<Node<T>> build_tree(
    const HashFunc<T> hash_func,
    const vector<T>& leaves)
  {
    vector<unique_ptr<Node<T>>> nodes;
    for (auto i = 0u; i < leaves.size(); ++i)
      nodes.push_back(make_unique<Leaf<T>>(leaves[i]));
    while (nodes.size() > 1)
      nodes = get_parent_nodes(hash_func, nodes);
    return std::move(nodes[0]);
  }

public:
  Tree(const HashFunc<T> hash_func, const vector<T>& leaves):
    root{build_tree(hash_func, leaves)}
  {}
  Tree(Tree&& t):
    root(std::move(t.root))
  {}

  T get_hash() const
  {
    return root->get_hash();
  }
};

//==========================================================================
}} // namespaces

#endif // !__OBTOOLS_MERKLE_H

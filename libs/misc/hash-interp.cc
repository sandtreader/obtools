//==========================================================================
// ObTools::Misc: hash-interp.cc
//
// Hash interpolator.  Takes a PropertyList and generates new properties
// based on hashes of interpolated strings from the existing ones.  Used to
// generate pathname / URL rewrites with load-balancing elements
//
// Copyright (c) 2015 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-misc.h"

namespace ObTools { namespace Misc {

//--------------------------------------------------------------------------
// Constructor from XML, reads <hash> elements from given root e.g.
//  <hash name="foo" modulus="10">$foo</hash>
HashInterpolator::HashInterpolator(const XML::Element& root)
{
  for(XML::Element::const_iterator p(root.get_children("hash")); p; ++p)
  {
    const XML::Element& hash_e = *p;
    add_hash(hash_e["name"], hash_e.get_attr_int("modulus"), *hash_e);
  }
}

//--------------------------------------------------------------------------
// Augment an existing PropertyList with hashes derived from existing
// properties
void HashInterpolator::augment(PropertyList& pl) const
{
  for(list<Hash>::const_iterator p=hashes.begin(); p!=hashes.end(); ++p)
  {
    const Hash& hash = *p;
    string value = pl.interpolate(hash.pattern);
    MD5 md5;
    uint64_t n = md5.hash_to_int(value) % hash.modulus;
    pl.add(hash.name, n);
  }
}

}} // namespaces

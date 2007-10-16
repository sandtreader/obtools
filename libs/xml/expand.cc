//==========================================================================
// ObTools::XML: expand.cc
//
// XML text expander
//
// Copyright (c) 2007 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-xml.h"
#include <unistd.h>
#include <stdio.h>
#include <fstream>
#include <errno.h>

namespace ObTools { namespace XML {

//--------------------------------------------------------------------------
// Expand an XML element into a string - see ot-xml.h for details
// \return the expanded string
string Expander::expand(const XML::Element& root) 
{
  // Start with an empty index list
  map<string, string> indices;
  return expand_recursive(root, indices);
}

//--------------------------------------------------------------------------
// Internal recursive expansion, taking map of current index values
// (= essentially, variable stack)
// Note:  Indices passed by value, so natural C++ stack will unwind and
// provide hiding of outer indices of the same name
string Expander::expand_recursive(const XML::Element& root,
				  map<string, string> indices)
{
  string text = root.content;  // In case it is optimised (in which case it
                               // won't have any children)

  // Check all sub-elements
  for(Element::iterator cp(root.children); cp; ++cp)
  {
    const Element& ce = *cp;

    if (ce.name.empty())
    {
      // Text element - use directly
      text += ce.content;
    }
    else if (ce.name == "expand:value")
    {
      // Simple expansion of the key 
      string key = ce["key"];
      map<string, string>::const_iterator p = expansions.find(key);
      if (p!=expansions.end()) text += p->second;
    }
    else if (ce.name == "expand:if")
    {
      // Expand this recursively if value is 'true' (begins [TtYy1])
      string key = ce["key"];
      map<string, string>::const_iterator p = expansions.find(key);
      if (p!=expansions.end())
      {
	string value = p->second;
	char c = value.empty()?0:value[0];
	if (c=='T' || c=='t' || c=='Y' || c=='y' || c=='1')
	  text += expand_recursive(ce, indices);
      }
    }
    else if (ce.name == "expand:unless")
    {
      // Expand this recursively if value is not 'true' (begins [TtYy1])
      string key = ce["key"];
      map<string, string>::const_iterator p = expansions.find(key);
      if (p!=expansions.end())
      {
	string value = p->second;
	char c = value.empty()?0:value[0];
	if (c!='T' && c!='t' && c!='Y' && c!='y' && c!='1')
	  text += expand_recursive(ce, indices);
      }
    }
    else if (ce.name == "expand:foreach")
    {
      // Iterate over every key beginning with 'key', setting index 'i' 
      // (or specified) to current suffix
      string key = ce["key"];
      string index = ce.get_attr("index", "i");

      for(map<string, string>::const_iterator p = expansions.begin();
	  p!=expansions.end(); ++p)
      {
	if (string(p->first, 0, key.size()) == key)
	{
	  string rest(p->first, key.size());

	  // Set suffix in index
	  indices[index] = rest;

	  // Recurse to children with new index
	  text += expand_recursive(ce, indices);
	}
      }
    }
    else if (ce.name == "expand:each")
    {
      // Get combined key with specified index from indices
      string key = ce["key"];
      string index = ce.get_attr("index", "i");

      map<string, string>::const_iterator p = indices.find(index);
      if (p!=indices.end())
      {
	// Add index value to key
	key += p->second;

	// Now expand as value
	p = expansions.find(key);
	if (p!=expansions.end()) text += p->second;
      }
    }
    else if (ce.name == "expand:index")
    {
      // Get index value
      string index = ce.get_attr("index", "i");

      map<string, string>::const_iterator p = indices.find(index);
      if (p!=indices.end()) text += p->second;
    }
    else
    {
      // Any other element - if it has children,
      // add start tag, expand content and then end-tag
      if (ce.children.size())
      {
	text += ce.start_to_string();
	text += expand_recursive(ce, indices);
	text += ce.end_to_string();
      }
      else
      {
	// Output empty-close form
	text += ce.to_string();
      }
    }
  }

  return text;
}


}} // namespaces

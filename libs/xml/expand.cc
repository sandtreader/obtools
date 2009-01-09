//==========================================================================
// ObTools::XML: expand.cc
//
// XML text expander
//
// Copyright (c) 2007 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-xml.h"
#include <stdio.h>
#include <fstream>
#include <errno.h>
#include <sstream>

namespace ObTools { namespace XML {

//--------------------------------------------------------------------------
// Expand the template with the given value document
// \return the expanded string
string Expander::expand(XML::Element& values)
{
  // Start with top-level template as context, empty vars
  map<string, string> vars;
  return expand_recursive(templ, values, 0, vars);
}

//--------------------------------------------------------------------------
// Internal recursive expansion, taking current template context and
// value context
string Expander::expand_recursive(const XML::Element& templ,
				  XML::Element& values,
				  int index,
				  map<string, string>& vars)
{
  string text = templ.content;  // In case it is optimised (in which case it
                                   // won't have any children)

  // Get an XPath reader on the values
  XPathProcessor xpath(values);

  // Check all sub-elements
  for(Element::iterator tp(templ.children); tp; ++tp)
  {
    const Element& te = *tp;

    if (te.name.empty())
    {
      // Text element - use directly
      text += te.content;
    }
    else if (te.name == "expand:replace"
	     || te.name == "expand:if"
	     || te.name == "expand:unless")
    {
      // All take a value or var attribute
      string value;
      if (te.has_attr("var"))
      {
	// Variable lookup
	string var = te["var"];
	value = vars[var];
      }
      else // Note - empty path is allowed
      {
	// XPath expression
	string path = te["value"];
	value = xpath[path];
      }

      if (te.name == "expand:replace")
      {
	// Simple expansion of the value
	text += value;
      }
      else if (te.name == "expand:if")
      {
	// Expand this recursively if value is 'true' (begins [TtYy1])
	char c = value.empty()?0:value[0];
	if (c=='T' || c=='t' || c=='Y' || c=='y' || c=='1')
	  text += expand_recursive(te, values, index, vars);
      }
      else if (te.name == "expand:unless")
      {
	// Expand this recursively if value is not 'true' (begins [TtYy1])
	char c = value.empty()?0:value[0];
	if (c!='T' && c!='t' && c!='Y' && c!='y' && c!='1')
	  text += expand_recursive(te, values, index, vars);
      }
    }
    else if (te.name == "expand:each")
    {
      // Get all elements matching 'element'
      string path = te["element"];
      list<Element *> elements = xpath.get_elements(path);

      int i=0;
      for(list<Element *>::iterator ep = elements.begin(); 
	  ep!=elements.end(); ++ep)
      {
	Element& e = **ep;

	// Recurse to children with this element as new value structure
	// and this loop's index, with new variables (provides scoping)
	map<string, string> newvars = vars;
	text += expand_recursive(te, e, i++, newvars);
      }
    }
    else if (te.name == "expand:index")
    {
      // Expand to current index in loop
      int base = te.get_attr_int("from", 1);
      ostringstream oss;
      oss << index+base;
      text += oss.str();
    }
    else if (te.name == "expand:set")
    {
      // Set a variable to the content
      string var = te["var"];
      vars[var] = expand_recursive(te, values, index, vars);
    }
    else
    {
      // Any other element - if it has children,
      // add start tag, expand content and then end-tag
      if (te.children.size())
      {
	text += te.start_to_string();
	text += expand_recursive(te, values, index, vars);
	text += te.end_to_string();
      }
      else
      {
	// Output empty-close form
	text += te.to_string();
      }
    }
  }

  return text;
}


}} // namespaces

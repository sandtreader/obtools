//==========================================================================
// ObTools::XMI: ot-xmi.h
//
// Public definitions for ObTools XMI reader
// 
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#ifndef __OBTOOLS_XMI_H
#define __OBTOOLS_XMI_H

#include <string>
#include <list>
#include <iostream>
#include "ot-xml.h"
#include "ot-uml.h"
using namespace std;

namespace ObTools { namespace XMI {

using namespace ObTools::UML;

//==========================================================================
// XMI exceptions
class ParseFailed {};

//==========================================================================
// XMI Reader class
class Reader
{
private:
  ostream& serr;       //error output stream

  void warning(const char *warn, const string& detail);
  void error(const char *err, const string& detail="") throw (ParseFailed);
  Attribute *read_attribute(ObTools::XML::Element& ae);
  Class *read_class(ObTools::XML::Element& ce);
  Association *read_association(ObTools::XML::Element& ae); 
  Package *read_package(ObTools::XML::Element& pe);

public:
  Package *model;        

  //------------------------------------------------------------------------
  // Constructors & Destructor
  // s is output stream for parsing errors
  Reader(ostream &s):
    serr(s),
    model(0)
  {}

  // Default - use cerr
  Reader():
    serr(cerr),
    model(0)
  {}

  ~Reader();

  //------------------------------------------------------------------------
  // Parse from given input stream
  // Throws ParseFailed if parse fails for any fatal reason
  // See also istream operator >> below, which is nicer
  void read_from(istream& s) throw (ParseFailed); 
};

//==========================================================================
// XMI stream operators

//------------------------------------------------------------------------
// >> operator to parse from istream
//
// e.g. cin >> reader;
//
// Throws ParseFailed if bad XML received
istream& operator>>(istream& s, Reader& p) throw (ParseFailed);

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_XMI_H




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

namespace ObTools { namespace XMI {

using namespace std;
using namespace ObTools;

//==========================================================================
// XMI exceptions
class ParseFailed {};

//==========================================================================
// XMI Reader class
class Reader
{
private:
  ostream& serr;                  //error output stream
  map<string, UML::Class *> classmap;  //map of ids to classes

  void warning(const char *warn, const string& detail);
  void error(const char *err, const string& detail="") throw (ParseFailed);
  UML::Class *lookup_class(const string& id);
  UML::Attribute *read_attribute(ObTools::XML::Element& ae);
  void scan_class(ObTools::XML::Element& ce);
  UML::Class *read_class(ObTools::XML::Element& ce);
  UML::Association *read_association(ObTools::XML::Element& ae); 
  UML::Package *read_package(ObTools::XML::Element& pe);

public:
  UML::Package *model;        

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




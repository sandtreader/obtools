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

//Mutual recursion problems - define only what ot-uml.h needs 
//before including it
namespace ObTools { namespace XMI { class Reader; }};
#include "ot-uml.h"

//==========================================================================
// Namespace
namespace ObTools { namespace XMI {

//Make our lives easier without polluting anyone else
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
  ostream& serr;                       //error output stream
  map<string, UML::Element *> idmap;   //map of ids to elements

  //We keep the following to ensure validity of the XML document for our
  //(and our model's) lifetime
  XML::Parser xml_parser;              

public:
  UML::Package *model;        

  //Log a warning
  void warning(const char *warn, const string& detail="");

  //Log an error and bomb out
  void error(const char *err, const string& detail="") throw (ParseFailed);

  //Record an ID to element mappimg
  void record_element(const string& id, UML::Element *e);

  //Get element from ID - return 0 if not found
  UML::Element *lookup_element(const string& id);

  // Constructor
  // s is output stream for parsing errors
  Reader(ostream &s=cerr);

  // Destructor
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




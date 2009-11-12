//==========================================================================
// ObTools::XMI: ot-xmi.h
//
// Public definitions for ObTools XMI reader
// 
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
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

  // Map of IDs to created UML Elements
  map<string, UML::Element *> uml_element_map;

  // Map of ID'd XML elements for things like Multiplicity which aren't
  // created in the model itself
  map<string, XML::Element *> xml_element_map;  

  //We keep the following to ensure validity of the XML document for our
  //(and our model's) lifetime
  XML::Parser xml_parser;              

  void gather_xml_element_ids(XML::Element& e);
  void upgrade_xmi_to_1_1(XML::Element &root);

public:
  UML::Model *model;        
  double xmi_version;   // 0 if unknown  
  map<string, UML::Classifier *> class_map;  // Map of classifiers

  //------------------------------------------------------------------------
  // Constructor
  // s is output stream for parsing errors
  Reader(ostream &s=cerr);

  //------------------------------------------------------------------------
  // Parse from given input stream
  // Throws ParseFailed if parse fails for any fatal reason
  // See also istream operator >> below, which is nicer
  void read_from(istream& s) throw (ParseFailed); 

  //------------------------------------------------------------------------
  // Log a warning
  void warning(const char *warn, const string& detail="");

  //------------------------------------------------------------------------
  // Log an error and bomb out
  void error(const char *err, const string& detail="") throw (ParseFailed);

  //------------------------------------------------------------------------
  // Record an ID to UML element mappimg
  void record_uml_element(const string& id, UML::Element *e);

  //------------------------------------------------------------------------
  // Get UML element from ID - return 0 if not found
  UML::Element *lookup_uml_element(const string& id);

  //------------------------------------------------------------------------
  // Get XML element from ID - return 0 if not found
  // (Note: XML element map is built automatically)
  XML::Element *lookup_xml_element(const string& id);

  //------------------------------------------------------------------------
  // Destructor
  ~Reader();
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




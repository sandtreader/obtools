//==========================================================================
// ObTools::XMI: uxmi.h
//
// Public definitions for ObTools XMI reader
// 
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#ifndef __OBTOOLS_XMI_H
#define __OBTOOLS_XMI_H

#include <string>
#include <list>
#include <deque>
#include <map>
#include <iostream>
using namespace std;

namespace ObTools { namespace XMI {

//==========================================================================
// UML general element (parent of all model elements)
enum ElementVisibility
{
  ELEMENT_PUBLIC,
  ELEMENT_PROTECTED,
  ELEMENT_PRIVATE
};

class Element
{
public:
  string id;          //Intra-file id
  string name;        //Published name
  ElementVisibility visibility;

  //------------------------------------------------------------------------
  //Constructors
  Element(ElementVisibility v=ELEMENT_PUBLIC):
    visibility(v)
  {}

  Element(string& n, ElementVisibility v=ELEMENT_PUBLIC):
    name(n),
    visibility(v) 
  {}

  Element(string& i, string& n, ElementVisibility v=ELEMENT_PUBLIC):
    id(i),
    name(n),
    visibility(v) 
  {}
};

//==========================================================================
// Multiplicity for attributes and associations
struct Multiplicity
{
  int lower; 
  int upper;  // -1 for *

  //Default constructor sets to 1-1
  Multiplicity():
    lower(1),
    upper(1)
  {}
};

// Forward
class Class;

//==========================================================================
// UML Attribute class
class Attribute: Element
{
public:
  Class *type;
  Multiplicity multi;

  //------------------------------------------------------------------------
  //Constructor
  Attribute(string& n, Class *t, 
	    ElementVisibility v=ELEMENT_PRIVATE,
	    Multiplicity m=Multiplicity()):
    Element(n,v),
    type(t),
    multi(m)
  {}
};

//==========================================================================
// UML operation parameter class
enum ParameterKind
{
  PARAMETER_IN,
  PARAMETER_INOUT,
  PARAMETER_OUT,
  PARAMETER_RETURN
};

class Parameter: Element
{
public:
  ParameterKind kind;
  Class *type;
  Multiplicity multi;

  //------------------------------------------------------------------------
  //Constructor
  Parameter(string& n, Class *t, ParameterKind k=PARAMETER_IN,
	    Multiplicity m=Multiplicity()):
    Element(n),
    type(t),
    kind(k),
    multi(m)
  {}
};


//==========================================================================
// UML Operation class
class Operation: Element
{
public:
  list<Parameter> parameters;  //Including return

  //------------------------------------------------------------------------
  //Constructor
  Operation(string& n, 
	    ElementVisibility v=ELEMENT_PUBLIC):
    Element(n,v)
  {}
};

//==========================================================================
// UML Class class
// Also used for Datatypes
enum ClassKind
{
  CLASS_CONCRETE,  // Normal user-defined class
  CLASS_ABSTRACT,  // Abstract user-defined class
  CLASS_PRIMITIVE  // System-defined datatype
};

class Class: Element
{
public:
  ClassKind kind;
  string stereotype;        
  list <Class *> parents;  // Generalisations
  list <Class *> children; // Specialisations
  list <Attribute> attributes;
  list <Operation> operations;

  //------------------------------------------------------------------------
  //Constructors
  Class(string& i, string& n,
	ClassKind k=CLASS_CONCRETE,
	ElementVisibility v=ELEMENT_PUBLIC):
    Element(i,n,v),
    kind(k)
  {}

  //With stereotype
  Class(string& i, string& n, 
	string& st,
	ClassKind k=CLASS_CONCRETE,
	ElementVisibility v=ELEMENT_PUBLIC):
    Element(i,n,v),
    kind(k),
    stereotype(st)
  {}

};

//==========================================================================
// UML AssociationEnd class
enum AggregationKind
{
  AGGREGATION_NONE,
  AGGREGATION_SHARED,
  AGGREGATION_COMPOSITE
};

class AssociationEnd: Element
{
public:
  Class *connection;
  Multiplicity multi;
  AggregationKind aggregation;
  bool navigable;
  bool ordered;

  //------------------------------------------------------------------------
  //Constructors
  AssociationEnd(Class *c,
		 Multiplicity m=Multiplicity(),
		 AggregationKind agg=AGGREGATION_NONE,
		 bool nav=true,
		 bool ord=false):
    Element(),
    connection(c),
    multi(m),
    aggregation(agg),
    navigable(nav),
    ordered(ord)
  {}

  //With name
  AssociationEnd(string& n,
		 Class *c,
		 Multiplicity m=Multiplicity(),
		 AggregationKind agg=AGGREGATION_NONE,
		 bool nav=true,
		 bool ord=false):
    Element(n),
    connection(c),
    multi(m),
    aggregation(agg),
    navigable(nav),
    ordered(ord)
  {}

  //Default for initialisation of Association
  AssociationEnd(): 
    Element(),
    connection(0) 
  {}
};

//==========================================================================
// UML Association class
class Association: Element
{
public:
  Class *type;               // For association classes, usually 0
  AssociationEnd ends[2];

  //------------------------------------------------------------------------
  //Constructors
  Association(Class *t=0):
    Element(),
    type(t)
  {}

  //With name
  Association(string&n, Class *t=0):
    Element(n),
    type(t)
  {}
};

//==========================================================================
// UML Package class
class Package: Element
{
public:
  list<Package> packages;  //Sub-packages
  list<Class> classes;     
  list<Association> associations;

  //Constructors (only here so we can use them in Model)
  Package(string& n): Element(n) {}
  Package(): Element() {}
};

//==========================================================================
// UML Model class
// Essentially the root package, but versioned
class Model: Package
{
public:
  int version;  //XMI version * 100 (e.g. 120)

  //------------------------------------------------------------------------
  //Constructors
  Model(string& n, int v):
    Package(n),
    version(v)
  {}

  //Default constructor for initialisation of Reader - note zero version
  Model(): 
    Package(),
    version(0) 
  {}
};


//==========================================================================
// XMI exceptions
class ParseFailed {};

//==========================================================================
// XMI Reader class
class Reader
{
private:
  ostream& serr;       //error output stream

public:
  Model model;        

  //------------------------------------------------------------------------
  // Constructors & Destructor
  // s is output stream for parsing errors
  Reader(ostream &s):
    serr(s)
  {}

  // Default - use cerr
  Reader():
    serr(cerr)
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




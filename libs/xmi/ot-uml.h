//==========================================================================
// ObTools::XMI: ot-uml.h
//
// Definition of ObTools UML class model
// 
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#ifndef __OBTOOLS_UML_H
#define __OBTOOLS_UML_H

#include <string>
#include <list>

using namespace std;

namespace ObTools { namespace UML {

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

  Element(const string& n, ElementVisibility v=ELEMENT_PUBLIC):
    name(n),
    visibility(v) 
  {}

  Element(const string& i, const string& n, 
	  ElementVisibility v=ELEMENT_PUBLIC):
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
class Attribute: public Element
{
public:
  Class *type;
  Multiplicity multi;

  //Printer
  void print(ostream& sout, int indent=0);

  //Constructor
  Attribute(const string& n, Class *t, 
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

class Parameter: public Element
{
public:
  ParameterKind kind;
  Class *type;
  Multiplicity multi;

  //Constructor
  Parameter(const string& n, Class *t, ParameterKind k=PARAMETER_IN,
	    Multiplicity m=Multiplicity()):
    Element(n),
    type(t),
    kind(k),
    multi(m)
  {}
};


//==========================================================================
// UML Operation class
class Operation: public Element
{
public:
  list<Parameter *> parameters;  //Including return

  //Printer
  void print(ostream& sout, int indent=0);

  //Constructor
  Operation(const string& n, 
	    ElementVisibility v=ELEMENT_PUBLIC):
    Element(n,v)
  {}

  //Destructor
  ~Operation();
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

class Class: public Element
{
public:
  ClassKind kind;
  string stereotype;        
  list <Class *> parents;  // Generalisations
  list <Class *> children; // Specialisations
  list <Attribute *> attributes;
  list <Operation *> operations;

  //Printer
  void print(ostream& sout, int indent=0);

  //Constructor
  Class(const string& i, const string& n,
	ClassKind k=CLASS_CONCRETE,
	ElementVisibility v=ELEMENT_PUBLIC):
    Element(i,n,v),
    kind(k)
  {}

  //Destructor
  ~Class();
};

//==========================================================================
// UML AssociationEnd class
enum AggregationKind
{
  AGGREGATION_NONE,
  AGGREGATION_SHARED,
  AGGREGATION_COMPOSITE
};

class AssociationEnd: public Element
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
  AssociationEnd(const string& n,
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
class Association: public Element
{
public:
  Class *type;               // For association classes, usually 0
  AssociationEnd ends[2];

  //Printer
  void print(ostream& sout, int indent=0);

  //------------------------------------------------------------------------
  //Constructors
  Association(Class *t=0):
    Element(),
    type(t)
  {}

  //With name
  Association(const string& n, Class *t=0):
    Element(n),
    type(t)
  {}
};

//==========================================================================
// UML Package class - also used for top-level model
class Package: public Element
{
public:
  list<Package *> packages;  //Sub-packages
  list<Class *> classes;     
  list<Association *> associations;

  //Printer
  void print(ostream& sout, int indent=0);

  //Constructor
  Package(const string& n): Element(n) {}

  //Destructor
  ~Package();
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_UML_H




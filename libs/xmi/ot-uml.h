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
#include "ot-xml.h"

//Mutual recursion problems - define only what ot-xmi.h needs 
//before including it
namespace ObTools { namespace UML { 
class Element; 
class Package;
}};
#include "ot-xmi.h"

//==========================================================================
// Namespace
namespace ObTools { namespace UML {

//Make our lives easier without polluting anyone else
using namespace std;
using namespace ObTools;

// Forward
class Class;
class DataType;

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

//==========================================================================
// Attribute/Parameter type - either Class or DataType
struct Type
{
  bool is_class;  //Otherwise datatype
  union 
  {
    Class *c;
    DataType *dt;
  } u;
  Multiplicity multi;
};

//==========================================================================
// UML general element (parent of all model elements)
// Handles generic stuff like tree structure, id, name and visibility
enum ElementVisibility
{
  ELEMENT_PUBLIC,
  ELEMENT_PROTECTED,
  ELEMENT_PRIVATE
};

class Element 
{
protected:
  string get_property(const string& attr_name,
		      const string& subelement_name);
  bool get_bool_property(const string& attr_name,
			 const string& subelement_name);
  string get_idref_property(const string& attr_name,
			    const string& subelement_name,
			    const string& subsubelement_name="");
  bool get_type(Type& t);
  void print_subelements(ostream& sout, int indent);

public:
  XMI::Reader& reader;       //Reader that is helping me
  XML::Element& source;      //XML Element I was read from

  string id;                 //Intra-file id
  string name;               //Published name
  ElementVisibility visibility;
  list<Element *> elements;  // Generic list of subelements (used by GC)

  //Function to build things that need valid references (second pass)
  //Default is to pass down to children
  virtual void build_refs();

  //Printing function - defaults to printing type, name, id and subelements
  virtual void print(ostream& sout, int indent=0);

  //Constructor - build from XML element
  //At this level we capture name, id and visibility
  Element(XMI::Reader &rdr, XML::Element& xe);

  //Virtual destructor - defaults to deleting subelements
  virtual ~Element();
};

//==========================================================================
// UML DataType and StereoType classes
// Only here for the RTTI - otherwise just Elements
class DataType: public Element 
{
public:
  //Constructor - just pass up to Element
  DataType(XMI::Reader& rdr, XML::Element& xe):Element(rdr, xe) {}
};

class Stereotype: public Element 
{
public:
  //Constructor - just pass up to Element
  Stereotype(XMI::Reader& rdr, XML::Element& xe):Element(rdr, xe) {}
};

//==========================================================================
// UML Attribute class
class Attribute: public Element
{
public:
  Type type;

  //Printer
  void print(ostream& sout, int indent=0);

  //Second-pass reference fix
  void build_refs();

  //Constructor 
  Attribute(XMI::Reader &rdr, XML::Element& xe);
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
  Type type;

  //Printer
  void print(ostream& sout, int indent=0);

  //Constructor 
  Parameter(XMI::Reader& rdr, XML::Element& xe);
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
  Operation(XMI::Reader& rdr, XML::Element& xe);
};

//==========================================================================
// UML Class class
class Class: public Element
{
public:
  bool is_abstract;
  Stereotype *stereotype;  // Or 0 if none        
  list <Class *> parents;  // Generalisations
  list <Class *> children; // Specialisations
  list <Attribute *> attributes;
  list <Operation *> operations;

  //Printer
  void print(ostream& sout, int indent=0);

  //Second-pass reference fix
  void build_refs();

  //Constructor
  Class(XMI::Reader& rdr, XML::Element& xe);
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

  //Printer
  void print(ostream& sout, int indent=0);

  //Constructor
  AssociationEnd(XMI::Reader& rdr, XML::Element& xe);
};

//==========================================================================
// UML Association class
class Association: public Element
{
public:
  Class *type;               // For association classes, usually 0
  AssociationEnd *ends[2];

  //Printer
  void print(ostream& sout, int indent=0);

  //Constructor 
  Association(XMI::Reader& rdr, XML::Element& xe);
};

//==========================================================================
// UML Package class - also used for top-level model
class Package: public Element
{
public:
  //Filtered lists of particular element types, not used by GC
  list<Package *> packages;      
  list<Class *> classes;     
  list<Association *> associations;

  //Printer
  void print(ostream& sout, int indent=0);

  //Constructor 
  Package(XMI::Reader& rdr, XML::Element& xe);
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_UML_H




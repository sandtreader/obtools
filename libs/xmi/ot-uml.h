//==========================================================================
// ObTools::XMI: ot-uml.h
//
// Definition of ObTools UML class model
//
// Basically a simplified subset of UML1.4 with the main Foundation.Core 
// Foundation.Data-Type and Foundation.Model-Management bits needed
// for simple Class Diagrams
// 
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#ifndef __OBTOOLS_UML_H
#define __OBTOOLS_UML_H

#include <string>
#include <list>
#include <vector>
#include "ot-xml.h"

//Mutual recursion problems - define only what ot-xmi.h needs 
//before including it
namespace ObTools { namespace UML { 
class Element; 
class Classifier;
class Model;
}};
#include "ot-xmi.h"

//==========================================================================
// Namespace
namespace ObTools { namespace UML {

//Make our lives easier without polluting anyone else
using namespace std;
using namespace ObTools;

//==========================================================================
// General data types from the Data-Types package (not UML:DataType!)

//--------------------------------------------------------------------------
// Multiplicity for attributes and associations etc.
// !UML: We only support a single level of range
struct Multiplicity
{
  int lower; 
  int upper;  // -1 for *

  //Default constructor sets to 1-1
  Multiplicity(): lower(1), upper(1) {}
};

ostream& operator<<(ostream& s, const Multiplicity& m);

//--------------------------------------------------------------------------
// Visibility
enum Visibility
{
  VISIBILITY_PUBLIC,
  VISIBILITY_PROTECTED,
  VISIBILITY_PRIVATE,
  VISIBILITY_PACKAGE
};

//--------------------------------------------------------------------------
// Parameter direction
enum ParameterDirection
{
  PARAMETER_IN,
  PARAMETER_INOUT,
  PARAMETER_OUT,
  PARAMETER_RETURN
};

//--------------------------------------------------------------------------
// Aggregation kind
enum AggregationKind
{
  AGGREGATION_NONE,
  AGGREGATION_AGGREGATE,
  AGGREGATION_COMPOSITE
};

//--------------------------------------------------------------------------
// Expression
// (we don't bother with all the subclasses)
struct Expression
{
  string language;
  string body;
};

//--------------------------------------------------------------------------
// Call Concurrency
enum CallConcurrency
{
  CONCURRENCY_SEQUENTIAL,
  CONCURRENCY_GUARDED,
  CONCURRENCY_CONCURRENT
};

//==========================================================================
// Forward refs
class Element;
class Classifier;
class Stereotype;
class GeneralizableElement;
class Generalization;
class Association;
class AssociationEnd;
class Attribute;
class Operation;
class Parameter;

//==========================================================================
//Element factory support
//(This should go inside class Element, but that triggers bug#297 in gcc2.95)

// Typedef for factory template function for any element
typedef Element *(* ElementFactoryFunc)(XMI::Reader&, XML::Element&);

// Template factory function satisfying the above, for any subclass
template<class T> Element *create_element(XMI::Reader& rdr, XML::Element& xe)
{ return new T(rdr, xe); }

//==========================================================================
// UML Element (parent of all UML elements)
// In the UML specification this only holds documentation, but we use
// it to assist in XMI - it holds the XMI id and the model tree structure
// to save subclasses reimplementing it.
//
// Note that the tree structure defined here is the _only_ mechanism
// used to clear up the tree on destruction - all other cross-references
// are weak.  Subclasses provide sugar functions to get filtered lists
// of subelements to make navigation easy without a lot of downcasting
class Element  //abstract
{
protected:
  //Support functions to handle XMI attribute/sub-element option
  string get_property(const string& attr_name,
		      const string& subelement_name);
  bool get_bool_property(const string& attr_name,
			 const string& subelement_name);
  bool get_int_property(const string& attr_name,
			const string& subelement_name,
			int def=0);
  string get_idref_property(const string& attr_name,
			    const string& subelement_name,
			    const string& subsubelement_name="");
  Element *get_element_property(const string& attr_name,
				const string& subelement_name,
				const string& subsubelement_name="");
  Classifier *get_classifier_property(const string& attr_name,
				      const string& subelement_name);
  GeneralizableElement *get_ge_property(const string& attr_name,
					const string& subelement_name);
  Multiplicity get_multiplicity();

  //Support for subclass constructor functions - read all subelements
  //of given types from XML source, and uses factory func to create them
  //If id_required is set (false by default), it ignores any elements 
  //without an 'xmi.id' attribute - these are refs 
  //Prunes descendant tree at 'prune', if given
  void read_subelements(const char *name, ElementFactoryFunc factory,
			bool id_required = false, const char *prune=0);

  // Element header printer - print header line
  // Override this in subclass: upcall to parent, then to add your info 
  // to the end of the line.  Do not add endl.
  virtual void print_header(ostream &sout);

  // Template function for sugar filters - builds list of only subelements
  // matching given type
  template<class T> list<T *> filter_subelements()
  {
    list<T *> l;
    for(list<Element *>::iterator p=subelements.begin();
	p!=subelements.end();
	p++)
    {
      T *t = dynamic_cast<T *>(*p);
      if (t) l.push_back(t);
    }
    return l;
  }

public:
  // XMI support
  XMI::Reader& reader;       //Reader that is helping me
  XML::Element& source;      //XML Element I was read from
  string id;                 //Unique xmi.id
  Element *parent;           //Element we're a part of (0 for root)
  list<Element *> subelements;  //Generic list of subelements (used by GC)

  // Real UML attributes
  string documentation;      //Extracted from TaggedValues

  //Constructor - build from XML element
  //At this level we capture id and documentation (if any), and recurse
  //to subelements
  Element(XMI::Reader &rdr, XML::Element& xe);

  //Function to build things that need valid references for things
  //that might be defined after them (second pass fixup)
  //Default is to pass down to children
  virtual void build_refs();

  // Element printer - indents to indent, calls down to print_header, then 
  // prints sub-elements at indent+2
  // Only override if you want to replace entire printing
  virtual void print(ostream& sout, int indent=0);

  //Virtual destructor - defaults to deleting subelements
  virtual ~Element();
};

//==========================================================================
// UML ModelElement (parent of all modelling elements)
// Handles generic stuff like name, stereotype and visibility

class ModelElement: public Element  //abstract
{
protected:
  void build_refs();                        //Capture stereotype ref
  void print_header(ostream& sout);  
  
public:
  string name;               //Published name
  Stereotype *stereotype;    //0 if none
  Visibility visibility;
  bool is_specification;

  //Constructor
  ModelElement(XMI::Reader &rdr, XML::Element& xe);
};

//==========================================================================
// UML Feature
class Feature: public ModelElement  // abstract
{
protected:
  void print_header(ostream& sout);  

public:
  bool is_static;      //= (ownerScope==sk_classifier)

  //Constructor
  Feature(XMI::Reader& rdr, XML::Element& xe);
};

//==========================================================================
// UML StructuralFeature
class StructuralFeature: public Feature  //abstract
{
protected:
  void build_refs();                        //Capture type
  void print_header(ostream& sout);  

public:
  Multiplicity multiplicity;
  bool is_ordered;
  Classifier *type;

  //Constructor
  StructuralFeature(XMI::Reader& rdr, XML::Element& xe);
};

//==========================================================================
// UML Attribute class
class Attribute: public StructuralFeature
{
protected:
  void print_header(ostream& sout);  

public:
  Expression initial_value;

  //Constructor 
  Attribute(XMI::Reader &rdr, XML::Element& xe);
};

//==========================================================================
// UML Parameter 
class Parameter: public ModelElement
{
protected:
  void build_refs();                        //Capture type
  void print_header(ostream& sout);  

public:
  Expression default_value;
  ParameterDirection kind;
  Classifier *type;

  //Constructor 
  Parameter(XMI::Reader& rdr, XML::Element& xe);
};

//==========================================================================
// UML BehaviouralFeature
class BehaviouralFeature: public Feature  //abstract
{
protected:
  void print_header(ostream& sout);  

public:
  bool is_query;

  //Sugar function to get parameters
  list<Parameter *> get_parameters()
    { return filter_subelements<Parameter>(); }

  //Constructor
  BehaviouralFeature(XMI::Reader& rdr, XML::Element& xe);
};

//==========================================================================
// UML Operation class
class Operation: public BehaviouralFeature
{
  void print_header(ostream& sout);  

public:
  CallConcurrency concurrency;
  bool is_abstract;
  bool is_root;
  bool is_leaf; 
  
  //Constructor
  Operation(XMI::Reader& rdr, XML::Element& xe);
};

//==========================================================================
// UML GeneralizableElement (creates inheritance structure)
// (sorry, fellow GB folk, decided to use the US spelling as in the standard)
class GeneralizableElement: public ModelElement   //abstract
{
protected:
  void print_header(ostream& sout);  

public:
  bool is_root;
  bool is_leaf;
  bool is_abstract;

  // List of pointers to Generalization objects
  list<Generalization *> generalizations; 
  list<Generalization *> specializations; 

  //Constructor
  GeneralizableElement(XMI::Reader &rdr, XML::Element& xe);
};

//==========================================================================
// UML Classifier - Class, DataType or Interface
// Handles associations and features
class Classifier: public GeneralizableElement  // abstract
{
public:
  list<AssociationEnd *> association_ends;  // That refer to us

  //Sugar functions to get list of Attributes and Operations
  list<Attribute *> get_attributes()
    { return filter_subelements<Attribute>(); }
  list<Operation *> get_operations()
    { return filter_subelements<Operation>(); }

  //Constructor
  Classifier(XMI::Reader &rdr, XML::Element& xe);
};

//==========================================================================
// UML Class 
// (most functionality comes from Classifier)
class Class: public Classifier
{
protected:
  void print_header(ostream& sout);  

public:
  bool is_active;

  // Sugar functions to get simple inheritance structure 
  list<Class *> get_parents();
  list<Class *> get_children();

  //Constructor
  Class(XMI::Reader& rdr, XML::Element& xe);
};

//==========================================================================
// UML DataType
// (all functionality comes from Classifier)
class DataType: public Classifier
{
public:
  //Assume DataTypes don't inherit

  //Constructor
  DataType(XMI::Reader& rdr, XML::Element& xe):
    Classifier(rdr, xe) {}
};

//==========================================================================
// UML Interface
// (all functionality comes from Classifier)
class Interface: public Classifier
{
public:
  // Sugar functions to get simple inheritance structure 
  list<Interface *> get_parents();
  list<Interface *> get_children();

  //Constructor
  Interface(XMI::Reader& rdr, XML::Element& xe):
    Classifier(rdr, xe) {}
};

//==========================================================================
// UML AssociationEnd 
class AssociationEnd: public ModelElement
{
protected:
  void build_refs();                        //Capture participant
  void print_header(ostream& sout);  

public:
  bool is_navigable;
  bool is_ordered;
  AggregationKind aggregation;
  Multiplicity multiplicity;
  Classifier *participant;                  //Thing we connect to
  int connection_index;                     //Which end we are in the 
                                            // association - 0..n

  //Constructor
  AssociationEnd(XMI::Reader& rdr, XML::Element& xe);
};

//==========================================================================
// UML Association 
class Association: public GeneralizableElement
{
public:
  // We provide the following rather than a sugar function because it's
  // such a common thing to index
  vector<AssociationEnd *> connections;     //Always 2 or more

  //Constructor 
  Association(XMI::Reader& rdr, XML::Element& xe);
};

//==========================================================================
// UML Relationship
// (no functionality - seems like a placeholder)
class Relationship: public ModelElement  //abstract
{
public:
  //Constructor
  Relationship(XMI::Reader& rdr, XML::Element& xe):
    ModelElement(rdr, xe) {}
};

//==========================================================================
// UML Generalization
class Generalization: public Relationship
{
protected:
  void build_refs();                        //Capture parent, child
  void print_header(ostream& sout);  

public:
  GeneralizableElement *parent;
  GeneralizableElement *child;

  //Constructor - just does safety initialisation - no primitive data
  Generalization(XMI::Reader& rdr, XML::Element& xe):
    parent(0),
    child(0),
    Relationship(rdr, xe) {}
};

//==========================================================================
// UML Primitive
// Here for type only
class Primitive: public DataType
{
public:
  //Constructor
  Primitive(XMI::Reader& rdr, XML::Element& xe):
    DataType(rdr, xe) {}
};

//==========================================================================
// UML Enumeration
class Enumeration: public DataType
{
protected:
  void print_header(ostream& sout);  

public:
  // We just flatten EnumerationLiterals to a string list
  list<string> literals;   //List of literal names

  //Constructor
  Enumeration(XMI::Reader& rdr, XML::Element& xe);
};

//==========================================================================
// UML StereoType 
// We don't bother with much of this except as a name, although it is
// still inheritable from GE
class Stereotype: public GeneralizableElement
{
public:
  //Constructor - just pass up to GE
  Stereotype(XMI::Reader& rdr, XML::Element& xe):
    GeneralizableElement(rdr, xe) {}
};

//==========================================================================
// UML Package 
// Currently ignore Namespace component of this
class Package: public GeneralizableElement
{
public:
  //Sugar functions to get particular element types
  list<Package *> get_subpackages()
    { return filter_subelements<Package>(); }
  list<Class *> get_classes()     
    { return filter_subelements<Class>(); }
  list<Interface *> get_interfaces()     
    { return filter_subelements<Interface>(); }
  list<DataType *> get_datatypes()     
    { return filter_subelements<DataType>(); }
  list<Association *> get_associations()
    { return filter_subelements<Association>(); }

  //Constructor 
  Package(XMI::Reader& rdr, XML::Element& xe);
};

//==========================================================================
// UML Model
// Just a package with a distinguished type and a version
class Model: public Package
{
  void print_header(ostream& sout);  

public:
  double uml_version;     

  //Constructor - after everything loaded up, we call build_refs to 
  //fix up references
  Model(XMI::Reader& rdr, XML::Element& xe, double version=0):
    uml_version(version),
    Package(rdr, xe) 
  { build_refs(); }
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_UML_H




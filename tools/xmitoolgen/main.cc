//==========================================================================
// ObTools::XMITools: xmitoolgen/main.cc
//
// Tool to read an <xt:tool> specification and generate C++ source for an XMI
// tool that implements it
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-toolgen.h"

using namespace ObTools;
using namespace ObTools::ToolGen;
using namespace std;

//==========================================================================
// XMI tool generator
class XMIGenerator: public Generator
{
private:
  //--------------------------------------------------------------------------
  // Scope type
  enum Scope
  {
    SCOPE_ROOT,   // Internal use
    SCOPE_MODEL,
    SCOPE_PACKAGE,
    SCOPE_CLASS,
    SCOPE_ATTRIBUTE,
    SCOPE_OPERATION,
    SCOPE_PARAMETER,
    SCOPE_ASSOCIATION,
    SCOPE_ASSOCIATION_END,
    SCOPE_GENERALIZATION
  };

  Scope get_scope(const string& name);
  string scope_type(Scope scope);
  string scope_nstype(Scope scope);
  string scope_var(Scope scope);

protected:
  //--------------------------------------------------------------------------
  // Obtain an element parameter name to use to in the function generated 
  // by the given template - use shortened scope requested
  // Overridable by 'var' attribute
  virtual string get_parameter_name(XML::Element &te)
  {
    string var = te["var"];
    if (var.size()) return var;

    string name = te.get_attr("scope", "class");
    Scope scope = get_scope(name);
    return scope_var(scope);
  }

  //--------------------------------------------------------------------------
  // Obtain the type of the element parameter for the function
  virtual string get_parameter_type(XML::Element &te)
  { 
    string name = te.get_attr("scope", "class");
    Scope scope = get_scope(name);
    return scope_nstype(scope) + "&";
  }

  //--------------------------------------------------------------------------
  // Iterate over child elements, expanding template inline
  // Accumulates expanded script in 'script'
  virtual void expand_inline(XML::Element& te, XML::Element& parent,
			     CPPT::Tags& tags,
			     int& max_ci,
			     const string& streamname,
			     string& script,
			     bool is_root = false);

  //--------------------------------------------------------------------------
  // Iterate over child elements, calling predefined template
  virtual void expand_use(XML::Element& use_e, 
			  XML::Element& define_e,
			  XML::Element& parent,
			  CPPT::Tags& tags,
			  const string& streamname,
			  bool is_root = false);

  //--------------------------------------------------------------------------
  // Generate includes / file-level code 
  virtual void generate_includes();

  //--------------------------------------------------------------------------
  // Generate code to create 'main' function which reads input and calls
  // templates (use generate_roots() for the latter)
  virtual void generate_main();

public:
  //------------------------------------------------------------------------
  // Constructor - read configuration from config file, output code
  // to given output stream, errors to error stream
  XMIGenerator(const string& _config_file, 
	       ostream& _sout=cout, ostream& _serr=cerr):
    Generator(_config_file, _sout, _serr) {}
};

//--------------------------------------------------------------------------
// Get scope from string
XMIGenerator::Scope XMIGenerator::get_scope(const string& name)
{
  if      (name == "model")     return SCOPE_MODEL;
  else if (name == "package")   return SCOPE_PACKAGE;
  else if (name == "class")     return SCOPE_CLASS;
  else if (name == "attribute") return SCOPE_ATTRIBUTE;
  else if (name == "operation") return SCOPE_OPERATION;
  else if (name == "parameter") return SCOPE_PARAMETER;
  else if (name == "association")     return SCOPE_ASSOCIATION;
  else if (name == "association_end") return SCOPE_ASSOCIATION_END;
  else if (name == "generalization")  return SCOPE_GENERALIZATION;

  serr << "Unknown scope: " << name << endl;  
  return SCOPE_MODEL; 
}

//--------------------------------------------------------------------------
// C++ type for element at given scope
// Note - returns raw type without ObTools::UML namespace
string XMIGenerator::scope_type(Scope scope)
{
  switch (scope)
  {
    case SCOPE_MODEL:     return "Model";
    case SCOPE_PACKAGE:   return "Package";
    case SCOPE_CLASS:     return "Class";
    case SCOPE_ATTRIBUTE: return "Attribute";
    case SCOPE_OPERATION: return "Operation";
    case SCOPE_PARAMETER: return "Parameter";
    case SCOPE_ASSOCIATION:     return "Association";
    case SCOPE_ASSOCIATION_END: return "AssociationEnd";
    case SCOPE_GENERALIZATION:  return "Generalization";
    default: return "";
  }
}

//--------------------------------------------------------------------------
// C++ type for element at given scope, with namespace
string XMIGenerator::scope_nstype(Scope scope)
{
  return "ObTools::UML::" + scope_type(scope);
}

//--------------------------------------------------------------------------
// Variable name for given scope
string XMIGenerator::scope_var(Scope scope)
{
  switch (scope)
  {
    case SCOPE_ROOT:      return "model";
    case SCOPE_MODEL:     return "m";
    case SCOPE_PACKAGE:   return "p";
    case SCOPE_CLASS:     return "c";
    case SCOPE_ATTRIBUTE: return "a";
    case SCOPE_OPERATION: return "o";
    case SCOPE_PARAMETER: return "p";
    case SCOPE_ASSOCIATION:    return "a";
    case SCOPE_ASSOCIATION_END:return "a";
    case SCOPE_GENERALIZATION: return "g";
    default: return "";
  }
}

//--------------------------------------------------------------------------
// Iterate over child elements, expanding template inline
// Accumulates expanded script in 'script'
void XMIGenerator::expand_inline(XML::Element& te, XML::Element& parent,
				 CPPT::Tags& tags,
				 int& max_ci,
				 const string& streamname,
				 string& script,
				 bool is_root)
{
  string sname = te.get_attr("scope", "class");
  Scope scope = get_scope(sname);
  string stype  = scope_type(scope);
  string nstype = scope_nstype(scope);
  string c_var = get_parameter_name(te);

  string p_sname = parent.get_attr("scope", "class");
  Scope p_scope = get_scope(p_sname);
  string p_var = get_parameter_name(parent);  // cope with 'var' override

  sout << "\n  //Call " << sname << " templates\n";

  // Special handling for model/package at root - call on self, since model is
  // a package
  if (is_root
      && (scope == SCOPE_PACKAGE 
       || scope == SCOPE_MODEL))
  {
    // Generate directly using the model as a package
    sout<<"  ObTools::UML::Model& " << c_var << " = *reader.model;\n\n";
    generate_template(te, te, tags, max_ci, "", "", streamname, script);
  }
  else 
  {
    string listop;
    // Special handling for generalizations and associations within classes
    // - not child elements
    if (p_scope == SCOPE_CLASS && scope == SCOPE_GENERALIZATION)
      listop = ".generalizations";
    else if (p_scope == SCOPE_CLASS && scope == SCOPE_ASSOCIATION_END)
      listop = ".association_ends";
    else
      listop = ".filter_subelements<" + nstype + ">()";

    string index_var = c_var + "_index";
    string count_var = c_var + "_count";

    sout << "  int " << index_var << " = 0;\n";
    sout << "  int " << count_var << " = " << p_var << listop << ".size();\n";

    sout << "  OBTOOLS_UML_FOREACH(" << stype << ", " << c_var << ",\n";
    sout << "                      " << p_var << listop << ")\n"; 

    generate_template(te, te, tags, max_ci, index_var, count_var, 
		      streamname, script);
    process_script(script, tags, streamname, max_ci);
    script.clear();

    sout << "  " << index_var << "++;\n";
    sout << "  OBTOOLS_UML_ENDFOR\n";
  }
}

//--------------------------------------------------------------------------
// Iterate over child elements, calling predefined template
void XMIGenerator::expand_use(XML::Element& use_e, 
			      XML::Element& define_e,
			      XML::Element& parent,
			      CPPT::Tags& tags,
			      const string& streamname,
			      bool is_root)
{
  string sname = define_e.get_attr("scope", "class");
  Scope scope = get_scope(sname);
  string stype  = scope_type(scope);
  string nstype = scope_nstype(scope);
  string c_var = get_parameter_name(define_e);

  string p_sname = parent.get_attr("scope", "class");
  Scope p_scope = get_scope(p_sname);
  string p_var = get_parameter_name(parent);  // cope with 'var' override

  sout << "\n  //Call " << sname << " templates\n";

  // Beware:  If we are called recursively, child_var and parent_var
  // could be the same - spot this and modify for it
  if (c_var == p_var) c_var = string("child_") + c_var;

  // Special handling for model/package at root - call on self, since model is
  // a package
  if (is_root
      && (scope == SCOPE_PACKAGE 
       || scope == SCOPE_MODEL))
  {
    // Generate directly using the model as a package
    sout<<"  ObTools::UML::Model& " << c_var << " = *reader.model;\n\n";
    generate_use(use_e, define_e, tags, c_var, "0", "1", streamname);
  }
  else 
  {
    string listop;
    // Special handling for generalizations and associations within classes
    // - not child elements
    if (p_scope == SCOPE_CLASS && scope == SCOPE_GENERALIZATION)
      listop = ".generalizations";
    else if (p_scope == SCOPE_CLASS && scope == SCOPE_ASSOCIATION_END)
      listop = ".association_ends";
    else
      listop = ".filter_subelements<" + nstype + ">()";

    string index_var = c_var + "_index";
    string count_var = c_var + "_count";

    sout << "  int " << index_var << " = 0;\n";
    sout << "  int " << count_var << " = " << p_var << listop << ".size();\n";

    sout << "  OBTOOLS_UML_FOREACH(" << stype << ", " << c_var << ",\n";
    sout << "                      " << p_var << listop << ")\n"; 

    generate_use(use_e, define_e, tags, c_var, 
		 index_var, count_var, streamname);
    
    sout << "  " << index_var << "++;\n";
    sout << "  OBTOOLS_UML_ENDFOR\n";
  }
}

//--------------------------------------------------------------------------
// Generate includes / file-level code 
void XMIGenerator::generate_includes()
{
  Generator::generate_includes();
  sout << "#include \"ot-xmi.h\"\n";
}

//--------------------------------------------------------------------------
// Generate code to create 'main' function which reads input and calls
// templates (use generate_roots() for the latter)
void XMIGenerator::generate_main()
{
  sout<<"//================================================================\n";
  sout<<"// Main function\n";
  sout<<"int main(int argc, char **argv)\n";
  sout<<"{\n";

  sout<<"  // Read configuration\n";
  generate_config_reader();

  sout<<"\n  // Load up XMI from input\n";
  sout<<"  ObTools::XMI::Reader reader;\n";

  sout<<"  try\n";
  sout<<"  {\n";
  sout<<"    cin >> reader;\n";
  sout<<"  }\n";
  sout<<"  catch (ObTools::XMI::ParseFailed)\n";
  sout<<"  {\n";
  sout<<"    cerr << \"XMI parse failed\" << endl;\n";
  sout<<"    return 2;\n";
  sout<<"  }\n\n";

  sout<<"  if (!reader.model) return 4;\n";

  sout<<"  // Call all the template functions with cout\n";

  generate_roots();

  sout<<"  return 0;\n";
  sout<<"}\n";
}

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  char *config_file;

  if (argc > 1)
    config_file = argv[1];
  else
  {
    cout << "Usage:\n";
    cout << "  " << argv[0] << " <config>\n\n";
    cout << "C++ source for tool is produced on stdout\n";
    return 0;
  }

  //Create generator
  XMIGenerator generator(config_file, cout, cerr);
  if (!generator) return 2;

  // Generate code
  generator.generate();

  return 0;  
}






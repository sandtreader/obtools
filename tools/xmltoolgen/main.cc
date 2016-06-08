//==========================================================================
// ObTools::XMLTools: xmltoolgen/main.cc
//
// Tool to read an <xt:tool> specification and generate C++ source for an XML
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
// XML tool generator
class XMLGenerator: public Generator
{
protected:
  //--------------------------------------------------------------------------
  // Obtain an element parameter name to use to in the function generated 
  // by the given template - use name of element requested, or 'child' if none
  // Overridable by 'var' attribute
  virtual string get_parameter_name(XML::Element &te)
  {
    string var = te.get_attr("var", te["element"]);
    if (var.empty()) var = "child";
    return var;
  }

  //--------------------------------------------------------------------------
  // Obtain the type of the element parameter for the function
  virtual string get_parameter_type(XML::Element &)
  { return "ObTools::XML::Element&"; }

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
  // Generate code to create 'main' function which reads input and calls
  // templates (use generate_roots() for the latter)
  virtual void generate_main();

public:
  //------------------------------------------------------------------------
  // Constructor - read configuration from config file, output code
  // to given output stream, errors to error stream
  XMLGenerator(const string& _config_file, 
	       ostream& _sout=cout, ostream& _serr=cerr):
    Generator(_config_file, _sout, _serr) {}
};

//--------------------------------------------------------------------------
// Iterate over child elements, expanding template inline
// Accumulates expanded script in 'script'
void XMLGenerator::expand_inline(XML::Element& te, XML::Element& parent,
				 CPPT::Tags& tags,
				 int& max_ci,
				 const string& streamname,
				 string& script,
				 bool is_root)
{
  string child_var = get_parameter_name(te);

  if (is_root)
  {
    sout << "  //Expand root template\n";
    sout<<"  ObTools::XML::Element& " << child_var 
	<< " = _parser.get_root();\n";

    // Generate directly on the root element
    generate_template(te, te, tags, max_ci, "", "", streamname, script);
  }
  else
  {
    string ename = te.get_attr("element");
    sout << "  //Expand " << (ename.empty()?"all":ename) << " templates\n";

    string parent_var = get_parameter_name(parent);
    string index_var = child_var + "_index";
    string count_var = child_var + "_count";

    sout << "  int " << index_var << " = 0;\n";
    sout << "  int " << count_var;
    if (ename.empty()) // All children
    {
      sout << " = " << parent_var << ".children.size();\n";
      sout << "  OBTOOLS_XML_FOREACH_CHILD(" 
	   << child_var << ", " << parent_var << ")\n";
    }
    else
    {
      sout << " = " <<parent_var<<".get_children(\""<<ename<<"\").size();\n";
      sout << "  OBTOOLS_XML_FOREACH_CHILD_WITH_TAG("
	   << child_var << ", " << parent_var << ", \"" << ename << "\")\n";
    }     


    generate_template(te, te, tags, max_ci, index_var, count_var, 
		      streamname, script);
    process_script(script, tags, streamname, max_ci);
    script.clear();

    sout << " " << index_var << "++;\n";
    sout << "  OBTOOLS_XML_ENDFOR\n";
  }
}

//--------------------------------------------------------------------------
// Iterate over child elements, calling predefined template
void XMLGenerator::expand_use(XML::Element& use_e, 
			      XML::Element& define_e,
			      XML::Element& parent,
			      CPPT::Tags& tags,
			      const string& streamname,
			      bool is_root)
{
  string child_var = get_parameter_name(define_e);

  if (is_root)
  {
    sout << "  //Call root template\n";
    sout<<"  ObTools::XML::Element& " << child_var 
	<< " = _parser.get_root();\n";

    // Generate directly on the root element
    generate_use(use_e, define_e, tags, child_var, "0", "1", streamname);
  }
  else
  {
    sout << "  //Call " << define_e["name"] << " templates\n";

    string ename = define_e.get_attr("element");
    string parent_var = get_parameter_name(parent);

    // Beware:  If we are called recursively, child_var and parent_var
    // could be the same - spot this and modify for it
    if (child_var == parent_var)
      child_var = string("child_") + child_var;

    string index_var = child_var + "_index";
    string count_var = child_var + "_count";

    sout << "  int " << index_var << " = 0;\n";
    sout << "  int " << count_var;
    if (ename.empty()) // All children
    {
      sout << " = " << parent_var << ".children.size();\n";
      sout << "  OBTOOLS_XML_FOREACH_CHILD(" 
	   << child_var << ", " << parent_var << ")\n";
    }
    else
    {
      sout << " = " <<parent_var<<".get_children(\""<<ename<<"\").size();\n";
      sout << "  OBTOOLS_XML_FOREACH_CHILD_WITH_TAG("
	   << child_var << ", " << parent_var << ", \"" << ename << "\")\n";
    }     

    generate_use(use_e, define_e, tags, child_var, 
		 index_var, count_var, streamname);

    sout << "  " << index_var << "++;\n";
    sout << "  OBTOOLS_XML_ENDFOR\n";
  }
}

//--------------------------------------------------------------------------
// Generate code to create 'main' function which reads input and calls
// templates (use generate_roots() for the latter)
void XMLGenerator::generate_main()
{
  sout<<"//================================================================\n";
  sout<<"// Main function\n";
  sout<<"int main(int argc, char **argv)\n";
  sout<<"{\n";

  sout<<"  // Load up XML from input\n";
  sout<<"  ObTools::XML::Parser _parser;\n\n";

  sout<<"  try\n";
  sout<<"  {\n";
  sout<<"    cin >> _parser;\n";
  sout<<"  }\n";
  sout<<"  catch (ObTools::XML::ParseFailed)\n";
  sout<<"  {\n";
  sout<<"    cerr << \"XML parse failed\" << endl;\n";
  sout<<"    return 2;\n";
  sout<<"  }\n\n";

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
  XMLGenerator generator(config_file, cout, cerr);
  if (!generator) return 2;

  // Generate code
  generator.generate();

  return 0;  
}






//==========================================================================
// ObTools::XMLTools: xmltoolgen.cc
//
// Tool to read an <xt:tool> specification and generate C++ source for an XML
// tool that implements it
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-toolgen.h"

using namespace ObTools;
using namespace ObTools::ToolGen;
using namespace std;

//==========================================================================
// XML tool generator, language independent
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
  // Generate code to call a template for all elements of given name
  virtual void generate_call(XML::Element& te, XML::Element& parent,
			     const string& suffix,
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
// Generate code to call a template for all elements of given name
void XMLGenerator::generate_call(XML::Element& te, XML::Element& parent,
				 const string& suffix,
				 bool is_root)
{
  string ename = te.get_attr("element");

  sout << "\n  //Call " << (ename.empty()?"all":ename) << " templates\n";

  if (is_root)
  {
    // Operate on root itself, to cout
    sout << "  template" << suffix << "(cout, root, 0, \"\");\n";
  }
  else 
  {
    string parent_var = get_parameter_name(parent);
    sout << "  _i=0;\n";
    if (ename.empty()) // All children
      sout << "  OBTOOLS_XML_FOREACH_CHILD(_child_e, " << parent_var << ")\n";
    else
      sout << "  OBTOOLS_XML_FOREACH_CHILD_WITH_TAG(_child_e, " 
	   << parent_var << ", \"" << ename << "\")\n";
    sout << "    template" << suffix << "(_sout, _child_e, _i++, _path);\n";
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
  sout<<"  ObTools::XML::Parser parser;\n\n";

  sout<<"  try\n";
  sout<<"  {\n";
  sout<<"    cin >> parser;\n";
  sout<<"  }\n";
  sout<<"  catch (ObTools::XML::ParseFailed)\n";
  sout<<"  {\n";
  sout<<"    cerr << \"XML parse failed\" << endl;\n";
  sout<<"    return 2;\n";
  sout<<"  }\n\n";

  sout<<"  ObTools::XML::Element& root = parser.get_root();\n\n";

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
  XMLGenerator generator(config_file, cout, cerr);
  if (!generator) return 2;

  // Generate code
  generator.generate();

  return 0;  
}






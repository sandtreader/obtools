//==========================================================================
// ObTools::XMLTools: xmltoolgen-cc.cc
//
// Tool to read <xmltool> specification and generate C++ source for an XML
// tool that implements it
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-xml.h"
#include "ot-cppt.h"
#include "ot-text.h"
#include <fstream>
#include <sstream>
#include <cstdlib>

using namespace ObTools;
using namespace std;

//--------------------------------------------------------------------------
// Fatal error handler
void die(const char *err, const char *f="")
{
  cerr << err << ' ' << f << endl;
  exit(2);
}

//--------------------------------------------------------------------------
// Spit out legal boilerplate
void legal(const char *config_file, XML::Configuration& config)
{
  cout<<"//================================================================\n";
  cout << "// Automatically generated by xmltoolgen-cc from " << config_file << endl;
  cout << "// -- DO NOT EDIT -- \n\n";
  cout<<"//================================================================\n";

  cout<<"// This generated code is derived from two sources:\n";
  cout<<"//   1 - A standard tool framework created by xmltoolgen-cc\n";
  cout<<"//   2 - Code templates created from '" << config_file << "'\n";
  cout<<"// There are therefore two copyrights and licenses, below\n\n";

  cout<<"// Note that since the tool framework and libraries (1) are licensed\n";
  cout<<"// under the GNU General Public License (GPL), this entire program\n";
  cout<<"// is covered under the terms of the GPL.\n\n";

  cout<<"// For the avoidance of doubt, xMill Consulting Limited does NOT\n";
  cout<<"// consider the OUTPUT of this program to be a derived work of the\n";
  cout<<"// xmltoolgen framework and libraries (source 1).\n\n";
  cout<<"// HOWEVER, we DO consider the output of this program to be a\n";
  cout<<"// derived work of the code templates contained in '" << config_file << "',\n";
  cout<<"// (source 2) in combination with the XML document that\n";
  cout<<"// the program takes as input\n\n";

  cout<<"// THEREFORE, the ownership and licence for distribution and\n";
  cout<<"// modification of the code generated by this tool are governed\n";
  cout<<"// by a combination of the ownership and licence of the 'code\n";
  cout<<"// templates' licence set out below, and that of the input XML document.\n\n";

  cout<<"//================================================================\n";
  cout<<"// Source 1: Tool framework\n";
  cout<<"// Copyright (c) xMill Consulting Limited 2003\n\n";

  cout<<"// This program is free software; you can redistribute it and/or\n";
  cout<<"// modify it under the terms of the GNU General Public License\n";
  cout<<"// as published by the Free Software Foundation; either version 2\n";
  cout<<"// of the License, or (at your option) any later version.\n\n";

  cout<<"// This program is distributed in the hope that it will be useful,\n";
  cout<<"// but WITHOUT ANY WARRANTY; without even the implied warranty of\n";
  cout<<"// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n";
  cout<<"// GNU General Public License for more details.\n\n";

  cout<<"// You should have received a copy of the GNU General Public\n";
  cout<<"// License along with this program; if not, please see:\n";
  cout<<"//   http://www.obtools.com/license/\n";
  cout<<"// or write to:\n";
  cout<<"//   Free Software Foundation, Inc.,\n";
  cout<<"//   59 Temple Place - Suite 330, Boston, MA\n";
  cout<<"//   02111-1307, USA\n\n";
  
  cout<<"//================================================================\n";
  cout<<"// Source 2: Code templates\n";
  cout << config["legal"] << endl;
  cout<<"//================================================================\n";
}

//--------------------------------------------------------------------------
// Output configuration variables from, er, configuration variables 
void config_vars(XML::Configuration& config)
{
  list<XML::Element *> maps = config.get_elements("config/map");
  list<XML::Element *> vars = config.get_elements("config/var");

  cout<<"//================================================================\n";
  cout<<"// Configuration items\n\n";
  
  cout<<"struct\n{\n";

  // Produce string maps for each map
  for(list<XML::Element *>::iterator p = maps.begin();
      p!=maps.end();
      ++p)
  {
    XML::Element *e = *p;
    cout << "  map<string, string> " << e->get_attr("name") << ";\n";
  }

  // Produce variables for each variable
  for(list<XML::Element *>::iterator p = vars.begin();
      p!=vars.end();
      ++p)
  {
    XML::Element *e = *p;
    string type = e->get_attr("type", "string");
    cout << "  " << type << " " << e->get_attr("name") << ";\n";
  }

  cout << "} config;\n\n";
}

//--------------------------------------------------------------------------
// Read script tags from root containing <script> element
// Using given tags as defaults
CPPT::Tags read_tags(XML::Element& root, CPPT::Tags& defaults)
{
  XML::XPathProcessor xpath(root);
  CPPT::Tags tags;

  tags.start_code = xpath.get_value("script/tags/start-code", 
				    defaults.start_code);
  tags.end_code   = xpath.get_value("script/tags/end-code",
				    defaults.end_code);

  tags.start_expr = xpath.get_value("script/tags/start-expr",
				    defaults.start_expr);
  tags.end_expr   = xpath.get_value("script/tags/end-expr",
				    defaults.end_expr);

  tags.start_comment = xpath.get_value("script/tags/start-comment",
				       defaults.start_comment);
  tags.end_comment   = xpath.get_value("script/tags/end-comment", 
				       defaults.end_comment);

  return tags;
}

//--------------------------------------------------------------------------
// Process a script to cout, using given tags
// Limit common indent removal to max_ci.  If not yet set (<0), sets it to
// common indent of this script
void process_script(const string& script, CPPT::Tags& tags, int& max_ci)
{
  // Tidy up script first - remove leading and trailing blank lines
  string myscript = Text::strip_blank_lines(script);

  if (myscript.size())
  {
    // Remove common indent, up to max_ci
    int ci = Text::get_common_indent(myscript);
    if (max_ci < 0) max_ci = ci;   // Capture first text as max strip
    if (ci > max_ci) ci=max_ci;    // Limit to max strip anyway
    myscript = Text::remove_indent(myscript, ci);

    cout << endl;  // Separate code (not output!)

    // Run it through CPPT
    istringstream sscr(myscript);
    CPPT::Processor processor(sscr, cout, tags, "sout");
    processor.process();
  }
}

//--------------------------------------------------------------------------
// Generate code to call a template for all elements of given name
void generate_call(XML::Element& te, const string& parent_suffix, int i,
		   bool is_root = false)
{
  string ename = te.get_attr("element");

  // Get my suffix appended to parent's
  ostringstream ssuf;
  ssuf << parent_suffix << '_' << i+1;
  string suffix = ssuf.str();

  cout << "\n  //Call " << (ename.empty()?"all":ename) << " templates\n";
  // Declare counter variable the first time
  cout << "  " << (i?"":"int ") << "i=0;\n";

  if (is_root)
  {
    // Operate on root itself, to cout
    cout << "  template" << suffix << "(cout, root, 0, \"\");\n";
  }
  else 
  {
    if (ename.empty()) // All children
      cout << "  OBTOOLS_XML_FOREACH_CHILD(child_e, e)\n";
    else
      cout << "  OBTOOLS_XML_FOREACH_CHILD_WITH_TAG(child_e, e, \"" 
	   << ename << "\")\n";
    cout << "    template" << suffix << "(sout, child_e, i++, path);\n";
    cout << "  OBTOOLS_XML_ENDFOR\n";
  }
}

//--------------------------------------------------------------------------
// Generate code for a particular template element
// max_ci is maximum indent to strip from code
void generate_template(XML::Element& te, CPPT::Tags& tags, 
		       int& max_ci, const string& suffix)
{
  // Check for optimised content
  if (te.content.size()) process_script(te.content, tags, max_ci);

  // Iterate all child elements
  int i=0;
  OBTOOLS_XML_FOREACH_CHILD(ce, te)
    if (ce.name.empty())
    {
      // It's script
      process_script(ce.content, tags, max_ci);
    }
    else if (ce.name == "template")
    {
      // Call to subtemplate, iterating over children
      generate_call(ce, suffix, i++);
    }
  OBTOOLS_XML_ENDFOR
}

//--------------------------------------------------------------------------
// Produce template functions from all templates at given level
void template_funcs(XML::Element& root, CPPT::Tags& tags,
		    int& max_ci, const string& suffix="")
{
  list<XML::Element *> templates = root.get_children("template");

  int i=0;
  for(list<XML::Element *>::iterator p = templates.begin();
      p!=templates.end();
      ++p, ++i)
  {
    XML::Element& e = **p;

    // Get my suffix appended to any existing
    ostringstream ssuf;
    ssuf << suffix << '_' << i+1;
    string mysuffix = ssuf.str();

    // Check for tag override in this element
    CPPT::Tags mytags = read_tags(e, tags); 

    // Produce sub-templates first
    template_funcs(e, mytags, max_ci, mysuffix);

    // Now my code - depends whether we need to open a new file or not
    XML::XPathProcessor xpath(e);
    string fn_script = xpath["filename"];
    string dir_script = xpath["directory"];

    string streamname = "sout";

    if (fn_script.size())
    {
      // Generate function to build filename
      cout<<"//----------------------------------------------------------------\n";
      cout << "// Filename builder for " << e.get_attr("name") << endl;
      cout << "string fn_template" << mysuffix 
	   << "(ObTools::XML::Element& e)\n";
      cout << "{\n";
      cout << "  ostringstream sout;\n";
      int temp=0; // No stripping
      process_script(fn_script, tags, temp);
      cout << "  return sout.str();\n";
      cout << "}\n\n";

      // Rename template function stream parameter so we can replace it
      streamname = "_sout";
    }

    if (dir_script.size())
    {
      // Generate function to build dirname
      cout<<"//----------------------------------------------------------------\n";
      cout << "// Directory name builder for " << e.get_attr("name") << endl;
      cout << "string dn_template" << mysuffix 
	   << "(ObTools::XML::Element& e)\n";
      cout << "{\n";
      cout << "  ostringstream sout;\n";
      int temp=0; // No stripping
      process_script(dir_script, tags, temp);
      cout << "  return sout.str();\n";
      cout << "}\n\n";
    }

    cout<<"//----------------------------------------------------------------\n";
    cout <<"// " << e.get_attr("name") << endl;
    cout << "void template" << mysuffix << "(ostream& " << streamname 
	 << ", ObTools::XML::Element& e, int index, string path)\n";
    cout << "{\n";

    if (dir_script.size())
    {
      // Generate path addition code
      cout << "  path += dn_template" << mysuffix << "(e) + \"/\";\n\n";

      cout << "  // Make directory\n";
      cout << "  string _cmd = string(\"mkdir -p \\\"\")+path+\"\\\"\";\n";
      cout << "  if (system(_cmd.c_str()))\n";
      cout << "  {\n";
      cout << "    cerr << \"Could not \" << _cmd << endl;\n";
      cout << "    exit(2);\n";
      cout << "  }\n\n";
    }

    if (fn_script.size())
    {
      // Generate file open code for sout - this is a bit evil, because of
      // the need to always preserve 'sout' as the output stream
      cout << "  string _fn = path+fn_template" << mysuffix << "(e);\n";
      cout << "  ofstream sout(_fn.c_str());\n";
      cout << "  if (!sout)\n";
      cout << "  {\n";
      cout << "    cerr << \"Can't create file: \" << _fn << endl;\n";
      cout << "    exit(4);\n";
      cout << "  }\n\n";
    }

    generate_template(e, mytags, max_ci, mysuffix);

    cout << "}\n\n";
  }
}

//--------------------------------------------------------------------------
// Produce main function
void do_main(XML::Configuration& config)
{
  cout<<"//================================================================\n";
  cout<<"// Main function\n";
  cout<<"int main(int argc, char **argv)\n";
  cout<<"{\n";

  cout<<"  // Load up XML from input\n";
  cout<<"  ObTools::XML::Parser parser;\n\n";

  cout<<"  try\n";
  cout<<"  {\n";
  cout<<"    cin >> parser;\n";
  cout<<"  }\n";
  cout<<"  catch (ObTools::XML::ParseFailed)\n";
  cout<<"  {\n";
  cout<<"    cerr << \"XML parse failed\" << endl;\n";
  cout<<"    return 2;\n";
  cout<<"  }\n\n";

  cout<<"  ObTools::XML::Element& root = parser.get_root();\n\n";

  cout<<"  // Call all the template functions with cout\n";

  list<XML::Element *> templates = config.get_elements("template");
  int i=0;
  for(list<XML::Element *>::iterator p = templates.begin();
      p!=templates.end();
      ++p)
  {
    XML::Element& e = **p;
    generate_call(e, "", i++, true);
  }

  cout<<"  return 0;\n";
  cout<<"}\n";
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

  // Read config file
  XML::Configuration config(config_file, XML::PARSER_OPTIMISE_CONTENT
			               | XML::PARSER_PRESERVE_WHITESPACE);
  if (!config.read("xmltool")) return 2;

  // Check for correct language
  if (config["script/@language"] != "C++")
    die("Wrong script language - I do C++");

  // Get standard tags
  XML::Element& root = config.get_root();
  CPPT::Tags tags = { "$(", ")$", "$=", "=$", "", "" };
  tags = read_tags(root, tags);

  // Start with our own boilerplate
  legal(config_file, config);

  // Some header stuff
  cout << "\n#include \"ot-xml.h\"\n";

  cout << "#include <fstream>\n";
  cout << "#include <sstream>\n";
  cout << "#include <cstdlib>\n\n";
  cout << "using namespace std;\n\n";

  // Their config items
  config_vars(config);

  // Their custom code
  string code = Text::strip_blank_lines(config["code"]);
  if (code.size())
  {
    cout<<"//================================================================\n";
    cout << "// Custom code from " << config_file << " <code> section\n\n";

    code = Text::remove_indent(code, Text::get_common_indent(code));
    cout << code << endl;
  }

  // Template functions
  cout<<"//================================================================\n";
  cout<<"// Template scripts from "<< config_file <<" <template> sections\n\n";
  int max_ci = -1;
  template_funcs(root, tags, max_ci);

  // Main function
  do_main(config);

  return 0;  
}






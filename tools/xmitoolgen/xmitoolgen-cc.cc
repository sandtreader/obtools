//==========================================================================
// ObTools::XMITools: xmitoolgen-cc.cc
//
// Tool to read <xmitool> specification and generate C++ source for an XMI
// tool that implements it
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-xml.h"
#include "ot-cppt.h"
#include <fstream>
#include <sstream>

using namespace ObTools;

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
  cout<<"//   2 - Output scripts created from '" << config_file << "'\n";
  cout<<"// There are therefore two copyrights and licenses, below\n\n";

  cout<<"// Note that since the tool framework and libraries are licensed\n";
  cout<<"// under the GNU General Public License (GPL), this entire program\n";
  cout<<"// is covered under the terms of the GPL.\n\n";

  cout<<"// For the avoidance of doubt, Object Toolsmiths Limited does NOT\n";
  cout<<"// consider the OUTPUT of this program to be a derived work of the\n";
  cout<<"// xmitoolgen framework and libraries (source 1).\n\n";
  cout<<"// HOWEVER, we DO consider the output of this program to be a\n";
  cout<<"// derived work of the output scripts contained in '" << config_file << "',\n";
  cout<<"// (source 2) in combination with the UML model (in XMI form) that\n";
  cout<<"// the program takes as input\n\n";

  cout<<"// THEREFORE, the ownership and licence for distribution and\n";
  cout<<"// modification of the code generated by this tool are governed\n";
  cout<<"// by a combination of the ownership and licence of the 'output\n";
  cout<<"// scripts' licence set out below, and that of the input UML model.\n\n";

  cout<<"//================================================================\n";
  cout<<"// Source 1: Tool framework\n";
  cout<<"// Copyright (c) Object Toolsmiths Limited 2003\n\n";

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
  cout<<"// Source 2: Output scripts\n";
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
CPPT::Tags read_tags(XML::Element& root)
{
  XML::XPathProcessor xpath(root);
  CPPT::Tags tags;

  tags.start_code = xpath.get_value("script/tags/start-code", "$(");
  tags.end_code   = xpath.get_value("script/tags/end-code",   ")$");

  tags.start_expr = xpath.get_value("script/tags/start-expr", "$=");
  tags.end_expr   = xpath.get_value("script/tags/end-expr",   "=$");

  tags.start_comment = xpath.get_value("script/tags/start-comment", "");
  tags.end_comment   = xpath.get_value("script/tags/end-comment",   "");

  return tags;
}

//--------------------------------------------------------------------------
// C++ type for element at given scope
string scope_type(const string& scope)
{
  if      (scope == "model")     return "ObTools::UML::Model";
  else if (scope == "package")   return "ObTools::UML::Package";
  else if (scope == "class")     return "ObTools::UML::Class";
  else if (scope == "operation") return "ObTools::UML::Operation";
  else if (scope == "attribute") return "ObTools::UML::Attribute";
  else if (scope == "parameter") return "ObTools::UML::Parameter";
  else if (scope == "association")    return "ObTools::UML::Association";
  else if (scope == "generalization") return "ObTools::UML::Generalization";
  else { die("Unknown scope: ", scope.c_str());  return ""; }   
}

//--------------------------------------------------------------------------
// Process a script to cout, using given tags
void process_script(const string& script, CPPT::Tags& tags)
{
  istringstream sscr(script);
  CPPT::Processor processor(sscr, cout, tags, "cout");
  processor.process();
}

//--------------------------------------------------------------------------
// Produce output functions
void output_funcs(XML::Configuration& config)
{
  list<XML::Element *> outputs = config.get_elements("outputs/output");

  cout<<"//================================================================\n";
  cout<<"// Output scripts\n\n";
  
  int i=1;
  for(list<XML::Element *>::iterator p = outputs.begin();
      p!=outputs.end();
      ++p, ++i)
  {
    XML::Element *e = *p;
    cout<<"//----------------------------------------------------------------\n";
    cout<<"// " << e->get_attr("name") << endl;
    string scope = e->get_attr("scope", "class");
    cout << "void output_" << i << "(ostream& sout, " 
	 << scope_type(scope) << "& " << scope[0] << ")\n";
    cout << "{\n";

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

  cout<<"  // Load up XMI from input\n";
  cout<<"  ObTools::XMI::Reader reader;\n\n";

  cout<<"  try\n";
  cout<<"  {\n";
  cout<<"    cin >> reader;\n";
  cout<<"  }\n";
  cout<<"  catch (ObTools::XMI::ParseFailed)\n";
  cout<<"  {\n";
  cout<<"    cerr << \"XMI parse failed\" << endl;\n";
  cout<<"    return 2;\n";
  cout<<"  }\n\n";

  cout<<"  if (!reader.model) return 4;\n\n";

  cout<<"  // Call all the output functions with cout\n";

  list<XML::Element *> outputs = config.get_elements("outputs/output");
  int i=1;
  for(list<XML::Element *>::iterator p = outputs.begin();
      p!=outputs.end();
      ++p, ++i)
  {
    XML::Element *e = *p;
    string scope = e->get_attr("scope", "class");
    cout << "  output_" << i << "(cout, foo);\n";
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
  if (!config.read("xmitool")) return 2;

  // Check for correct language
  if (config["script/@language"] != "C++")
    die("Wrong script language - I do C++");

  // Start with our own boilerplate
  legal(config_file, config);

  // Some header stuff
  cout << "\n#include \"ot-xmi.h\"\n\n";

  // Their config items
  config_vars(config);

  // Their custom code
  cout<<"//================================================================\n";
  cout << "// Custom code from " << config_file << " <code> section\n";
  cout << config["code"] << endl;

  // Output functions
  cout<<"//================================================================\n";
  cout<<"// Output scripts from " << config_file << " <output> sections\n\n";
  output_funcs(config);

  // Main function
  do_main(config);

  return 0;  
}






//==========================================================================
// ObTools::Tools::ToolGen: generator.cc
//
// Output-language-independent implementation of a general tool generator
//
// Copyright (c) 2004 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-toolgen.h"
#include "ot-cppt.h"
#include "ot-text.h"
#include <fstream>
#include <sstream>
#include <cstdlib>

namespace ObTools { namespace ToolGen {

//------------------------------------------------------------------------
// Constructor - read configuration from config file, output code
// to given stream, errors to other given stream
Generator::Generator(const string& _config_file, 
		     ostream& _sout, ostream &_serr):
  config_file(_config_file),
  config(_config_file, 
	 XML::PARSER_OPTIMISE_CONTENT | XML::PARSER_PRESERVE_WHITESPACE),
  sout(_sout), serr(_serr)
{
  config.fix_namespace("xt", "obtools.com/ns/tools");

  if (!config.read("xt:tool"))
    serr << "Can't read tool configuration file " 
	 << config_file << " (no xt:tool)\n";

  // Check for correct language
  if (config["xt:script/@language"] != "C++")
    serr << "Wrong script language - I do C++ only\n";

  XML::Element& root = config.get_root();
  CPPT::Tags deftags = { "$(", ")$", "$=", "=$", "", "" };
  tags = read_tags(root, deftags);

  ok = true;
}

//--------------------------------------------------------------------------
// Generate legal boilerplate
void Generator::generate_legal()
{
  sout<<"//================================================================\n";
  sout<< "// Automatically generated by xmltoolgen-cc from " << config_file; 
  sout<< "\n// -- DO NOT EDIT -- \n\n";
  sout<<"//================================================================\n";

  sout<<"// This generated code is derived from two sources:\n";
  sout<<"//   1 - A standard tool framework created by xmltoolgen-cc\n";
  sout<<"//   2 - Code templates created from '" << config_file << "'\n";
  sout<<"// There are therefore two copyrights and licenses, below\n\n";

  sout<<"// Note that since the tool framework and libraries (1) are licensed\n";
  sout<<"// under the GNU General Public License (GPL), this entire program\n";
  sout<<"// is covered under the terms of the GPL.\n\n";

  sout<<"// For the avoidance of doubt, xMill Consulting Limited does NOT\n";
  sout<<"// consider the OUTPUT of this program to be a derived work of the\n";
  sout<<"// xmltoolgen framework and libraries (source 1).\n\n";
  sout<<"// HOWEVER, we DO consider the output of this program to be a\n";
  sout<<"// derived work of the code templates contained in '" 
      << config_file << "',\n";
  sout<<"// (source 2) in combination with the XML document that\n";
  sout<<"// the program takes as input\n\n";

  sout<<"// THEREFORE, the ownership and licence for distribution and\n";
  sout<<"// modification of the code generated by this tool are governed\n";
  sout<<"// by a combination of the ownership and licence of the 'code\n";
  sout<<"// templates' licence set out below, and that of the input XML document.\n\n";

  sout<<"//================================================================\n";
  sout<<"// Source 1: Tool framework\n";
  sout<<"// Copyright (c) xMill Consulting Limited 2003\n\n";

  sout<<"// This program is free software; you can redistribute it and/or\n";
  sout<<"// modify it under the terms of the GNU General Public License\n";
  sout<<"// as published by the Free Software Foundation; either version 2\n";
  sout<<"// of the License, or (at your option) any later version.\n\n";

  sout<<"// This program is distributed in the hope that it will be useful,\n";
  sout<<"// but WITHOUT ANY WARRANTY; without even the implied warranty of\n";
  sout<<"// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n";
  sout<<"// GNU General Public License for more details.\n\n";

  sout<<"// You should have received a copy of the GNU General Public\n";
  sout<<"// License along with this program; if not, please see:\n";
  sout<<"//   http://www.obtools.com/license/\n";
  sout<<"// or write to:\n";
  sout<<"//   Free Software Foundation, Inc.,\n";
  sout<<"//   59 Temple Place - Suite 330, Boston, MA\n";
  sout<<"//   02111-1307, USA\n\n";
  
  sout<<"//================================================================\n";
  sout<<"// Source 2: Code templates\n";
  sout << config["legal"] << endl;
  sout<<"//================================================================\n";
}

//--------------------------------------------------------------------------
// Output configuration variables from, er, configuration variables 
void Generator::generate_config_vars()
{
  list<XML::Element *> maps = config.get_elements("xt:config/xt:map");
  list<XML::Element *> vars = config.get_elements("xt:config/xt:var");

  sout<<"//================================================================\n";
  sout<<"// Configuration items\n\n";
  
  sout<<"struct\n{\n";

  // Produce string maps for each map
  for(list<XML::Element *>::iterator p = maps.begin();
      p!=maps.end();
      ++p)
  {
    XML::Element *e = *p;
    sout << "  map<string, string> " << e->get_attr("name") << ";\n";
  }

  // Produce variables for each variable
  for(list<XML::Element *>::iterator p = vars.begin();
      p!=vars.end();
      ++p)
  {
    XML::Element *e = *p;
    string type = e->get_attr("type", "string");
    sout << "  " << type << " " << e->get_attr("name") << ";\n";
  }

  sout << "} _config;\n\n";
}

//--------------------------------------------------------------------------
// Read script tags from root containing <xt:script> element
// Using given tags as defaults
CPPT::Tags Generator::read_tags(XML::Element& root, CPPT::Tags& defaults)
{
  XML::XPathProcessor xpath(root);
  CPPT::Tags tags;

  tags.start_code = xpath.get_value("xt:script/xt:tags/xt:start-code", 
				    defaults.start_code);
  tags.end_code   = xpath.get_value("xt:script/xt:tags/xt:end-code",
				    defaults.end_code);

  tags.start_expr = xpath.get_value("xt:script/xt:tags/xt:start-expr",
				    defaults.start_expr);
  tags.end_expr   = xpath.get_value("xt:script/xt:tags/xt:end-expr",
				    defaults.end_expr);

  tags.start_comment = xpath.get_value("xt:script/xt:tags/xt:start-comment",
				       defaults.start_comment);
  tags.end_comment   = xpath.get_value("xt:script/xt:tags/xt:end-comment", 
				       defaults.end_comment);

  return tags;
}

//--------------------------------------------------------------------------
// Process a script to cout, using given tags
// Limit common indent removal to max_ci.  If not yet set (<0), sets it to
// common indent of this script
void Generator::process_script(const string& script, CPPT::Tags& tags, 
			       int& max_ci)
{
  // Tidy up script first - remove leading and trailing blank lines
  string myscript = Text::strip_blank_lines(script);

  if (myscript.size())
  {
    // Remove common indent, up to max_ci
    int ci = Text::get_common_indent(myscript);

    if (max_ci < 0) max_ci = ci;   // Capture first text as max strip
    if (ci > max_ci) ci=max_ci;    // Limit to max strip anyway
    if (ci < max_ci) max_ci = ci;  // Fix up if our first guess was wrong

    myscript = Text::remove_indent(myscript, ci);

    // Run it through CPPT
    istringstream sscr(myscript);
    CPPT::Processor processor(sscr, sout, tags, "_sout");
    processor.process();
  }
}

//--------------------------------------------------------------------------
// Generate code for a particular template element
// e is the current element, te is the most locally enclosing template
//  (which may be the same)
// max_ci is maximum indent to strip from code
// Accumulates script in script, dumps it on hitting a sub-template
void Generator::generate_template(XML::Element& e, XML::Element& te,
				  CPPT::Tags& tags, 
				  int& max_ci, const string& suffix,
				  string& script)
{
  // Check for optimised content
  if (e.content.size()) script += e.content;

  // Iterate all child elements
  int i=0;
  OBTOOLS_XML_FOREACH_CHILD(ce, e)
    if (ce.name.empty())
    {
      // Add content to script
      script += ce.content;
    }
    else 
    {
      // Get my suffix appended to parent's
      ostringstream ssuf;
      ssuf << suffix << '_' << i+1;
      string mysuffix = ssuf.str();

      if (ce.name == "xt:template")
      {
	// Process and clear script before calling sub-template
	process_script(script, tags, max_ci);
	script.clear();

	// Call to subtemplate, iterating over children
	generate_call(ce, te, mysuffix);
      }

      // Recurse to sub-elements, except ignoring xt:xxx 
      if (ce.name.substr(0,3) != "xt:")
      {
	// Add start tag to script
	script += ce.start_to_string();

	// Recurse to generate content, keeping 'te' set the same
	generate_template(ce, te, tags, max_ci, mysuffix, script);

	// Process end tag as a script
	script += ce.end_to_string();
      }
    }
    i++;
  OBTOOLS_XML_ENDFOR
}

//--------------------------------------------------------------------------
// Produce template functions from all templates at given level
void Generator::template_funcs(XML::Element& root, CPPT::Tags& tags,
			       int& max_ci, const string& suffix)
{
  int i=0;
  OBTOOLS_XML_FOREACH_CHILD(e, root)
    if (e.name.size())  // Ignore raw content
    {
      // Get my suffix appended to any existing
      ostringstream ssuf;
      ssuf << suffix << '_' << i+1;
      string mysuffix = ssuf.str();

      // Check for template
      if (e.name == "xt:template")
      {
	// Now my code - depends whether we need to open a new file or not
	XML::XPathProcessor xpath(e);
	string fn_script = xpath["xt:file"];
	string dir_script = xpath["xt:dir"];

	string streamname = "_sout";

	// Check for tag override in this element
	CPPT::Tags mytags = read_tags(e, tags); 

	// Get element parameter name and type
	string var = get_parameter_name(e);
	string vartype = get_parameter_type(e);
	string param = vartype + " " + var;

	int old_max_ci = max_ci;
	if (fn_script.size())
	{
	  // Generate function to build filename
	  sout<<"//----------------------------------------------------------------\n";
	  sout << "// Filename builder for " << e.get_attr("name") << endl;
	  sout << "string fn_template" << mysuffix << "(" << param << ")\n";
	  sout << "{\n";
	  sout << "  ostringstream _sout;\n";
	  int temp=0; // No stripping
	  process_script(fn_script, tags, temp);
	  sout << "  return _sout.str();\n";
	  sout << "}\n\n";

	  // Rename template function stream parameter so we can replace it
	  streamname = "_sout2";

	  // Reset max common indent for new file
	  max_ci = -1;
	}

	if (dir_script.size())
	{
	  // Generate function to build dirname
	  sout<<"//----------------------------------------------------------------\n";
	  sout << "// Directory name builder for " 
	       << e.get_attr("name") << endl;
	  sout << "string dn_template" << mysuffix << "(" << param << ")\n";
	  sout << "{\n";
	  sout << "  ostringstream _sout;\n";
	  int temp=0; // No stripping
	  process_script(dir_script, tags, temp);
	  sout << "  return _sout.str();\n";
	  sout << "}\n\n";
	}

	// Recurse to children 
	template_funcs(e, mytags, max_ci, mysuffix);

	sout<<"//----------------------------------------------------------------\n";
	sout <<"// " << e.get_attr("name") << endl;
	sout << "void template" << mysuffix << "(ostream& " << streamname 
	     << ", " << param 
	     << ", int _index, string _path)\n";
	sout << "{\n";
	sout << "  int _i;\n";

	if (dir_script.size())
	{
	  // Generate path addition code
	  sout <<"  _path += dn_template" << mysuffix 
	       <<"(" << var << ") + \"/\";\n\n";

	  sout <<"  // Make directory\n";
	  sout <<"  string _cmd = string(\"mkdir -p \\\"\")+_path+\"\\\"\";\n";
	  sout <<"  if (system(_cmd.c_str()))\n";
	  sout <<"  {\n";
	  sout <<"    cerr << \"Could not \" << _cmd << endl;\n";
	  sout <<"    exit(2);\n";
	  sout <<"  }\n\n";
	}

	if (fn_script.size())
	{
	  // Generate file open code for sout 
	  sout << "  string _fn = _path+fn_template" << mysuffix 
	       << "(" << var << ");\n";
	  // Output Regen stream if wanted for this file
	  if (xpath.get_value_bool("xt:file/@regen"))
	    sout << "  ObTools::ReGen::rofstream _sout(_fn.c_str());\n";
	  else
	    sout << "  ofstream _sout(_fn.c_str());\n";
	  sout << "  if (!_sout)\n";
	  sout << "  {\n";
	  sout << "    cerr << \"Can't create file: \" << _fn << endl;\n";
	  sout << "    exit(4);\n";
	  sout << "  }\n\n";
	}

	string script;
	generate_template(e, e, mytags, max_ci, mysuffix, script);
	process_script(script, mytags, max_ci);

	sout << "}\n\n";

	// Return common indent for outer file
	if (fn_script.size()) max_ci = old_max_ci;
      }
      else
      {
	// Just recurse
	template_funcs(e, tags, max_ci, mysuffix);
      }
    }
    i++;
  OBTOOLS_XML_ENDFOR
}

//--------------------------------------------------------------------------
// Generate code to run root templates (call from 'main')
void Generator::generate_roots()
{
  // Iterate all child elements
  XML::Element& root = config.get_root();
  int i=0;
  OBTOOLS_XML_FOREACH_CHILD(ce, root)
    if (ce.name == "xt:template")
    {
      // Get my suffix 
      ostringstream ssuf;
      ssuf << '_' << i+1;
      string suffix = ssuf.str();

      // Call to template, iterating over children
      generate_call(ce, root, suffix, true);
    }
    i++;
  OBTOOLS_XML_ENDFOR
}

//--------------------------------------------------------------------------
// Overall tool generator
void Generator::generate()
{
  // Start with our own boilerplate
  generate_legal();

  // Some header stuff
  sout << "\n#include \"ot-xml.h\"\n";

  sout << "#include <fstream>\n";
  sout << "#include <sstream>\n";
  sout << "#include <cstdlib>\n\n";
  sout << "using namespace std;\n\n";

  // Their config items
  generate_config_vars();

  // Their custom code
  string code = Text::strip_blank_lines(config["xt:code"]);
  if (code.size())
  {
    cout<<"//================================================================\n";
    cout << "// Custom code from " << config_file << " <xt:code> section\n\n";

    code = Text::remove_indent(code, Text::get_common_indent(code));
    sout << code << endl;
  }

  // Template functions
  sout<<"//================================================================\n";
  sout<<"// Template scripts from "<< config_file 
      <<" <xt:template> sections\n\n";
  int max_ci = -1;
  template_funcs(config.get_root(), tags, max_ci);

  // Main function
  generate_main();
}



}} // namespaces



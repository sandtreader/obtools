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
  ok(false),
  config(_config_file, 
	 XML::PARSER_OPTIMISE_CONTENT 
       | XML::PARSER_PRESERVE_WHITESPACE
       | XML::PARSER_BE_LENIENT),
  sout(_sout), serr(_serr)
{
  config.fix_namespace("xt", "obtools.com/ns/tools");

  if (!config.read("xt:tool"))
  {
    serr << "Can't read tool configuration file " 
	 << config_file << " (no xt:tool)\n";
    return;
  }

  // Check for correct language
  if (config["xt:script/@language"] != "C++")
  {
    serr << "Wrong script language - I do C++ only\n";
    return;
  }

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
  sout<< "// Automatically generated by a toolgen program from " << config_file; 
  sout<< "\n// -- DO NOT EDIT -- \n\n";
  sout<<"//================================================================\n";

  sout<<"// This generated code is derived from two sources:\n";
  sout<<"//   1 - A standard tool framework created by ObTools toolgen\n";
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
  sout << config["xt:legal"] << endl;
  sout<<"//================================================================\n";
}

//--------------------------------------------------------------------------
// Generate includes / file-level code 
void Generator::generate_includes()
{
  sout << "\n#include \"ot-xml.h\"\n";
  sout << "\n#include \"ot-text.h\"\n";

  sout << "#include <fstream>\n";
  sout << "#include <sstream>\n";
  sout << "#include <cstdlib>\n\n";
  sout << "using namespace std;\n\n";
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
// Process a script to sout, using given tags
// Limit common indent removal to max_ci.  If not yet set (<0), sets it to
// common indent of this script
void Generator::process_script(const string& script, CPPT::Tags& tags, 
			       const string& streamname, int& max_ci)
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
    CPPT::Processor processor(sscr, sout, tags, streamname);
    processor.process();
  }
}

//--------------------------------------------------------------------------
// Generate xt:start include when [indexname] is zero
void Generator::generate_start(XML::Element& te,
			       CPPT::Tags& tags, 
			       const string& indexname,
			       const string& streamname)
{
  OBTOOLS_XML_FOREACH_CHILD_WITH_TAG(se, te, "xt:start")
    int temp=0; // No stripping
    sout << "  if (!" << indexname << ")\n  {\n  ";
    process_script(*se, tags, streamname, temp);
    sout << "  }\n";
  OBTOOLS_XML_ENDFOR
}
			
//--------------------------------------------------------------------------
// Generate xt:end include if [indexname] is non-zero
void Generator::generate_end(XML::Element& te,
			     CPPT::Tags& tags, 
			     const string& indexname,
			     const string& streamname)
{
  OBTOOLS_XML_FOREACH_CHILD_WITH_TAG(se, te, "xt:end")
    int temp=0; // No stripping
    sout << "  if (" << indexname << ")\n  {\n  ";
    process_script(*se, tags, streamname, temp);
    sout << "  }\n";
  OBTOOLS_XML_ENDFOR
}

//--------------------------------------------------------------------------
// Generate use of a predefined 'macro' template
// Accumulates script in script
void Generator::generate_use(XML::Element& use_e, 
			     XML::Element& define_e,
			     CPPT::Tags& tags, 
			     const string& childname,
			     const string& indexname,
			     const string& streamname)
{
  // Create script expansion for each parameter
  map <string, bool> params_used;

  OBTOOLS_XML_FOREACH_CHILD_WITH_TAG(pe, use_e, "xt:param")
    string pn = pe["name"];

    // Generate param stream name
    string psn = string("_param_s_")+pn;
 
    // Generate code to build dirname
    sout << "  ostringstream " << psn << ";\n";
    int temp=0; // No stripping
    process_script(*pe, tags, psn, temp);
    sout << "  string _param_" << pn << " = " << psn << ".str();\n";
    params_used[pn] = true;
  OBTOOLS_XML_ENDFOR

  // Generate call to template function
  sout << "  //Call to defined template '" << define_e["name"] << "'\n";
  sout << "  template_" << define_e["name"] << "(" << streamname << ", "
       << childname << ", " << indexname << ", _path";

  // Check each argument in the definition to see if we've provided it, 
  // and generate it, use default if not
  OBTOOLS_XML_FOREACH_CHILD_WITH_TAG(ae, define_e, "xt:arg")
    sout << ",\n    ";
    string an = ae["name"];
    if (params_used[an])
      sout << "_param_" << an;
    else
      sout << "\"" << ae["default"] << "\"";
  OBTOOLS_XML_ENDFOR

  sout << ");\n";
}

//--------------------------------------------------------------------------
// Generate code for a particular template element
// e is the current element, te is the most locally enclosing template
//  (which may be the same)
// max_ci is maximum indent to strip from code
// Accumulates script in script, dumps it on hitting a sub-template
void Generator::generate_template(XML::Element& e, XML::Element& te,
				  CPPT::Tags& tags, 
				  int& max_ci, 
				  const string& streamname,
				  string& script)
{
  // Check for optimised content
  if (e.content.size()) script += e.content;

  // Now my code - depends whether we need to open a new file or not
  XML::XPathProcessor xpath(e);
  string fn_script = xpath["xt:file"];
  string dir_script = xpath["xt:dir"];

  // Check for tag override in this element
  CPPT::Tags mytags = read_tags(e, tags); 

  string mystream = streamname;
  int old_max_ci = max_ci;

  if (dir_script.size())
  {
    // Generate code to build dirname
    sout << "  ostringstream _dirname_s;\n";
    int temp=0; // No stripping
    process_script(dir_script, tags, "_dirname_s", temp);
    sout << "  string _dirname = _dirname_s.str();\n";

    // Generate path addition code
    sout <<"  string _oldpath = _path;\n";
    sout <<"  _path += _dirname + \"/\";\n\n";

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
    // Generate code to build filename
    sout << "  ostringstream _filename_s;\n";
    int temp=0; // No stripping
    process_script(fn_script, tags, "_filename_s", temp);
    sout << "  string _filename = _path + _filename_s.str();\n";

    // Output Regen stream if wanted for this file
    if (xpath.get_value_bool("xt:file/@regen"))
      sout << "  ObTools::ReGen::rofstream";
    else
      sout << "  ofstream";

    sout << " _sout(_filename.c_str());\n";
    sout << "  if (!_sout)\n";
    sout << "  {\n";
    sout << "    cerr << \"Can't create file: \" << _filename <<endl;\n";
    sout << "    exit(4);\n";
    sout << "  }\n\n";

    // Reset max common indent for new file
    max_ci = -1;

    // Move to new stream for subsequent output
    mystream = "_sout";
  }

  // Iterate all child elements
  OBTOOLS_XML_FOREACH_CHILD(ce, e)
    if (ce.name.empty())
    {
      // Add content to script
      script += ce.content;
    }
    else 
    {
      if (ce.name == "xt:template")
      {
	// Process and clear script before calling sub-template
	process_script(script, tags, mystream, max_ci);
	script.clear();

	// Give us a block context to avoid redeclarations
	sout << "\n  {\n";

	// Iterate all children through inline expanded template
	expand_inline(ce, te, mytags, max_ci, mystream, script);
	process_script(script, mytags, mystream, max_ci);
	script.clear();

	// Close block context
	sout << "  }\n\n";
      }
      else if (ce.name == "xt:use")
      {
	// Find defined template
	string def = ce["template"];
	if (def.size())
	{
	  XML::Element *def_e = defines[def];
	  if (def_e)
	  {
	    XML::Element& de = *def_e;

	    // Process and clear script before calling sub-template
	    process_script(script, tags, mystream, max_ci);
	    script.clear();

	    // Give us a block context to avoid redeclarations
	    sout << "\n  {\n";

	    // Iterate all children through called template
	    expand_use(ce, de, te, tags, mystream);

	    // Close block context
	    sout << "  }\n\n";
	  }
	  else serr << "No such template defined: " << def << endl;
	}
	else serr << "No 'template' argument for xt:use\n";
      }

      // Recurse to sub-elements, except ignoring xt:xxx 
      if (ce.name.substr(0,3) != "xt:")
      {
	// Add start tag to script
	script += ce.start_to_string();

	// Recurse to generate content, keeping 'te' set the same
	generate_template(ce, te, tags, max_ci, mystream, script);

	// Process end tag as a script
	script += ce.end_to_string();
      }
    }
  OBTOOLS_XML_ENDFOR

  // Process any tail-end script
  process_script(script, mytags, mystream, max_ci);
  script.clear();

  // Output code to restore path
  if (dir_script.size())
    sout <<"  _path = _oldpath;\n";

  // Return common indent for outer file
  if (fn_script.size()) max_ci = old_max_ci;
}

//--------------------------------------------------------------------------
// Read all xt:code elements and include them verbatim
void Generator::generate_code()
{
  XML::Element& root = config.get_root();
  OBTOOLS_XML_FOREACH_CHILD_WITH_TAG(ce, root, "xt:code")
    string code = Text::strip_blank_lines(*ce);
    if (code.size())
    {
      sout<<"//================================================================\n";
      sout << "// Custom code from " << config_file 
	   << " <xt:code> section\n\n";

      code = Text::remove_indent(code, Text::get_common_indent(code));
      sout << code << endl;
    }
  OBTOOLS_XML_ENDFOR
}

//--------------------------------------------------------------------------
// Read all xt:define elements, code generate them, and hold for use later
void Generator::generate_defines()
{
  XML::Element& root = config.get_root();
  OBTOOLS_XML_FOREACH_CHILD_WITH_TAG(te, root, "xt:define")
    string name = te["name"];

    // Store this element in map, keyed by name
    defines[name] = &te;

    // Get parameter for template function
    string p_var = get_parameter_name(te);
    string p_type = get_parameter_type(te);
    string param = p_type + " " + p_var;

    sout<<"//================================================================\n";
    sout << "// Defined template '" << name << "'\n";
    sout << "void template_" << name << "(ostream& sout, " << param << ",\n";
    sout << "     int " << p_var << "_index, string _path";

    // Create argument strings
    OBTOOLS_XML_FOREACH_CHILD_WITH_TAG(ae, te, "xt:arg")
      sout << ",\n     string " << ae["name"];
    OBTOOLS_XML_ENDFOR
    sout << ")\n{\n";

    int max_ci = -1;
    string script;
    generate_template(te, te, tags, max_ci, "sout", script);
    process_script(script, tags, "sout", max_ci);
    script.clear();

    sout << "}\n\n";
  OBTOOLS_XML_ENDFOR
}

//--------------------------------------------------------------------------
// Generate code to run root templates (call from 'main')
void Generator::generate_roots()
{
  XML::Element& root = config.get_root();

  // Expand all root templates
  OBTOOLS_XML_FOREACH_CHILD_WITH_TAG(ce, root, "xt:template")
    // Get my suffix 
    string script;
    int max_ci = -1;

    // Give us a block context to avoid redeclarations
    cout << "\n  {\n";

    // Call to template, iterating over children
    expand_inline(ce, root, tags, max_ci, "cout", script, true);

    // Close block context
    cout << "  }\n\n";
  OBTOOLS_XML_ENDFOR

  // Call all macros at root
  OBTOOLS_XML_FOREACH_CHILD_WITH_TAG(ce, root, "xt:use")
    // Find defined template
    string def = ce["template"];
    if (def.size())
    {
      XML::Element *def_e = defines[def];
      if (def_e)
      {
	XML::Element& de = *def_e;

	// Give us a block context to avoid redeclarations
	cout << "\n  {\n";

	// Iterate all children through called template
	expand_use(ce, de, root, tags, "cout", true);

	// Close block context
	cout << "  }\n\n";
      }
      else serr << "No such template defined: " << def << endl;
    }
    else serr << "No 'template' argument for xt:use\n";
  OBTOOLS_XML_ENDFOR
}

//--------------------------------------------------------------------------
// Overall tool generator
void Generator::generate()
{
  // Start with our own boilerplate
  generate_legal();

  // Some header stuff
  generate_includes();

  // Their config items
  generate_config_vars();

  // Their custom code
  generate_code();

  // Generate defined templates ('macros')
  generate_defines();

  // Main function
  generate_main();
}



}} // namespaces



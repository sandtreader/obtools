//==========================================================================
// ObTools::Tools::Toolgen: ot-toolgen.h
//
// Core definitions for all Tool Generators
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#ifndef __OBTOOLS_TOOLGEN_H
#define __OBTOOLS_TOOLGEN_H

#include <string>
#include "ot-xml.h"
#include "ot-cppt.h"

namespace ObTools { namespace ToolGen {

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Abstract tool generator, language independent
class Generator
{
private:
  const string& config_file;  // Filename of config file
  CPPT::Tags tags;            // Base tags
  bool ok;                    // Whether started OK

  CPPT::Tags read_tags(XML::Element& root, CPPT::Tags& defaults);
  void process_script(const string& script, CPPT::Tags& tags, int& max_ci);
  void template_funcs(XML::Element& root, CPPT::Tags& tags,
		      int& max_ci, const string& suffix="");
  void generate_legal();
  void generate_config_vars();

protected:
  XML::Configuration config;  // Input configuration
  ostream& sout;              // Output stream for code
  ostream& serr;              // Output stream for errors

  //--------------------------------------------------------------------------
  // Obtain an element parameter name to use to in the function generated 
  // by the given template
  virtual string get_parameter_name(XML::Element &te) = 0;

  //--------------------------------------------------------------------------
  // Obtain the type of the element parameter for the function
  virtual string get_parameter_type(XML::Element &te) = 0;

  //--------------------------------------------------------------------------
  // Generate code to call a template for all elements of given name
  virtual void generate_call(XML::Element& te, XML::Element& parent,
			     const string& suffix,
			     bool is_root = false) = 0;

  //--------------------------------------------------------------------------
  // Generate code for a particular template element
  // e is the current element, te is the most locally enclosing template
  //  (which may be the same)
  // max_ci is maximum indent to strip from code
  // Accumulates script in script, dumps it on hitting a sub-template
  virtual void generate_template(XML::Element& te, XML::Element& parent,
				 CPPT::Tags& tags, 
				 int& max_ci, const string& suffix,
				 string& script);

  //--------------------------------------------------------------------------
  // Generate code to call root templates
  void generate_roots();

  //--------------------------------------------------------------------------
  // Generate includes / file-level code 
  virtual void generate_includes();

  //--------------------------------------------------------------------------
  // Generate code to create 'main' function which reads input and calls
  // templates (use generate_roots() for the latter)
  virtual void generate_main() = 0;

  //--------------------------------------------------------------------------
  // Virtual destructor
  virtual ~Generator() {}

public:
  //------------------------------------------------------------------------
  // Constructor - read configuration from config file, output code
  // to given output stream, errors to error stream
  Generator(const string& _config_file, 
	    ostream& _sout=cout, ostream& serr=cerr);

  //------------------------------------------------------------------------
  // Validity check
  bool operator!() { return !ok; }

  //--------------------------------------------------------------------------
  // Overall tool generator
  void generate();
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_TOOLGEN_H




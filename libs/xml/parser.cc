//==========================================================================
// ObTools::XML: parser.cc
//
// XML parser
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-xml.h"
#include <sstream>
using namespace ObTools::XML;


//--------------------------------------------------------------------------
// Parse a stream
// This is the main document-level parser
//
// Throws XML::ParseFailed if unsuccessful
void Parser::parse_stream(istream& s) throw (ParseFailed)
{
  // Clear stacks and root
  elements.clear();
  ns_maps.clear();
  if (root) delete root;
  root = 0;

  bool done=false;
  while (!done)
  {
    xmlchar c=0;

    //Read first character, stripping all leading whitespace
    //XML: This violates the requirement to send everything to the application
    //but indicate that some whitespace is insignificant.  We assume that
    //the application isn't interested in leading whitespace unless it wants
    //everything (PRESERVE_WHITESPACE), and we don't implement per-element
    //xml:space semantics either

    //Except - if we've been asked to and within the document,
    //whitespace as normal character data
    if (root && (flags & PARSER_PRESERVE_WHITESPACE))
      s.get(c);
    else
      c = skip_ws(s);

    //Now's a good time for a sanity check
    if (!c)
    {
      if (root)
	fatal("Input stream failed unexpectedly");
      else
	fatal("Empty or unreadable document");
    }

    //Check for tag or text
    if (c == '<')
    {
      // Process tags - next character identifies type
      s.get(c);  //No WS strip
      switch (c)
      {
	case '!':  // Comment or DOCTYPE
	  s.get(c);
	  switch (c)
	  {
	    case '-':  // Comment
	      s.get(c); // Must read this, otherwise we'll accept <!--->
	      if (c=='-')
		skip_comment(s);
	      else
	      {
		error("Weird comment");
		skip_to_gt(s);  //Try to recover
	      }
	      break;

	    default:  // Anything else, don't much care
	      skip_to_gt(s);
	      break;
	  }
	  break;

	case '?':  // Prolog or other PI - just ignore
	  skip_pi(s);
	  break;

	case '/': // End tag
	  s.get(c);
	  if (is_name_start(c))
	    read_end_tag(c, s);
	  else
	    fatal("Illegal end tag");

	  //Check for end - can only happen when stack goes empty after
	  //end tag
	  if (elements.empty()) done=true;
	  break;
       
	default:  // Something else - could be tag
	  if (is_name_start(c))
	  {
	    // Check for empty tag & empty stack - done
	    if (read_tag(c, s) && elements.empty()) done=true;
	  }
	  else
	  {
	    // Check for leniency here - if so, keep it as character
	    // data, like SGML would
	    if (flags & PARSER_BE_LENIENT)
	    {
	      s.unget();              // Push back second char
	      read_content('<', s);   // Keep '<' as text
	    }
	    else fatal("Illegal tag");
	  }
      }
    }
    else if (root)
    {
      //Read content into data sub-elements
      read_content(c, s);
    }
    else
    {
      //If we're reading the root, only a tag-like thing is OK
      fatal("Non-tag data at start of document");
    }
  }
}

//--------------------------------------------------------------------------
// Read a tag and attributes
// c is first character already read
// Returns whether element read is empty
bool Parser::read_tag(xmlchar c, istream &s) throw (ParseFailed)
{
  string name = read_rest_of_name(c, s);

  //OK, now we have a valid new element
  Element *e = new Element(name);
  e->line = line;

  //Now loop looking for attributes or >
  bool empty=false;
  for(;;)
  {
    //Next must be / > or whitespace, or it's an error
    if (!isspace(c) && c!='/' && c!='>')
      fatal("Illegal start tag");

    //If it's whitespace (not / or >), skip it
    if (c!='/' && c!='>') c = skip_ws(s, c);


    //Empty close />
    if (c=='/')
    {
      empty=true;
      s.get(c);
      if (c!='>') fatal("Illegal empty close");
      break;
    }

    //Normal close
    if (c=='>') break;

    //It must be a name start now
    if (!is_name_start(c)) fatal("Illegal attribute name");

    //Get attribute name
    string aname = read_rest_of_name(c, s);

    //This must be unique
    if (e->attrs.find(aname) != e->attrs.end())
      fatal("Duplicate attribute name");

    //Skip optional WS, then ensure =
    if (isspace(c)) c = skip_ws(s, c);
    if (c!='=') fatal("No = given for attribute");

    //Skip optional WS again and get start of value 
    c = skip_ws(s);

    //Must be some kind of quote (this is XML, not SGML!)
    if (c!='"' && c!='\'') fatal("Attribute value not quoted");
    xmlchar quote = c;

    //Read attribute value to matching quote
    string aval;
    for(;;)
    {
      c=0;
      s.get(c);
      if (!c) fatal("Document ended in attribute value");
      if (c==quote) break;
      if (c=='&') 
	read_ref(aval, s);  //Expand refs
      else
	aval+=c;
    }

    //Add this attribute to the map
    e->attrs[aname]=aval;

    //Get next char and loop to optional whitespace before close or
    //another attribute
    c=0;
    s.get(c);
  }

  //Add this as a child of the last open element, if any
  if (elements.size()) elements.back()->add(e);

  //Process namespaces etc. for this element
  initial_processing(e);

  //If empty, finalise it now, otherwise stack it as new open element
  if (empty) 
    final_processing(e);
  else
    elements.push_back(e);

  //If we don't have a root before, we do now
  if (!root) root = e;

  return empty;
}

//--------------------------------------------------------------------------
// Read an end tag
// c is first character already read
void Parser::read_end_tag(xmlchar c, istream &s) throw (ParseFailed)
{
  string name = read_rest_of_name(c, s);

  //Skip optional whitespace
  if (isspace(c)) c=skip_ws(s, c);

  //This must now be a '>' or its an error
  if (c!='>') throw ParseFailed();

  //This should be whatever's on the top of the stack...
  //... if there is a stack
  if (elements.size())
  {
    Element *e = elements.back();
    if (name==e->name)
    {
      //That's OK, pop the stack
      elements.pop_back();

      //Finalise this element (translate namespaces etc.)
      final_processing(e);
    }
    else
    {
      //Complain, but ignore it
      ostringstream oss;
      oss << "Mis-nested tags - expected </" << e->name 
	  << ">, opened at line " << e->line << ", but got </"
	  << name << ">";
      error(oss.str());
    }
  }
  else
  {
    //Stack empty - complain but ignore it 
    error("End-tag found but no elements open");
  }
}

//--------------------------------------------------------------------------
// Read data content
// Handles whitespace compression and tail stripping
// c is first character already read
void Parser::read_content(xmlchar c, istream &s) throw (ParseFailed)
{
  // Read until non-escaped '<'
  string content;
  bool first = true;

  for(;;)
  {
    // Use passed-in character the first time round

    //Magic whitespace handling - if this is whitespace, keep a single
    //space unless we are about to end, then strip to read the rest
    //(If we're preserving whitespace, ignore all this and treat it as
    // any other character)
    if (!(flags & PARSER_PRESERVE_WHITESPACE) && isspace(c))
    {
      c = skip_ws(s, c);
      if (c!='<') content+=' ';  //Compress to single space, suppress at end
      first = false;
    }

    // First (passed in) character '<' is allowed if lenient
    if (c=='<' && !first)
    {
      //New tag - push this back and stop
      s.unget();
      break;
    }

    //Expand &
    if (c=='&')
      read_ref(content, s);
    else if (!c)
      fatal("Unexpected end of stream");
    else 
    {
      if (c=='\n') line++;   // Count unmashed lines if preserving
      content+=c;
    }
    
    // Read another for next time
    c=0;
    s.get(c);
    first = false;
  }

  //Create sub-element with content, and add it to currently open element
  if (elements.size()) 
  {
    Element *e = elements.back();

    //See if the last thing we read was also a content element - if so,
    //we probably got broken by a comment, and it would be nice to just
    //continue it so it remains available for optimisation
    if (e->children.size() && e->children.back()->name.empty())
    {
      //We'll treat the comment like whitespace, because we suppressed
      //any that was at end of previous one or start of this.

      //XML: This means that 'foo<!-- -->bar' gets turned into 'foo bar' 
      //rather than 'foobar'.  I suppose I could do some
      //horrendous remembering of front and back suppression so we
      //know if this space is really there, but I think anyone not
      //using PRESERVE_WHITESPACE is unlikely to care.  If they are
      //using PRESERVE_WHITESPACE, though, we'd better not mess it up
      if (!(flags & PARSER_PRESERVE_WHITESPACE))
	e->children.back()->content+=' ';
      e->children.back()->content+=content;
    }
    else e->add(new Element("", content));
  }
}

//--------------------------------------------------------------------------
// Read a reference (CharRef or EntityRef)
// Assumes & has been read.  Allows &#nn; &#xXX; and &ent;
// text is string to add to (note can be multiple chars if UTF8)
void Parser::read_ref(string& text, istream &s) throw (ParseFailed)
{
  xmlchar c=0;
  s.get(c);
  if (c=='#') //CharRef
  {
    int n;
    s.get(c);
    //Turn off skipws while reading numbers
    // - would like to use s >> noskipws, but GNU 2.95 series broken
    s.unsetf(ios::skipws);

    if (c=='x')
    {
      c=0;
      s >> hex >> n >> c; 
    }
    else
    {
      s.unget();
      c=0;
      s >> n >> c; 
    }
    s.setf(ios::skipws);

    // c must now be ';', otherwise it either couldn't read a number
    // or didn't end with ;
    if (c!=';') fatal("Malformed character reference");

    //Some subtleties here...  This parser ignores the encoding
    //specified in the prolog, and hence is only really conformant for
    //documents with the default encoding, UTF8.  That's enough for me!

    //However, it causes some work here: character references are in
    //the document character set (Unicode), NOT the encoding - hence
    //to make this consistent, we have to expand the Unicode value to
    //UTF8 here

    // 7-bit is simple
    if (n < 0x80)
      text += (xmlchar)n;
    else 
    {
      // 8-bit and above depends on the length
      // NB we only handle UCS2 here, so can only go to 3 bytes
      if (n < 0x800)
	text += (xmlchar)(0xC0 | (n >> 6));
      else
      {
	text += (xmlchar)(0xE0 | (n >> 12));
	text += (xmlchar)(0x80 | ((n >> 6) & 0x3f));
      }

      text += (xmlchar)(0x80 | (n & 0x3f));
    }
  }
  else if (isalpha(c)) //Entity name - alphabetic only
  {
    string ent = read_rest_of_name(c, s);
    //Must have ; terminator
    if (c!=';') fatal("Malformed entity reference");

    //Not many options, simple tests will do
    //(Always thought that switch(ent) should be allowed here!)
    if (ent=="lt")
      text+='<';
    else if (ent=="gt")
      text+='>';
    else if (ent=="amp")
      text+='&';
    else if (ent=="apos")
      text+='\'';
    else if (ent=="quot")
      text+='"';
    else
      fatal("Unrecognised entity name");
  }
  else 
  {
    // Check for leniency here - if so, keep it as character
    // data, like SGML would
    if (flags & PARSER_BE_LENIENT)
    {
      text += '&';  // Push the original '&' into the text
      s.unget();    // Hand the next char back to content reader
    }
    else fatal("Weird reference - unescaped '&'?");
  }
}

//--------------------------------------------------------------------------
// Read the rest of a name (while name chars)
// c is initially first character read
// Returns name, modifies 'c' to first non-name character read
string Parser::read_rest_of_name(xmlchar& c, istream& s)
{
  string name;
  name+=c;

  //Read rest of name
  for(;;)
  {
    c=0;
    s.get(c);
    if (is_name_char(c))
      name+=c;
    else
      break;
  }

  return name;
}

//--------------------------------------------------------------------------
// Read to end of !DOCTYPE etc - just look for '>'
void Parser::skip_to_gt(istream &s) throw (ParseFailed)
{
  for(;;)
  {
    xmlchar c=0;
    s.get(c);
    if (c == '>') break;
    else if (c == '\n') line++;
    else if (!c) fatal("Unexpected end-of-file");
  }
}

//--------------------------------------------------------------------------
// Read to end of comment - look for -->
void Parser::skip_comment(istream &s) throw (ParseFailed)
{
  for(;;)
  {
    xmlchar c=0;
    s.get(c);
    if (c=='-')
    {
      c=0; // Beware non-touch on EOF
      s.get(c);
      if (c=='-')
      {
	c=0;
	s.get(c);
	if (c=='>') break;
	else if (c == '\n') line++;
      }
      else if (c == '\n') line++;
    }
    else if (c == '\n') line++;
    else if (!c) fatal("Unexpected end-of-file in comment");
  }
}

//--------------------------------------------------------------------------
// Read to end of PI - look for ?>
void Parser::skip_pi(istream &s) throw (ParseFailed)
{
  for(;;)
  {
    xmlchar c=0;
    s.get(c);
    if (c=='?')
    {
      c=0;
      s.get(c);
      if (c == '\n') line++;
      if (c=='>') break;
    }

    if (!c) fatal("Unexpected end-of-file in PI");
  }
}

//--------------------------------------------------------------------------
// Log an error, but continue
void Parser::error(const string& s)
{
  errors++;
  serr << "XML Error: " << s << " at line " << line << endl;
}


//--------------------------------------------------------------------------
// Log a fatal error and throw exception
void Parser::fatal(const string& s) throw (ParseFailed)
{
  errors++;
  serr << "XML Fatal Error: " << s << " at line " << line << endl;
  throw ParseFailed();
}

//------------------------------------------------------------------------
// Get root element
// Returns Element::none if not valid
Element& Parser::get_root() 
{
  if (root) 
    return *root;
  else
    return Element::none;
}

//------------------------------------------------------------------------
// Get root element detached from parser, so you can keep it after the
// parser has died
// Returns 0 if not valid
Element *Parser::detach_root()
{
  if (root)
  {
    Element *r = root;
    root = 0;
    return r;
  }
  else return 0;
}

//------------------------------------------------------------------------
// Do initial processing on an element (after attributes read)
// Handles namespace map building - actual processing of namespace is
// done in final_processing
void Parser::initial_processing(Element *e)
{
  if (flags & PARSER_FIX_NAMESPACES)
  {
    // Open a new nsmap level - copy current and push to back of ns_maps
    if (ns_maps.size())
      ns_maps.push_back(ns_maps.back());
    else
      ns_maps.resize(1);

    //Look for xmlns:* attributes and add to current map
    for(map<string,string>::const_iterator p=e->attrs.begin();
	p!=e->attrs.end();
	p++)
    {
      const string& aname=p->first;

      //Begins with xmlns?
      //GCC2.95 bug - string::compare parameters are nonstandard - use probably
      //inefficient substring compare
      if (string(aname, 0, 5)=="xmlns")
      {
	//Only xmlns alone?  Default namespace
	if (aname.size()==5)
	{
	  //Store as empty prefix (default)
	  ns_maps.back()[""]=p->second;
	}
	//xmlns:prefix?
	else if (aname[5]==':')
	{
	  string prefix(aname,6);

	  //Store with prefix
	  ns_maps.back()[prefix]=p->second;
	}
	//Otherwise, ignore it
      }
    }
  }
}

//------------------------------------------------------------------------
// Do final processing on an element
void Parser::final_processing(Element *e)
{
  //If we've been asked to, 'optimise' single content sub-elements
  if (flags & PARSER_OPTIMISE_CONTENT)
    e->optimise();

  if ((flags & PARSER_FIX_NAMESPACES)
    && ns_maps.size())  // Should always be OK, but be safe
  {
    //Check current element name in latest ns_map and see if we need to 
    //translate it - including default namespace
    substitute_name(e->name, true);

    //Substitute attributes as well - non-obvious because we can't just
    //replace key names, and it's a hassle to manipulate maps in place.
    //For simplicity, but not speed, we take a copy of the map and copy
    //them all across
    map<string,string> old_attrs(e->attrs);
    e->attrs.clear();

    //Run through all the old ones and copy them back, optionally
    //modified
    for(map<string,string>::iterator p=old_attrs.begin();
	p!=old_attrs.end();
        p++)
    {
      string name=p->first;

      //Subsitute without defaulting 
      substitute_name(name);

      e->attrs[name]=p->second;
    }

    //Chop current level off ns_map stack
    ns_maps.pop_back();
  }
}

//------------------------------------------------------------------------
//Substitute a name (element or attribute) according to the current
//namespace map (assumed non empty)
//Replaces the name in place if necessary
//Only substitutes default namespaces if usedef is true - set for elements
//not for attributes
//Returns whether any subsititution done
void Parser::substitute_name(string& name, bool usedef)
{
  map<string,string>& topmap = ns_maps.back();
  string prefix;
    
  //Look for a prefix
  string::size_type pos=name.find(':');
  if (pos!=string::npos)
    prefix.assign(name, 0, pos++);
  else
  {
    if (!usedef) return;  //Stop if default not allowed

    //Failing that, look for "" as a default...
    //... which prefix is conveniently already set to!
    pos=0;  //Grab whole of name for 'rest' later
  }

  //Lookup this prefix in latest map
  if (topmap.find(prefix)!=topmap.end())
  {
    string rest(name,pos);
    string& nsname=topmap[prefix];

    //Look up nsname in user_ns_map and substitute prefix
    if (user_ns_map.find(nsname)!=user_ns_map.end())
    {
      string& newprefix=user_ns_map[nsname];

      //Rebuild name in place
      name=newprefix;
      name += ':';
      name += rest;
    }
  }
}

//------------------------------------------------------------------------
// Add namespace to prefix mapping
// Any prefix used for this namespace in the document will be replaced
// for the given prefix.  This allows you to normalise the tags you
// check for
void Parser::fix_namespace(const char *name, const char *prefix)
{
  user_ns_map[name]=prefix;
  flags |= PARSER_FIX_NAMESPACES;
}

//--------------------------------------------------------------------------
// Destructor
// Destroys root
Parser::~Parser()
{
  if (root) delete root;
}

//==========================================================================
//Stream functions

//--------------------------------------------------------------------------
// Parse from given stream
void Parser::read_from(istream &s) throw (ParseFailed)
{
  parse_stream(s);
}

//--------------------------------------------------------------------------
// Parse from given string
void Parser::read_from(const string &s) throw (ParseFailed)
{
  istringstream iss(s);
  parse_stream(iss);
}

//------------------------------------------------------------------------
// >> operator to read from istream
istream& ObTools::XML::operator>>(istream& s, Parser& p) throw (ParseFailed)
{ 
  p.read_from(s);
  return s;
}



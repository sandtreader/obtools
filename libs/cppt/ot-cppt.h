//==========================================================================
// ObTools::CPPT: ot-cppt.h
//
// Public definitions for C++ template processor
// 
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#ifndef __OBTOOLS_CPPT_H
#define __OBTOOLS_CPPT_H

#include <iostream>
#include <string>
#include <list>

namespace ObTools { namespace CPPT {

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Token state
enum TokenState
{
  TOKEN_READING,          // Waiting for more input
  TOKEN_VALID,            // Input derived a token
  TOKEN_INVALID           // Input derived a non-token 
};

//==========================================================================
// Token recogniser 
class TokenRecogniser
{
private:
  list<string> tokens;
  unsigned int index;
  unsigned int longest_valid;
  string current_token;

public:
  //------------------------------------------------------------------------
  //Constructor
  TokenRecogniser(): index(0), longest_valid(0) {}

  //------------------------------------------------------------------------
  //Add a token - empty strings are silently ignored
  void add_token(const string& tok) { if (tok.size()) tokens.push_back(tok); }

  //------------------------------------------------------------------------
  //Process a character
  //Returns whether character was used and kept in current_token
  //State of token validity also returned
  bool process_char(char c, TokenState& state);

  //------------------------------------------------------------------------
  //Get current token or invalid input
  string get_token() { return current_token; }
};

//==========================================================================
// Internal processor state
enum ProcessorState
{
  PS_NORMAL,  // Passing through text
  PS_CODE,    // In C++ code
  PS_EXPR,    // In C++ expr
  PS_COMMENT  // In template comment 
};

//==========================================================================
// Tags list
struct Tags
{
  string start_code;
  string end_code;
  string start_expr;
  string end_expr;
  string start_comment;
  string end_comment;
};

//==========================================================================
// Template processor class
class Processor
{
private:
  istream &sin;
  ostream &sout;
  const char *sname;
  Tags tags;

  ProcessorState state;
  bool started_text;

  //Individual token recognisers for each state
  TokenRecogniser tr_normal;
  TokenRecogniser tr_code;
  TokenRecogniser tr_expr;
  TokenRecogniser tr_comment;

  void output_text(char c);
  void output_text(const char *s);
  void open_code();
  void open_expr();
  void close_code();
  void close_expr();
  void strip_eol();

public:
  //------------------------------------------------------------------------
  //Constructor
  //streamname is the name of the stream for boilerplate text output
  Processor(istream& instream, ostream& outstream,
	    const Tags& ts,
	    const char *streamname="cout");

  //------------------------------------------------------------------------
  //Process the instream into the outstream until EOF
  void process();
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_CPPT_H




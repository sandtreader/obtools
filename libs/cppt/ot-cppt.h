//==========================================================================
// ObTools::CPPT: cppt.h
//
// Public definitions for C++ template processor
// 
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#ifndef __OBTOOLS_CPPT_H
#define __OBTOOLS_CPPT_H

#include <iostream>
using namespace std;

namespace ObTools { namespace CPPT {

//==========================================================================
// Internal processor state
enum ProcessorState
{
  PS_NORMAL,  // Passing through text
  PS_BLOCK,   // In C++ block code (inside <? ?>)
  PS_EXPR     // In C++ expr code (inside <?= ?>)
};

//==========================================================================
// Template processor class
class Processor
{
private:
  istream &sin;
  ostream &sout;
  const char *sname;

  ProcessorState state;
  bool started_text;

  void text_char(char c);
  void open_block();
  void open_expr();
  void close_block();
  void close_expr();
  void strip_eol();

public:
  //------------------------------------------------------------------------
  //Constructor
  //streamname is the name of the stream for boilerplate text output
  Processor(istream& instream, ostream& outstream,
	    const char *streamname="cout"):
    sin(instream),
    sout(outstream),
    sname(streamname)
  {}

  //------------------------------------------------------------------------
  //Process the instream into the outstream until EOF
  void process();
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_CPPT_H




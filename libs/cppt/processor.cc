//==========================================================================
// ObTools::CPPT: processor.cc
//
// C++ template processor
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "cppt.h"
using namespace ObTools::CPPT;

//--------------------------------------------------------------------------
// Output a text character to the given outstream, taking care of wrapping
// in C++ output
void Processor::text_char(char c)
{
  // If not in a stream already, start one
  if (!started_text)
  {
    // If newline, just output endl and leave stream closed
    if (c=='\n')
      sout << "  " << sname << " << endl;\n";
    else
    {
      // Open stream
      sout << "  " << sname << " << \"";
      started_text = true;
    }
  }

  // Check again that above started stream (not a newline)
  if (started_text)
  {
    switch (c)
    {
      case '\n':
	sout << "\\n\";" << endl;
	started_text = false;
	break;

      case '"':  // Back-slashify quotes
	sout << "\\\"";
	break;

      default:
	sout << c;
    }
  }
}

//--------------------------------------------------------------------------
// Close any open text out and open a C++ block
void Processor::open_block()
{
  if (started_text)
    sout << "\";" << endl;
}

//--------------------------------------------------------------------------
// Close any open text out and open a C++ expr
void Processor::open_expr()
{
  if (started_text)
    sout << "\" << ";
}

//--------------------------------------------------------------------------
// Close C++ block
void Processor::close_block()
{
  //Just wait for more text
  started_text = false;
}

//--------------------------------------------------------------------------
// Close C++ expr
void Processor::close_expr()
{
  // If text already open, keep it on same line for readability
  if (started_text)
    sout << " << \"";
}

//--------------------------------------------------------------------------
// Strip trailing whitespace and subsequent newline
void Processor::strip_eol()
{
  for(;;)
  {
    char c=0;
    sin.get(c);
    if (c!=' ' && c!='\t')
    {
      if (c!='\n') sin.unget();
      return;
    }
  }
}

//--------------------------------------------------------------------------
// Process an instream into an outstream
void Processor::process()
{
  state=PS_NORMAL;
  started_text=false;

  for(;;)
  {
    char c=0;
    sin.get(c);
    if (!c) break;

    switch (state)
    {
      case PS_NORMAL:
	// Look for <? or <?=
	if (c=='<')
	{
	  c=0;
	  sin.get(c);
	  if (c=='?')
	  {
	    c=0;
	    sin.get(c);
	    if (c=='=')
	    {
	      open_expr();
	      state = PS_EXPR;
	    }
	    else
	    {
	      open_block();
	      state = PS_BLOCK;
	      sin.unget();
	      strip_eol();
	    }
	  }
	  else 
	  {
	    //Pass < through as if normal
	    text_char('<');
	    sin.unget();
	  }
	}
	else text_char(c);
	break;

      case PS_BLOCK:
      case PS_EXPR:
	// Look for ?>
	if (c=='?')
	{
	  c=0;
	  sin.get(c);
	  if (c=='>')
	  {
	    // Close block/expr
	    if (state==PS_BLOCK)
	    {
	      close_block();
	      strip_eol();
	    }
	    else
	      close_expr();

	    state = PS_NORMAL;
	  }
	  else
	  {
	    //Pass ? through as if normal
	    sout << '?';
	    sin.unget();
	  }
	}
	else sout << c;
	break;
    }
  }

  //In case it stops unexpectedly in the middle of text (no final newline),
  //force return to C++ block to close out the text
  if (state == PS_NORMAL) open_block();
}


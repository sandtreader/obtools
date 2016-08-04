//==========================================================================
// ObTools::CPPT: processor.cc
//
// C++ template processor
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-cppt.h"
using namespace ObTools::CPPT;

//--------------------------------------------------------------------------
//Constructor
Processor::Processor(istream& instream, ostream& outstream,
                     const Tags& ts, const string& streamname):
    sin(instream),
    sout(outstream),
    sname(streamname),
    tags(ts)
{
  // Put start tags in normal TR
  tr_normal.add_token(tags.start_code);
  tr_normal.add_token(tags.start_expr);
  tr_normal.add_token(tags.start_comment);

  // Split end tags into individual state TR's
  tr_code.add_token(tags.end_code);
  tr_expr.add_token(tags.end_expr);
  tr_comment.add_token(tags.end_comment);
}

//--------------------------------------------------------------------------
// Output a text character to the given outstream, taking care of wrapping
// in C++ output
void Processor::output_text(char c)
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

      case '"':  // Back-slashify quotes and backslashes
      case '\\':
        sout << "\\" << c;
        break;

      default:
        sout << c;
    }
  }
}

//--------------------------------------------------------------------------
// Ditto for entire string
void Processor::output_text(const char *s)
{
  while (*s) output_text(*s++);
}

//--------------------------------------------------------------------------
// Close any open text out and open a C++ block
void Processor::open_code()
{
  if (started_text)
    sout << "\";" << endl;
}

//--------------------------------------------------------------------------
// Close any open text out and open a C++ expr
void Processor::open_expr()
{
  if (started_text)
    sout << "\" << (";
  else
  {
    //Start one, in expression state
    sout << "  " << sname << " << (";
    started_text = true;
  }
}

//--------------------------------------------------------------------------
// Close C++ block
void Processor::close_code()
{
  //Just wait for more text
  started_text = false;
}

//--------------------------------------------------------------------------
// Close C++ expr
void Processor::close_expr()
{
  sout << ")";

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

  retry:
    TokenState tokstate;
    bool used_char = false;

    switch (state)
    {
      case PS_NORMAL:
        // Pass to normal TR
        used_char = tr_normal.process_char(c, tokstate);

        // Check for new tokens
        switch (tokstate)
        {
          case TOKEN_READING:
            // Output this character as text if not used
            if (!used_char) output_text(c);
            break;

          case TOKEN_VALID:
          {
            string token = tr_normal.get_token();
            if (token == tags.start_code)
            {
              open_code();
              state = PS_CODE;
              strip_eol();
            }
            else if (token == tags.start_expr)
            {
              open_expr();
              state = PS_EXPR;
            }
            else if (token == tags.start_comment)
            {
              state = PS_COMMENT;
            }

            // Retry this character if not used
            if (!used_char) goto retry;
            break;
          }

          case TOKEN_INVALID:
            // Output unwanted characters as text, and retry
            output_text(tr_normal.get_token().c_str());
            if (!used_char) goto retry;
            break;
        }
        break;

        case PS_CODE:
          // Pass to code TR
          used_char = tr_code.process_char(c, tokstate);

          // Check for new tokens
          switch (tokstate)
          {
            case TOKEN_READING:
              // Output this character verbatim if not used
              if (!used_char) sout << c;
              break;

            case TOKEN_VALID:
            {
              string token = tr_code.get_token();
              if (token == tags.end_code)
              {
                close_code();
                strip_eol();
                state = PS_NORMAL;
              }
            }

            // Retry this character if not used
            if (!used_char) goto retry;
            break;

            case TOKEN_INVALID:
              // Pass mistaken token through as normal, retry unused
              sout << tr_code.get_token();
              if (!used_char) goto retry;
              break;
          }

          break;

        case PS_EXPR:
          // Pass to expr TR
          used_char = tr_expr.process_char(c, tokstate);

          // Check for new tokens
          switch (tokstate)
          {
            case TOKEN_READING:
              // Output this character verbatim if not used
              if (!used_char) sout << c;
              break;

            case TOKEN_VALID:
            {
              string token = tr_expr.get_token();
              if (token == tags.end_expr)
              {
                close_expr();
                state = PS_NORMAL;
              }
            }
            // Retry this character if not used
            if (!used_char) goto retry;
            break;

            case TOKEN_INVALID:
              // Pass mistaken token through verbatim, retry unused
              sout << tr_expr.get_token();
              if (!used_char) goto retry;
              break;
          }
          break;

        case PS_COMMENT:
          // Pass to comment TR
          used_char = tr_comment.process_char(c, tokstate);

          // Check for new tokens
          switch (tokstate)
          {
            case TOKEN_READING:
              // Swallow it
              break;

            case TOKEN_VALID:
            {
              string token = tr_comment.get_token();
              if (token == tags.end_comment)
              {
                state = PS_NORMAL;
                strip_eol();
              }
            }
            if (!used_char) goto retry;
            break;

            case TOKEN_INVALID:
              // Swallow mistake, but retry unused
              if (!used_char) goto retry;
              break;
          }
          break;
    }

  }

  //In case it stops unexpectedly in the middle of text (no final newline),
  //force return to C++ code to close out the text
  if (state == PS_NORMAL) open_code();
}


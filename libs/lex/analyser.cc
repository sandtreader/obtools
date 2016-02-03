//==========================================================================
// ObTools::Lex: analyser.cc
//
// Simple lexical analyser / tokeniser
//
// Copyright (c) 2016 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-lex.h"
#include "ot-text.h"

namespace ObTools { namespace Lex {

//------------------------------------------------------------------------
// Read a token from the input
Token Analyser::read_token() throw (Exception)
{
  if (pending_token.type != Token::UNKNOWN)
  {
    Token result = pending_token;
    pending_token = Token();
    return result;
  }

  if (!input) return Token(Token::END);
  char c = get();

  // Ignore whitespace
  while (isspace(c)) c=get();

  // End of input
  if (!c) return Token(Token::END);

  // Name tokens
  if (isalpha(c))
  {
    // Read a name
    string name;
    name += c;
    for(;;)
    {
      c = peek();
      if (isalpha(c) || isdigit(c) || c=='_')
      {
        c = get();
        name += c;
      }
      else break;
    }

    return Token(Token::NAME, name);
  }

  // Number tokens
  else if (isdigit(c))
  {
    // Read a string value
    string value;
    value += c;

    // Integer part
    for(;;)
    {
      c = peek();
      if (isdigit(c))
      {
        c = get();
        value += c;
      }
      else break;
    }

    // Optional decimal part
    if (c == '.')
    {
      c = get();
      value += c;
      for(;;)
      {
        c = peek();
        if (isdigit(c))
        {
          c = get();
          value += c;
        }
        else break;
      }
    }

    // Optional exponential
    if (c == 'e' || c == 'E')
    {
      c = get();
      value += c;

      c = get();
      if (!c) throw Exception("End of input in number exponent");

      // Optional +/-
      if (c == '+' || c == '-')
      {
        value += c;
        c = get();
        if (!c) throw Exception("End of input in number exponent");
      }

      if (isdigit(c))
      {
        value += c;
        for(;;)
        {
          c = peek();
          if (isdigit(c))
          {
            c = get();
            value += c;
          }
          else break;
        }
      }
      else throw Exception("Bad character in number exponent");
    }

    return Token(Token::NUMBER, value);
  }

  // Strings
  else if (c=='"')
  {
    string value;
    for(;;)
    {
      c = get();
      switch (c)
      {
        case 0: throw Exception("End of input in string");

        case '\"': return Token(Token::STRING, value);

        case '\\':
          c = get();
          switch (c)
          {
            case 0: throw Exception("End of input in escape");

            // Passed through
            case '\\': break;
            case '"': break;

            // Whitespace single-letter escapes
            case 'b': c='\b'; break;
            case 'f': c='\f'; break;
            case 'n': c='\n'; break;
            case 'r': c='\r'; break;
            case 't': c='\t'; break;

            // \uABCD unicode
            case 'u':
            {
              string hex;
              for(int i=0; i<4; i++)
              {
                c = get();
                if (!c) throw Exception("End of input in \\u escape");
                hex += c;
              }

              Text::UTF8::append(value, Text::xtoi(hex));
              c=0;   // block from add below
            }
            break;

            default:
              throw Exception(string("Unrecognised string escape '")+c+"'");
          }
          // Falling

        default:
          if (c) value += c;  // Can be dropped in Unicode parse above
      }
    }
  }

  // Symbols
  else
  {
    string symbol;
    symbol += c;

    for(int len=1; ;len++)
    {
      bool found = false;

      // Look up as prefix in symbol list
      for(list<string>::const_iterator p=symbols.begin();
          !found && p!=symbols.end(); ++p)
      {
        const string& candidate = *p;

        // Look for prefix
        if (!candidate.compare(0, len, symbol))
          found = true;
      }

      // If we found it, consume and add this character, and peek for another
      if (found)
      {
        if (len>1) get();  // Consume the peeked one, first is get() already
        symbol += peek();
      }
      else
      {
        // If not found and symbol is only one character, fail
        if (len == 1)
          throw Exception(string("Unrecognised token near '")+c+"'");
        else
        {
          // Strip off last character and use that
          return Token(Token::SYMBOL, string(symbol, 0, len-1));
        }
      }
    }
  }
}


}} // namespaces

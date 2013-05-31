//==========================================================================
// ObTools::Expression: token.cc
//
// Simple hard-wired expression tokeniser
//
// Copyright (c) 2013 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-expr.h"
#include "ot-text.h"

namespace ObTools { namespace Expression {

//------------------------------------------------------------------------
// Read a token from the input
Token Tokeniser::read_token()
{
  char c = getc();

  // Ignore whitespace
  while (isspace(c)) c=getc();

  // Name tokens
  if (isalpha(c))
  {
    // Read a name
    string name;
    name += c;
    for(;;)
    {
      c = getc();
      if (isalpha(c) || isdigit(c) || c=='_')
        name += c;
      else
        break;
    }

    rewind();
    return Token(Token::NAME, name);
  }

  // Number tokens
  else if (isdigit(c))
  {
    // Read a string value, then convert
    string value;
    value += c;

    // Integer part
    for(;;)
    {
      c = getc();
      if (isdigit(c))
        value += c;
      else
        break;
    }

    // Optional decimal part
    if (c == '.')
    {
      value += c;
      for(;;)
      {
        c = getc();
        if (isdigit(c))
          value += c;
        else
          break;
      }
    }

    // Maybe exponential later?!!!

    rewind();
    return Token(Token::NUMBER, Text::stof(value));
  }

  // Operator / EOT
  else switch(c)
  {
    // End of text
    case 0: return Token(Token::EOT);

    // Simple one-character cases
    case '+': return Token(Token::PLUS);
    case '-': return Token(Token::MINUS);
    case '*': return Token(Token::MUL);
    case '/': return Token(Token::DIV);
    case '(': return Token(Token::LPAR);
    case ')': return Token(Token::RPAR);

    // One or two character equivalents - &/&& |/|| =/==
    case '&':
      if (getc()!=c) rewind();
      return Token(Token::AND);

    case '|':
      if (getc()!=c) rewind();
      return Token(Token::OR);

    case '=':
      if (getc()!=c) rewind();
      return Token(Token::EQ);

    // Comparators - < <= > >= <> !=
    case '<':
      switch (getc())
      {
        case '=': return Token(Token::LTEQ);
        case '>': return Token(Token::NE);
        default: rewind(); return Token(Token::LT);
      }

    case '>':
      switch (getc())
      {
        case '=': return Token(Token::GTEQ);
        default: rewind(); return Token(Token::GT);
      }

    case '!':
      if (getc() == '=') return Token(Token::NE);
      // Falling if not!

    // Unrecognised
    default:
      serr << "Unrecognised token near '" << c << "'\n";
      return Token(Token::UNKNOWN);
  }
}


}} // namespaces

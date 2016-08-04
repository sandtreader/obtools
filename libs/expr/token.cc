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

//--------------------------------------------------------------------------
// Read a token from the input
Token Tokeniser::read_token() throw (Exception)
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
      c = peekc();
      if (isalpha(c) || isdigit(c) || c=='_')
      {
        name += c;
        step();
      }
      else break;
    }

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
      c = peekc();
      if (isdigit(c))
      {
        value += c;
        step();
      }
      else break;
    }

    // Optional decimal part
    if (c == '.')
    {
      step();
      value += c;
      for(;;)
      {
        c = peekc();
        if (isdigit(c))
        {
          value += c;
          step();
        }
        else break;
      }
    }

    // Maybe exponential later?!!!

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

    // Doubled character cases - && || ==
    case '&':
      if (peekc()!=c) throw Exception("Single (bitwise) & not implemented");
      step();
      return Token(Token::AND);

    case '|':
      if (peekc()!=c) throw Exception("Single (bitwise) | not implemented");
      step();
      return Token(Token::OR);

    case '=':
      if (peekc()!=c) throw Exception("Assignment not implemented - use ==");
      step();
      return Token(Token::EQ);

    // Comparators - < <= > >= !=
    case '<':
      switch (peekc())
      {
        case '=': step(); return Token(Token::LTEQ);
        default: return Token(Token::LT);
      }

    case '>':
      switch (peekc())
      {
        case '=': step(); return Token(Token::GTEQ);
        default: return Token(Token::GT);
      }

    case '!':
      switch (peekc())
      {
        case '=': step(); return Token(Token::NE);
        default: return Token(Token::NOT);
      }

    // Unrecognised
    default:
      throw Exception(string("Unrecognised token near '")+c+"'");
  }
}


}} // namespaces

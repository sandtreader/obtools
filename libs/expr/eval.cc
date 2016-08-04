//==========================================================================
// ObTools::Expression: eval.cc
//
// Simple hard-wired expression evaluator
//
// Copyright (c) 2013 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-expr.h"

namespace ObTools { namespace Expression {

//--------------------------------------------------------------------------
// Get value for a name in the expression
// By default just errors and returns 0 - override if variables required
double Evaluator::get_value_for_name(const string& name) throw (Exception)
{
  throw Exception("Variable name lookup for '" + name + "' not implemented");
  return 0;
}

//--------------------------------------------------------------------------
// Get next token
void Evaluator::next() throw (Exception)
{
  token = tokeniser.read_token();
}

//--------------------------------------------------------------------------
// Read a factor
double Evaluator::read_factor() throw (Exception)
{
  switch (token.type)
  {
    case Token::MINUS:  // Unary minus
      next();
      return -read_factor();

    case Token::NOT:    // Unary not
      next();
      return read_factor()?0:1;

    case Token::NUMBER:
    {
      double v = token.value;
      next();
      return v;
    }

    case Token::NAME:
    {
      double v = get_value_for_name(token.name);
      next();
      return v;
    }

    case Token::LPAR:
    {
      next();
      double v = read_expression();
      if (token.type == Token::RPAR)
        next();
      else
        throw Exception("Mismatched parentheses");
      return v;
    }

    case Token::EOT:
      throw Exception("Unexpected end");
      return 0;

    default:
      throw Exception("Unrecognised token");
      return 0;
  }
}

//--------------------------------------------------------------------------
// Read a term
double Evaluator::read_term() throw (Exception)
{
  double v = read_factor();
  for(;;)
  {
    switch (token.type)
    {
      case Token::MUL:
      {
        next();
        v *= read_factor();
      }
      break;

      case Token::DIV:
      {
        next();
        v /= read_factor();
      }
      break;

      default: return v;
    }
  }
}

//--------------------------------------------------------------------------
// Read a side (of conditional)
double Evaluator::read_side() throw (Exception)
{
  double v = read_term();
  for(;;)
  {
    switch (token.type)
    {
      case Token::PLUS:
      {
        next();
        v += read_term();
      }
      break;

      case Token::MINUS:
      {
        next();
        v -= read_term();
      }
      break;

      default: return v;
    }
  }
}

//--------------------------------------------------------------------------
// Read a predicate (comparison)
double Evaluator::read_predicate() throw (Exception)
{
  double v = read_side();

  switch (token.type)
  {
    case Token::EQ:
      next();
      return (v == read_side())?1:0;

    case Token::NE:
      next();
      return (v != read_side())?1:0;

    case Token::LT:
      next();
      return (v < read_side())?1:0;

    case Token::GT:
      next();
      return (v > read_side())?1:0;

    case Token::LTEQ:
      next();
      return (v <= read_side())?1:0;

    case Token::GTEQ:
      next();
      return (v >= read_side())?1:0;

    default: return v;
  }
}

//--------------------------------------------------------------------------
// Read an expression
double Evaluator::read_expression() throw (Exception)
{
  double v = read_predicate();
  for(;;)
  {
    switch (token.type)
    {
      case Token::AND:
      {
        next();
        double w = read_predicate();  // Avoid lazy evaluation
        v = (v && w)?1:0;
      }
      break;

      case Token::OR:
      {
        next();
        double w = read_predicate();  // Avoid lazy evaluation
        v = (v || w)?1:0;
      }
      break;

      default: return v;
    }
  }
}

//--------------------------------------------------------------------------
// Evaluate an expression
double Evaluator::evaluate(const string& expr) throw (Exception)
{
  tokeniser.reset(expr);
  next();

  double v = read_expression();

  // Must be EOF now
  if (token.type != Token::EOT)
    throw Exception("Parse error");

  return v;
}


}} // namespaces

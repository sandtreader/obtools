//==========================================================================
// ObTools::JSON: parser.cc
//
// JSON parser
//
// Copyright (c) 2016 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-json.h"
#include <math.h>

namespace ObTools { namespace JSON {

//--------------------------------------------------------------------------
// Constructor on an istream
Parser::Parser(istream& input): lex(input)
{
  // Add symbols
  lex.add_symbol("{");
  lex.add_symbol("}");
  lex.add_symbol(":");
  lex.add_symbol(",");
  lex.add_symbol("[");
  lex.add_symbol("]");
}

//--------------------------------------------------------------------------
// Read the rest of an object (after reading the {)
Value Parser::read_rest_of_object()
{
  // Read an object
  Value object(Value::OBJECT);

  // Read properties
  for(;;)
  {
    Lex::Token token = lex.read_token();
    if (token.type == Lex::Token::SYMBOL
        && token.value == "}")
      break;
    else if (token.type == Lex::Token::NAME
             || token.type == Lex::Token::STRING)
    {
      string name = token.value;
      token = lex.read_token();

      if (token.type != Lex::Token::SYMBOL
          || token.value != ":")
        throw Exception("Expected :");

      // Recurse for value
      Value value = read_value();
      object.o[name] = value;
    }
    else throw Exception(string("Bad property name ")+token.value);

    // The next symbol must be , or }
    token = lex.read_token();
    if (token.type == Lex::Token::SYMBOL)
    {
      if (token.value == "}")
        break;
      else if (token.value != ",")
        throw Exception("Expected , or }");
    }
    else throw Exception("Expected , or }");
  }

  return object;
}

//--------------------------------------------------------------------------
// Read the rest of an array (after reading the [)
Value Parser::read_rest_of_array()
{
  // Read an array
  Value array(Value::ARRAY);

  // Read elements
  for(;;)
  {
    Lex::Token token = lex.read_token();
    if (token.type == Lex::Token::SYMBOL
        && token.value == "]")
      break;

    // Put the token back for value
    lex.put_back(token);

    // Recurse for value
    Value value = read_value();
    array.a.push_back(value);

    // The next symbol must be , or ]
    token = lex.read_token();
    if (token.type == Lex::Token::SYMBOL)
    {
      if (token.value == "]")
        break;
      else if (token.value != ",")
        throw Exception("Expected , or ]");
    }
    else throw Exception("Expected , or ]");
  }

  return array;
}


//--------------------------------------------------------------------------
// Read a value
Value Parser::read_value()
{
  try
  {
    Lex::Token token = lex.read_token();
    switch (token.type)
    {
      case Lex::Token::END:
        return Value(Value::NULL_);

      case Lex::Token::NUMBER:
        if (token.value.find('.') != string::npos)
          return Value(Value::NUMBER, Text::stof(token.value));
        else
          return Value(Text::stoi64(token.value));

      case Lex::Token::STRING:
        return Value(token.value);

      case Lex::Token::NAME:
        if (token.value == "null")
          return Value(Value::NULL_);
        else if (token.value == "true")
          return Value(Value::TRUE);
        else if (token.value == "false")
          return Value(Value::FALSE);
        else
          throw Exception(string("Unrecognised bare name ")+token.value);

      case Lex::Token::SYMBOL:
        if (token.value == "{")
          return read_rest_of_object();
        else if (token.value == "[")
          return read_rest_of_array();
        else
          throw Exception(string("Misplaced symbol ")+token.value);

      default:
        throw Exception("Unrecognised token ");
    }
  }
  catch (Lex::Exception e)
  {
    throw Exception(e.error);
  }
}


}} // namespaces

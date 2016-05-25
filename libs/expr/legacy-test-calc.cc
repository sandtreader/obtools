//==========================================================================
// ObTools::Expression: test-calc.cc
//
// Test calculator utility
//
// Copyright (c) 2013 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-expr.h"

using namespace std;
using namespace ObTools;

int main()
{
  Expression::Evaluator evaluator;
  for(;;)
  {
    char expr[1000];
    cin.getline(expr, 1000);
    if (!expr[0]) continue;

    try
    {
      double result = evaluator.evaluate(expr);
      cout << ">> " << result << endl;
    }
    catch (Expression::Exception e)
    {
      cerr << "! " << e.error << endl;
    }
  }
}

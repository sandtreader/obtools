//==========================================================================
// ObTools::Gnuplot: ot-gnuplot.h
//
// Public definitions for ObTools::Gnuplot
// Library for creating gnuplot output
//
// Copyright (c) 2016 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_GNUPLOT_H
#define __OBTOOLS_GNUPLOT_H

#include <ostream>
#include <iomanip>

namespace ObTools { namespace Gnuplot {

using namespace std;

//==========================================================================
// Output class
class Output
{
private:
  ostream& os;

public:
  //------------------------------------------------------------------------
  // Constructor
  Output(ostream& _os, const string& label):
    os(_os)
  {
    os << "set terminal png" << endl;
    os << R"(plot '-' using 1:2 title ")" << label << R"(" with lines)"
       << endl;
    os << setprecision(17);
  }

  //------------------------------------------------------------------------
  // Add a point
  void add_point(double x, double y)
  {
    os << "\t" << x << " " << y << endl;
  }

  //------------------------------------------------------------------------
  // Destructor
  ~Output()
  {
    os << "EOF" << endl;
  }
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_GNUPLOT_H

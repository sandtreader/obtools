//==========================================================================
// ObTools::Time: test-duration.cc
//
// Test harness for time library - reading and converting durations
//
// Copyright (c) 2005-2019 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-time.h"
#include "ot-text.h"
#include <sstream>
#include <iostream>

using namespace std;
using namespace ObTools;

TEST(DurationTest, TestDurationParseFromFile)
{
  // Each line is input|expected-seconds
  const char *tests = R"(
# Errors
foo|0
|0

# Basic seconds
0|0
1|1
300|300
27.45|27.45

# HMS style
1:30|90
1.5:0|90
0:90|90
1:0:0|3600
1:1:0|3660
01:02:03|3723
1:0:0:0|86400

# Units
99ns|9.9e-08
20.4us|2.04e-05
66.6ms|0.0666
1s|1
1S|1
1sec|1
5secs|5
3.9 seconds|3.9
1m|60
1 minute|60
3 mins|180
4.5 minutes|270
1H|3600
1 hour|3600
1 hr|3600
2 hours|7200
2 hrs|7200
1d|86400
1 DAY|86400
2 days|172800
1w|604800
1 week|604800
1 wk|604800
2 weeks|1209600
2 wks|1209600

# Combined units
1 minute 15 seconds|75
1M15S|75
1 hour|3600
2 hours|7200
14 hours 10 minutes 11.99 seconds|51011.99
14h10m11.99s|51011.99

# ISO durations
PT5S|5
PT5M|300
PT5M30S|330
PT1H|3600
PT1H30M|5400
PT1H30M50S|5450
P1D|86400
P1DT12H|129600
)";

  // Read a line at a time, and convert to duration
  istringstream iss(tests);
  while (!iss.eof())
  {
    string line;
    getline(iss, line);
    if (line.empty()) continue;
    if (line[0] == '#') continue;

    const vector<string> bits = Text::split(line, '|');
    if (bits.size() != 2)
      FAIL() << "Bad line [" << line << "]\n";

    const Time::Duration d(bits[0]);
    const double secs = Text::stof(bits[1]);

    EXPECT_DOUBLE_EQ(d.seconds(), secs) << " from " << bits[0] << endl;
  }
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}




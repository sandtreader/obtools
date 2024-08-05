//==========================================================================
// ObTools::Control: test-pid.cc
//
// Test harness for PID loop
//
// Copyright (c) 2024 Paul Clark
//==========================================================================

#include <gtest/gtest.h>
#include "ot-control.h"

using namespace std;
using namespace ObTools;
using namespace ObTools::Control;

TEST(PIDLoop, default_construction_sets_parameters_0)
{
  PIDLoop pid;
  EXPECT_EQ(0, pid.get_k_p());
  EXPECT_EQ(0, pid.get_k_i());
  EXPECT_EQ(0, pid.get_k_d());
  EXPECT_EQ(0, pid.get_set_point());
}


TEST(PIDLoop, explicit_construction_sets_parameters)
{
  PIDLoop pid(1, 2, 3);
  EXPECT_EQ(1.0, pid.get_k_p());
  EXPECT_EQ(2.0, pid.get_k_i());
  EXPECT_EQ(3.0, pid.get_k_d());
}

TEST(PIDLoop, parameters_set_correctly)
{
  PIDLoop pid;
  pid.set_parameters(1, 2, 3);
  EXPECT_EQ(1.0, pid.get_k_p());
  EXPECT_EQ(2.0, pid.get_k_i());
  EXPECT_EQ(3.0, pid.get_k_d());

  pid.set_k_p(10);
  EXPECT_EQ(10.0, pid.get_k_p());
  EXPECT_EQ(2.0, pid.get_k_i());
  EXPECT_EQ(3.0, pid.get_k_d());

  pid.set_k_i(20);
  EXPECT_EQ(10.0, pid.get_k_p());
  EXPECT_EQ(20.0, pid.get_k_i());
  EXPECT_EQ(3.0, pid.get_k_d());

  pid.set_k_d(30);
  EXPECT_EQ(10.0, pid.get_k_p());
  EXPECT_EQ(20.0, pid.get_k_i());
  EXPECT_EQ(30.0, pid.get_k_d());
}

TEST(PIDLoop, set_point_sets_correctly)
{
  PIDLoop pid;
  EXPECT_EQ(0.0, pid.get_set_point());

  pid.set_set_point(42);
  EXPECT_EQ(42.0, pid.get_set_point());
}

TEST(PIDLoop, proportion_only_converges)
{
  PIDLoop pid(0.5);
  pid.set_set_point(1.0);
  pid.reset();

  EXPECT_EQ(0.5,   pid.tick(0.0,  1.0));
  EXPECT_EQ(0.25,  pid.tick(0.5,  2.0));
  EXPECT_EQ(0.125, pid.tick(0.75, 3.0));
}

TEST(PIDLoop, integral_only_accumulates)
{
  PIDLoop pid(0.0, 0.1);
  pid.set_set_point(1.0);
  pid.reset();

  EXPECT_NEAR(0.1,   pid.tick(0.0,  1.0), 1e-6);
  EXPECT_NEAR(0.15,  pid.tick(0.5,  2.0), 1e-6);
  EXPECT_NEAR(0.175, pid.tick(0.75, 3.0), 1e-6);
}

TEST(PIDLoop, differential_only_spikes)
{
  PIDLoop pid(0.0, 0.0, 0.1);
  pid.set_set_point(1.0);
  pid.reset();

  EXPECT_NEAR(0.1,   pid.tick(0.0,  1.0), 1e-6);
  EXPECT_NEAR(-0.05,  pid.tick(0.5,  2.0), 1e-6);
  EXPECT_NEAR(-0.025, pid.tick(0.75, 3.0), 1e-6);
}

TEST(PIDLoop, full_pid)
{
  PIDLoop pid(0.5, 0.1, 0.1);
  pid.set_set_point(1.0);
  pid.reset();

  EXPECT_NEAR(0.5   + 0.1  + 0.1,     pid.tick(0.0,  1.0), 1e-6);  // e = 1.0
  EXPECT_NEAR(0.25  + 0.15 - 0.05,    pid.tick(0.5,  2.0), 1e-6);  // e = 0.5
  EXPECT_NEAR(0.125 + 0.175 - 0.025,  pid.tick(0.75, 3.0), 1e-6);  // e = 0.25
  EXPECT_NEAR(0     + 0.175 - 0.0125, pid.tick(1,    5.0), 1e-6);  // e = 0 (note t=5)
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

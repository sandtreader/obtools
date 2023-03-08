//==========================================================================
// ObTools::Serial: test-tty.cc
//
// Test harness for serial library
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-serial.h"
#include <stdlib.h>
#include <fcntl.h>
#include <stdlib.h>
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

TEST(SerialTest, TestTTYOpen)
{
  const auto parent_fd = posix_openpt(O_RDWR | O_NOCTTY);
  ASSERT_GE(parent_fd, 0) << "Could not open pseudoterminal";
  ASSERT_EQ(0, grantpt(parent_fd)) << "Could not grant pseudoterminal";
  ASSERT_EQ(0, unlockpt(parent_fd)) << "Could not unlock pseudoterminal";
  const auto *child_name = ptsname(parent_fd);
  ASSERT_TRUE(child_name) << "Could not get child pseudoterminal name";
  Serial::TTY tty;
  EXPECT_TRUE(tty.open(child_name));
  close(parent_fd);
}

TEST(SerialTest, TestTTYGetParameters)
{
  const auto parent_fd = posix_openpt(O_RDWR | O_NOCTTY);
  ASSERT_GE(parent_fd, 0) << "Could not open pseudoterminal";
  ASSERT_EQ(0, grantpt(parent_fd)) << "Could not grant pseudoterminal";
  ASSERT_EQ(0, unlockpt(parent_fd)) << "Could not unlock pseudoterminal";
  const auto *child_name = ptsname(parent_fd);
  ASSERT_TRUE(child_name) << "Could not get child pseudoterminal name";
  Serial::TTY tty;
  ASSERT_TRUE(tty.open(child_name));
  Serial::Parameters params;
  EXPECT_TRUE(tty.get_parameters(params));
  EXPECT_EQ(38400, params.in_speed);
  EXPECT_EQ(38400, params.out_speed);
  EXPECT_EQ(Serial::InputFlags::cr_to_nl | Serial::InputFlags::xon,
            params.in_flags);
  EXPECT_EQ(Serial::OutputFlags::post_processing |
            Serial::OutputFlags::nl_to_cr_nl,
            params.out_flags);
  EXPECT_EQ(Serial::CharFlags::char_size_8 |
            Serial::CharFlags::enable_receiver,
            params.char_flags);
  EXPECT_EQ(Serial::LocalFlags::generate_signals |
            Serial::LocalFlags::canonical_mode |
            Serial::LocalFlags::echo |
            Serial::LocalFlags::erase_char_word |
            Serial::LocalFlags::erase_line |
            Serial::LocalFlags::echo_control |
            Serial::LocalFlags::erase_line_char_by_char |
            Serial::LocalFlags::input_processing,
            params.local_flags);
  EXPECT_EQ(Serial::SpecialChars{}, params.special_chars);
  EXPECT_EQ(1, params.min_chars_for_non_canon_read);
  EXPECT_EQ(chrono::milliseconds{0}, params.non_canon_read_timeout);
  close(parent_fd);
}

TEST(SerialTest, TestTTYSetParameters)
{
  const auto parent_fd = posix_openpt(O_RDWR | O_NOCTTY);
  ASSERT_GE(parent_fd, 0) << "Could not open pseudoterminal";
  ASSERT_EQ(0, grantpt(parent_fd)) << "Could not grant pseudoterminal";
  ASSERT_EQ(0, unlockpt(parent_fd)) << "Could not unlock pseudoterminal";
  const auto *child_name = ptsname(parent_fd);
  ASSERT_TRUE(child_name) << "Could not get child pseudoterminal name";
  Serial::TTY tty;
  ASSERT_TRUE(tty.open(child_name));
  Serial::Parameters params;
  params.in_speed = 9600;
  params.out_speed = 9600;
  params.char_flags = Serial::CharFlags::ignore_modem_control |
                      Serial::CharFlags::enable_receiver |
                      Serial::CharFlags::char_size_8;
  params.min_chars_for_non_canon_read = 1;
  EXPECT_TRUE(tty.set_parameters(params));
  close(parent_fd);
}

TEST(SerialTest, TestTTYGetLine)
{
  const auto parent_fd = posix_openpt(O_RDWR | O_NOCTTY);
  ASSERT_GE(parent_fd, 0) << "Could not open pseudoterminal";
  ASSERT_EQ(0, grantpt(parent_fd)) << "Could not grant pseudoterminal";
  ASSERT_EQ(0, unlockpt(parent_fd)) << "Could not unlock pseudoterminal";
  const auto *child_name = ptsname(parent_fd);
  ASSERT_TRUE(child_name) << "Could not get child pseudoterminal name";
  Serial::TTY tty;
  ASSERT_TRUE(tty.open(child_name));
  const auto expected = string{"Hello World!"};
  write(parent_fd, expected.c_str(), expected.size());
  write(parent_fd, "\n", 1);
  auto actual = string{};
  EXPECT_EQ(Serial::TTY::GetLineResult::ok, tty.get_line(actual));
  EXPECT_EQ(expected, actual);
  close(parent_fd);
}

TEST(SerialTest, TestTTYWriteLine)
{
  const auto parent_fd = posix_openpt(O_RDWR | O_NOCTTY);
  ASSERT_GE(parent_fd, 0) << "Could not open pseudoterminal";
  ASSERT_EQ(0, grantpt(parent_fd)) << "Could not grant pseudoterminal";
  ASSERT_EQ(0, unlockpt(parent_fd)) << "Could not unlock pseudoterminal";
  const auto *child_name = ptsname(parent_fd);
  ASSERT_TRUE(child_name) << "Could not get child pseudoterminal name";
  Serial::TTY tty;
  ASSERT_TRUE(tty.open(child_name));
  const auto expected = string{"Hello World!"};
  EXPECT_TRUE(tty.write_line(expected));
  auto actual = string(expected.size(), '\0');
  read(parent_fd, &actual[0], actual.size());
  EXPECT_EQ(expected, actual);
  close(parent_fd);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

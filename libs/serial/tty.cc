//==========================================================================
// ObTools::Serial: tty.cc
//
// Implementation for TTY class
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-serial.h"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <iostream>

namespace ObTools { namespace Serial {

namespace {

uint32_t baud_to_value(int baud)
{
  switch (baud)
  {
    case B0: return 0;
    case B50: return 50;
    case B75: return 75;
    case B110: return 110;
    case B134: return 134;
    case B150: return 150;
    case B200: return 200;
    case B300: return 300;
    case B600: return 600;
    case B1200: return 1200;
    case B1800: return 1800;
    case B2400: return 2400;
    case B4800: return 4800;
    case B9600: return 9600;
    case B19200: return 19200;
    case B38400: return 38400;
#ifdef B57600
    case B57600: return 57600;
#endif
#ifdef B115200
    case B115200: return 115200;
#endif
#ifdef B230400
    case B230400: return 230400;
#endif
#ifdef B460800
    case B460800: return 460800;
#endif
#ifdef B500000
    case B500000: return 500000;
#endif
#ifdef B576000
    case B576000: return 576000;
#endif
#ifdef B921600
    case B921600: return 921600;
#endif
#ifdef B1000000
    case B1000000: return 1000000;
#endif
#ifdef B1152000
    case B1152000: return 1152000;
#endif
#ifdef B1500000
    case B1500000: return 1500000;
#endif
#ifdef B2000000
    case B2000000: return 2000000;
#endif
#ifdef B2500000
    case B2500000: return 2500000;
#endif
#ifdef B3000000
    case B3000000: return 3000000;
#endif
#ifdef B3500000
    case B3500000: return 3500000;
#endif
#ifdef B4000000
    case B4000000: return 4000000;
#endif
    default: return -1;
  }
}

int value_to_baud(uint32_t value)
{
  switch (value)
  {
    case 0: return B0;
    case 50: return B50;
    case 75: return B75;
    case 110: return B110;
    case 134: return B134;
    case 150: return B150;
    case 200: return B200;
    case 300: return B300;
    case 600: return B600;
    case 1200: return B1200;
    case 1800: return B1800;
    case 2400: return B2400;
    case 4800: return B4800;
    case 9600: return B9600;
    case 19200: return B19200;
    case 38400: return B38400;
#ifdef B57600
    case 57600: return B57600;
#endif
#ifdef B115200
    case 115200: return B115200;
#endif
#ifdef B230400
    case 230400: return B230400;
#endif
#ifdef B460800
    case 460800: return B460800;
#endif
#ifdef B500000
    case 500000: return B500000;
#endif
#ifdef B576000
    case 576000: return B576000;
#endif
#ifdef B921600
    case 921600: return B921600;
#endif
#ifdef B1000000
    case 1000000: return B1000000;
#endif
#ifdef B1152000
    case 1152000: return B1152000;
#endif
#ifdef B1500000
    case 1500000: return B1500000;
#endif
#ifdef B2000000
    case 2000000: return B2000000;
#endif
#ifdef B2500000
    case 2500000: return B2500000;
#endif
#ifdef B3000000
    case 3000000: return B3000000;
#endif
#ifdef B3500000
    case 3500000: return B3500000;
#endif
#ifdef B4000000
    case 4000000: return B4000000;
#endif
    default: return -1;
  }
}

}

//--------------------------------------------------------------------------
// Open a device
bool TTY::open(const string& device)
{
  if (fd >= 0)
    close();
  fd = ::open(device.c_str(), O_RDWR | O_NOCTTY);
  return fd >= 0;
}

//--------------------------------------------------------------------------
// Close device
void TTY::close()
{
  ::close(fd);
  fd = -1;
}

//--------------------------------------------------------------------------
// Get parameters
bool TTY::get_parameters(Parameters& params)
{
  if (fd < 0)
    return false;

  auto tio = termios{};
  if (tcgetattr(fd, &tio))
    return false;

  params.in_speed = baud_to_value(cfgetispeed(&tio));
  params.out_speed = baud_to_value(cfgetospeed(&tio));
  params.in_flags = static_cast<InputFlags>(tio.c_iflag);
  params.out_flags = static_cast<OutputFlags>(tio.c_oflag);
  // Note: speeds are stored in the char flags of the termios structure so
  //       we must deal with them
  params.char_flags = static_cast<CharFlags>(tio.c_cflag & ~CBAUD);
  params.local_flags = static_cast<LocalFlags>(tio.c_lflag);
  params.special_chars.discard = tio.c_cc[VDISCARD];
  params.special_chars.eof = tio.c_cc[VEOF];
  params.special_chars.eol = tio.c_cc[VEOL];
  params.special_chars.eol2 = tio.c_cc[VEOL2];
  params.special_chars.erase = tio.c_cc[VERASE];
  params.special_chars.interrupt = tio.c_cc[VINTR];
  params.special_chars.kill = tio.c_cc[VKILL];
  params.special_chars.literal_next = tio.c_cc[VLNEXT];
  params.special_chars.quit = tio.c_cc[VQUIT];
  params.special_chars.reprint = tio.c_cc[VREPRINT];
  params.special_chars.start = tio.c_cc[VSTART];
  params.special_chars.stop = tio.c_cc[VSTOP];
  params.special_chars.suspend = tio.c_cc[VSUSP];
  params.special_chars.word_erase = tio.c_cc[VWERASE];
  params.min_chars_for_non_canon_read = tio.c_cc[VMIN];
  params.non_canon_read_timeout = chrono::milliseconds{tio.c_cc[VTIME] * 100};

  return true;
}

//--------------------------------------------------------------------------
// Set parameters
bool TTY::set_parameters(const Parameters& params)
{
  auto tio = termios{};

  tio.c_iflag = static_cast<underlying_type<InputFlags>::type>(
                                                          params.in_flags);
  tio.c_oflag = static_cast<underlying_type<OutputFlags>::type>(
                                                          params.out_flags);
  tio.c_cflag = static_cast<underlying_type<CharFlags>::type>(
                                                          params.char_flags);
  tio.c_lflag = static_cast<underlying_type<LocalFlags>::type>(
                                                          params.local_flags);
  tio.c_cc[VDISCARD] = params.special_chars.discard;
  tio.c_cc[VEOF] = params.special_chars.eof;
  tio.c_cc[VEOL] = params.special_chars.eol;
  tio.c_cc[VEOL2] = params.special_chars.eol2;
  tio.c_cc[VERASE] = params.special_chars.erase;
  tio.c_cc[VINTR] = params.special_chars.interrupt;
  tio.c_cc[VKILL] = params.special_chars.kill;
  tio.c_cc[VLNEXT] = params.special_chars.literal_next;
  tio.c_cc[VQUIT] = params.special_chars.quit;
  tio.c_cc[VREPRINT] = params.special_chars.reprint;
  tio.c_cc[VSTART] = params.special_chars.start;
  tio.c_cc[VSTOP] = params.special_chars.stop;
  tio.c_cc[VSUSP] = params.special_chars.suspend;
  tio.c_cc[VWERASE] = params.special_chars.word_erase;
  tio.c_cc[VMIN] = params.min_chars_for_non_canon_read;
  tio.c_cc[VTIME] = params.non_canon_read_timeout.count() / 100;

  // Speed last, because on some systems it is stored in the char flags!
  if (cfsetispeed(&tio, value_to_baud(params.in_speed)))
    return false;
  if (cfsetospeed(&tio, value_to_baud(params.out_speed)))
    return false;

  if (tcflush(fd, TCIOFLUSH))
    return false;
  if (tcsetattr(fd, TCSANOW, &tio))
    return false;

  // The set returns true if anything succeeded, so we need to check that
  // everything succeeded
  auto new_tio = termios{};
  if (tcgetattr(fd, &new_tio))
    return false;

  return !memcmp(&tio, &new_tio, sizeof(tio));
}

//--------------------------------------------------------------------------
// Get a line from the device
bool TTY::get_line(string& line, const chrono::microseconds& timeout)
{
  if (fd < 0)
    return false;
  if (timeout.count())
  {
    // Read timeout is set, so wait for input up to it
    auto fds = fd_set{};
    auto tv = timeval{timeout.count() / 1000000, timeout.count() % 1000000};
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    select(fd + 1, &fds, nullptr, nullptr, &tv);
    auto response = FD_ISSET(fd, &fds);
    if (!response)
      return false;
  }
  line.clear();
  auto c = char{};
  while (read(fd, &c, 1) && c != '\n')
    line += c;
  return true;
}

//--------------------------------------------------------------------------
// Write a line to the device
bool TTY::write_line(const string& line)
{
  if (fd < 0)
    return false;
  write(fd, line.c_str(), line.size());
  write(fd, "\r", 1);
  return true;
}

//--------------------------------------------------------------------------
// Destructor
TTY::~TTY()
{
  if (fd >= 0)
    close();
}

}}

//==========================================================================
// ObTools::Serial: ot-serial.h
//
// Public definitions for ObTools::Serial
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_SERIAL_H
#define __OBTOOLS_SERIAL_H

#include "ot-gen.h"
#include <string>
#include <ostream>
#include <chrono>
#include <termios.h>

namespace ObTools { namespace Serial {

using namespace std;

//==========================================================================
// Input Flags
enum class InputFlags: tcflag_t
{
  none = 0,
  ignore_break = IGNBRK,
  break_interrupt = BRKINT,
  ignore_parity = IGNPAR,
  parity_mark = PARMRK,
  parity_check = INPCK,
  strip_eighth = ISTRIP,
  nl_to_cr = INLCR,
  ignore_cr = IGNCR,
  cr_to_nl = ICRNL,
  to_lower_case = IUCLC,
  xon = IXON,
  xany = IXANY,
  xoff = IXOFF,
  full_queue_bell = IMAXBEL,
  utf8 = IUTF8,
};

//--------------------------------------------------------------------------
// Stream output
ostream& operator<<(ostream& os, InputFlags flags);

//--------------------------------------------------------------------------
// Bitwise and
constexpr InputFlags operator&(InputFlags a, InputFlags b)
{
  return static_cast<InputFlags>(
      static_cast<underlying_type<InputFlags>::type>(a)
      & static_cast<underlying_type<InputFlags>::type>(b));
}

//--------------------------------------------------------------------------
// Bitwise or
constexpr InputFlags operator|(InputFlags a, InputFlags b)
{
  return static_cast<InputFlags>(
      static_cast<underlying_type<InputFlags>::type>(a)
      | static_cast<underlying_type<InputFlags>::type>(b));
}

//--------------------------------------------------------------------------
// Test for set flags
inline bool has_flags(InputFlags flags)
{
  return flags != InputFlags::none;
}

//==========================================================================
// Output Flags
enum class OutputFlags: tcflag_t
{
  none = 0,
  post_processing = OPOST,
  to_upper_case = OLCUC,
  nl_to_cr_nl = ONLCR,
  cr_to_nl = OCRNL,
  no_cr_col_0 = ONOCR,
  no_cr = ONLRET,
  fill_chars = OFILL,
  fill_del = OFDEL,
  newline_delay = NLDLY,
  cr_delay_1 = CR1,
  cr_delay_2 = CR2,
  cr_delay = CRDLY,
  tab_delay_1 = TAB1,
  tab_delay_2 = TAB2,
  tab_delay = TABDLY,
  backspace_delay = BSDLY,
  form_feed_delay = FFDLY,
  vertical_tab_delay = VTDLY,
};

//--------------------------------------------------------------------------
// Stream output
ostream& operator<<(ostream& os, OutputFlags flags);

//--------------------------------------------------------------------------
// Bitwise and
constexpr OutputFlags operator&(OutputFlags a, OutputFlags b)
{
  return static_cast<OutputFlags>(
      static_cast<underlying_type<OutputFlags>::type>(a)
      & static_cast<underlying_type<OutputFlags>::type>(b));
}

//--------------------------------------------------------------------------
// Bitwise or
constexpr OutputFlags operator|(OutputFlags a, OutputFlags b)
{
  return static_cast<OutputFlags>(
      static_cast<underlying_type<OutputFlags>::type>(a)
      | static_cast<underlying_type<OutputFlags>::type>(b));
}

//--------------------------------------------------------------------------
// Test for set flags
inline bool has_flags(OutputFlags flags)
{
  return flags != OutputFlags::none;
}

//==========================================================================
// Char Flags
enum class CharFlags: tcflag_t
{
  none = 0,
  char_size_6 = CS6,
  char_size_7 = CS7,
  char_size_8 = CSIZE,
  two_stop_bits = CSTOPB,
  enable_receiver = CREAD,
  parity_gen = PARENB,
  parity_odd = PARODD,
  hang_up = HUPCL,
  ignore_modem_control = CLOCAL,
  stick_parity = CMSPAR,
  enable_rts_cts = CRTSCTS,
};

//--------------------------------------------------------------------------
// Stream output
ostream& operator<<(ostream& os, CharFlags flags);

//--------------------------------------------------------------------------
// Bitwise and
constexpr CharFlags operator&(CharFlags a, CharFlags b)
{
  return static_cast<CharFlags>(
      static_cast<underlying_type<CharFlags>::type>(a)
      & static_cast<underlying_type<CharFlags>::type>(b));
}

//--------------------------------------------------------------------------
// Bitwise or
constexpr CharFlags operator|(CharFlags a, CharFlags b)
{
  return static_cast<CharFlags>(
      static_cast<underlying_type<CharFlags>::type>(a)
      | static_cast<underlying_type<CharFlags>::type>(b));
}

//--------------------------------------------------------------------------
// Test for set flags
inline bool has_flags(CharFlags flags)
{
  return flags != CharFlags::none;
}

//==========================================================================
// Local Flags
enum class LocalFlags: tcflag_t
{
  none = 0,
  generate_signals = ISIG,
  canonical_mode = ICANON,
  xcase = XCASE,
  echo = ECHO,
  erase_char_word = ECHOE,
  erase_line = ECHOK,
  echo_nl = ECHONL,
  no_flush_on_signal = NOFLSH,
  to_stop = TOSTOP,
  echo_control = ECHOCTL,
  print_erased = ECHOPRT,
  erase_line_char_by_char = ECHOKE,
  flushed = FLUSHO,
  pending_input = PENDIN,
  input_processing = IEXTEN,
};

//--------------------------------------------------------------------------
// Stream output
ostream& operator<<(ostream& os, LocalFlags flags);

//--------------------------------------------------------------------------
// Bitwise and
constexpr LocalFlags operator&(LocalFlags a, LocalFlags b)
{
  return static_cast<LocalFlags>(
      static_cast<underlying_type<LocalFlags>::type>(a)
      & static_cast<underlying_type<LocalFlags>::type>(b));
}

//--------------------------------------------------------------------------
// Bitwise or
constexpr LocalFlags operator|(LocalFlags a, LocalFlags b)
{
  return static_cast<LocalFlags>(
      static_cast<underlying_type<LocalFlags>::type>(a)
      | static_cast<underlying_type<LocalFlags>::type>(b));
}

//--------------------------------------------------------------------------
// Test for set flags
inline bool has_flags(LocalFlags flags)
{
  return flags != LocalFlags::none;
}

//==========================================================================
// Special Characters
struct SpecialChars
{
  char discard = 017;
  char eof = 004;
  char eol = 0;
  char eol2 = 0;
  char erase = 0177;
  char interrupt = 003;
  char kill = 025;
  char literal_next = 026;
  char quit = 034;
  char reprint = 022;
  char start = 021;
  char stop = 023;
  char suspend = 032;
  char word_erase = 027;

  //------------------------------------------------------------------------
  // Equality operator
  bool operator==(const SpecialChars& b) const
  {
    return discard == b.discard &&
           eof == b.eof &&
           eol == b.eol &&
           eol2 == b.eol2 &&
           erase == b.erase &&
           interrupt == b.interrupt &&
           kill == b.kill &&
           literal_next == b.literal_next &&
           quit == b.quit &&
           reprint == b.reprint &&
           start == b.start &&
           stop == b.stop &&
           suspend == b.suspend &&
           word_erase == b.word_erase;
  }
};

//==========================================================================
// Parameters
struct Parameters
{
  int in_speed = -1;
  int out_speed = -1;
  InputFlags in_flags;
  OutputFlags out_flags;
  CharFlags char_flags;
  LocalFlags local_flags;
  SpecialChars special_chars;
  uint8_t min_chars_for_non_canon_read = 1;
  // Note: Will be rounded down to deciseconds
  chrono::milliseconds non_canon_read_timeout;
};

//==========================================================================
// TTY
class TTY
{
private:
  int fd = -1;

public:
  //------------------------------------------------------------------------
  // Open a device
  bool open(const string& device);

  //------------------------------------------------------------------------
  // Close device
  void close();

  //------------------------------------------------------------------------
  // Get parameters
  bool get_parameters(Parameters& params);

  //------------------------------------------------------------------------
  // Set parameters
  bool set_parameters(const Parameters& params);

  //------------------------------------------------------------------------
  // Get a line from the device
  enum class GetLineResult
  {
    ok,
    fail,
    timeout,
    interrupt,
  };
  GetLineResult get_line(string& line,
             const chrono::microseconds& timeout = chrono::microseconds{0});

  //------------------------------------------------------------------------
  // Write a line to the device
  bool write_line(const string& line);

  //------------------------------------------------------------------------
  // Validity check
  inline explicit operator bool() const
  {
    return fd >= 0;
  }

  //------------------------------------------------------------------------
  // Destructor
  ~TTY();
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_SERIAL_H

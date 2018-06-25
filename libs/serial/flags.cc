//==========================================================================
// ObTools::Serial: flags.cc
//
// Implementation for flags types
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-serial.h"

namespace ObTools { namespace Serial {

//==========================================================================
// Input Flags
ostream& operator<<(ostream& os, InputFlags flags)
{
  os << (has_flags(flags & InputFlags::ignore_break) ? "+" : "-")
     << "ignore_break "
     << (has_flags(flags & InputFlags::break_interrupt) ? "+" : "-")
     << "break_interrupt "
     << (has_flags(flags & InputFlags::ignore_parity) ? "+" : "-")
     << "ignore_parity "
     << (has_flags(flags & InputFlags::parity_mark) ? "+" : "-")
     << "parity_mark "
     << (has_flags(flags & InputFlags::parity_check) ? "+" : "-")
     << "parity_check "
     << (has_flags(flags & InputFlags::strip_eighth) ? "+" : "-")
     << "strip_eighth "
     << (has_flags(flags & InputFlags::nl_to_cr) ? "+" : "-")
     << "nl_to_cr "
     << (has_flags(flags & InputFlags::ignore_cr) ? "+" : "-")
     << "ignore_cr "
     << (has_flags(flags & InputFlags::cr_to_nl) ? "+" : "-")
     << "cr_to_nl "
     << (has_flags(flags & InputFlags::to_lower_case) ? "+" : "-")
     << "to_lower_case "
     << (has_flags(flags & InputFlags::xon) ? "+" : "-")
     << "xon "
     << (has_flags(flags & InputFlags::xany) ? "+" : "-")
     << "xany "
     << (has_flags(flags & InputFlags::xoff) ? "+" : "-")
     << "xoff "
     << (has_flags(flags & InputFlags::full_queue_bell) ? "+" : "-")
     << "full_queue_bell "
     << (has_flags(flags & InputFlags::utf8) ? "+" : "-")
     << "utf8";
  return os;
}

//==========================================================================
// Output Flags
ostream& operator<<(ostream& os, OutputFlags flags)
{
  os << (has_flags(flags & OutputFlags::post_processing) ? "+" : "-")
     << "post_processing "
     << (has_flags(flags & OutputFlags::to_upper_case) ? "+" : "-")
     << "to_upper_case "
     << (has_flags(flags & OutputFlags::nl_to_cr_nl) ? "+" : "-")
     << "nl_to_cr_nl "
     << (has_flags(flags & OutputFlags::cr_to_nl) ? "+" : "-")
     << "cr_to_nl "
     << (has_flags(flags & OutputFlags::no_cr_col_0) ? "+" : "-")
     << "no_cr_col_0 "
     << (has_flags(flags & OutputFlags::no_cr) ? "+" : "-")
     << "no_cr "
     << (has_flags(flags & OutputFlags::fill_chars) ? "+" : "-")
     << "fill_chars "
     << (has_flags(flags & OutputFlags::fill_del) ? "+" : "-")
     << "fill_del "
     << (has_flags(flags & OutputFlags::newline_delay) ? "+" : "-")
     << "newline_delay "
     << (has_flags(flags & OutputFlags::cr_delay_1) ? "+" : "-")
     << "cr_delay_1 "
     << (has_flags(flags & OutputFlags::cr_delay_2) ? "+" : "-")
     << "cr_delay_2 "
     << (has_flags(flags & OutputFlags::cr_delay) ? "+" : "-")
     << "cr_delay "
     << (has_flags(flags & OutputFlags::tab_delay_1) ? "+" : "-")
     << "tab_delay_1 "
     << (has_flags(flags & OutputFlags::tab_delay_2) ? "+" : "-")
     << "tab_delay_2 "
     << (has_flags(flags & OutputFlags::tab_delay) ? "+" : "-")
     << "tab_delay "
     << (has_flags(flags & OutputFlags::backspace_delay) ? "+" : "-")
     << "backspace_delay "
     << (has_flags(flags & OutputFlags::form_feed_delay) ? "+" : "-")
     << "form_feed_delay "
     << (has_flags(flags & OutputFlags::vertical_tab_delay) ? "+" : "-")
     << "vertical_tab_delay";
  return os;
}

//==========================================================================
// Char Flags
ostream& operator<<(ostream& os, CharFlags flags)
{
  os << (has_flags(flags & CharFlags::char_size_6) ? "+" : "-")
     << "char_size_6 "
     << (has_flags(flags & CharFlags::char_size_7) ? "+" : "-")
     << "char_size_7 "
     << (has_flags(flags & CharFlags::char_size_8) ? "+" : "-")
     << "char_size_8 "
     << (has_flags(flags & CharFlags::two_stop_bits) ? "+" : "-")
     << "two_stop_bits "
     << (has_flags(flags & CharFlags::enable_receiver) ? "+" : "-")
     << "enable_receiver "
     << (has_flags(flags & CharFlags::parity_gen) ? "+" : "-")
     << "parity_gen "
     << (has_flags(flags & CharFlags::parity_odd) ? "+" : "-")
     << "parity_odd "
     << (has_flags(flags & CharFlags::hang_up) ? "+" : "-")
     << "hang_up "
     << (has_flags(flags & CharFlags::ignore_modem_control) ? "+" : "-")
     << "ignore_modem_control "
     << (has_flags(flags & CharFlags::stick_parity) ? "+" : "-")
     << "stick_parity "
     << (has_flags(flags & CharFlags::enable_rts_cts) ? "+" : "-")
     << "enable_rts_cts";
  return os;
}

//==========================================================================
// Local Flags
ostream& operator<<(ostream& os, LocalFlags flags)
{
  os << (has_flags(flags & LocalFlags::generate_signals) ? "+" : "-")
     << "generate_signals "
     << (has_flags(flags & LocalFlags::canonical_mode) ? "+" : "-")
     << "canonical_mode "
     << (has_flags(flags & LocalFlags::xcase) ? "+" : "-")
     << "xcase "
     << (has_flags(flags & LocalFlags::echo) ? "+" : "-")
     << "echo "
     << (has_flags(flags & LocalFlags::erase_char_word) ? "+" : "-")
     << "erase_char_word "
     << (has_flags(flags & LocalFlags::erase_line) ? "+" : "-")
     << "erase_line "
     << (has_flags(flags & LocalFlags::echo_nl) ? "+" : "-")
     << "echo_nl "
     << (has_flags(flags & LocalFlags::to_stop) ? "+" : "-")
     << "to_stop "
     << (has_flags(flags & LocalFlags::echo_control) ? "+" : "-")
     << "echo_control "
     << (has_flags(flags & LocalFlags::print_erased) ? "+" : "-")
     << "print_erased "
     << (has_flags(flags & LocalFlags::erase_line_char_by_char) ? "+" : "-")
     << "erase_line_char_by_char "
     << (has_flags(flags & LocalFlags::flushed) ? "+" : "-")
     << "flushed "
     << (has_flags(flags & LocalFlags::pending_input) ? "+" : "-")
     << "pending_input "
     << (has_flags(flags & LocalFlags::input_processing) ? "+" : "-")
     << "input_processing";
  return os;
}

}}

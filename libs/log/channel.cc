//==========================================================================
// ObTools::Log: channel.cc
//
// Standard channel implementations
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-log.h"

namespace ObTools { namespace Log {

//==========================================================================
// LineChannel

//--------------------------------------------------------------------------
// Logging function
void LineChannel::log(Message& msg)
{
  MT::Lock lock(mutex);
  char stm[81];
  
  strftime(stm, 80, time_format.c_str(), localtime(&msg.timestamp));

  string& text = msg.text;
  for(string::iterator p=text.begin(); p!=text.end(); p++)
  {
    if (*p=='\n')
    {
      if (buffer.size()) 
      {
	string line = stm;
	line += buffer;
	log_line(line);
	buffer.erase();
      }
      // Ignore blank lines
    }
    else buffer+=*p;
  }
}

//==========================================================================
// ostream StreamChannel

//--------------------------------------------------------------------------
// Logging function
void StreamChannel::log_line(const string& line)
{
  stream << line << endl;
}


}} // namespaces




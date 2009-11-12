//==========================================================================
// ObTools::DNS: test-resolver.cc
//
// Test harness for DNS library
//
// Copyright (c) 2008 Obtools Limited.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-dns.h"
#include "ot-log.h"
#include "ot-misc.h"
#include <iostream>

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    cout << "Give a domain name\n";
    return 2;
  }

  int log_level = Log::LEVEL_SUMMARY;

  // Read options
  int i;
  for(i=1; i<argc-1; i++)
  {
    string opt(argv[i]);
    if (opt == "-q" || opt=="--quiet") 
      log_level--;
    else if (opt == "-v" || opt=="--verbose") 
      log_level++;
    else
    {
      cerr << "Unknown option: " << opt << endl;
      return 2;
    }
  }

  string domain(argv[argc-1]);

  // Set up logging
  Log::StreamChannel chan_out(cout);
  Log::LevelFilter level_out((Log::Level)log_level, chan_out);
  Log::logger.connect(level_out);
  Log::Streams log;


  DNS::Resolver resolver;

  // TXT record
  string txt = resolver.query_txt(domain);
  cout << "TXT:\n" << txt << endl;

  // CERT record
  string der = resolver.query_cert(domain);
  cout << "CERT:\n";
  Misc::Dumper dumper(cout);
  dumper.dump(der);

  return 0;

}





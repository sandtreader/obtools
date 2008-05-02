//==========================================================================
// ObTools::Access: test-access.cc
//
// Test harness for access checker library
//
// Copyright (c) 2008 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-access.h"
#include "ot-log.h"

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  if (argc < 2)
  {
    cout << "Usage:\n";
    cout << "  " << argv[0] << " <config file>\n";
    return 0;
  }

  XML::Configuration config(argv[1]);
  if (!config.read("root")) return 2;

  XML::Element *access_e = config.get_element("access");
  if (!access_e)
  {
    cerr << "No access element!\n";
    return 2;
  }

  Access::Checker checker(*access_e);

  // Now read lines of resource/user pairs to check
  while (cin)
  {
    string resource, user, addr;
    cin >> resource >> user >> addr;
    if (resource.empty()) break;

    Net::IPAddress address(addr);
    bool result = checker.check(resource, addr, user);

    cout << resource << "\t" << user << "\t" << address << "\t"
	 << (result?"ALLOW":"DENY") << endl;
  }

  return 0;  
}





//==========================================================================
// ObTools::SOAP: test-soap.cc
//
// Test harness for SOAP library functions
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-soap.h"
#include <iostream>

using namespace std;
using namespace ObTools;


//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  SOAP::Message msg;
  msg.add_header(new XML::Element("xm:header1"), SOAP::Header::ROLE_NEXT);
  msg.add_header(new XML::Element("xm:header2"), "xm:role", false);
  msg.add_body(new XML::Element("xm:body"));
  cout << "Constructed message:\n" << msg;

  SOAP::Message msg2(cin, cerr);
  cout << "\nRead message:\n" << msg2;

  cout << "\nHeaders:\n";
  list<SOAP::Header> headers = msg2.get_headers();
  for(list<SOAP::Header>::iterator p = headers.begin();
      p!= headers.end();
      p++)
  {
    SOAP::Header h = *p;
    cout << "- " << h.content->name;
    if (h.must_understand) cout << " (must understand)";
    cout << " (role " << (int)h.role << ")\n";
  }

  cout << "\nBody elements:\n";
  list<XML::Element *> bodies = msg2.get_bodies();
  for(list<XML::Element *>::iterator p = bodies.begin();
      p!= bodies.end();
      p++)
    cout << "- " << (*p)->name << endl;

  return 0;  
}





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
int main()
{
  SOAP::Message msg;
  msg.add_namespace("xmlns:xm", "http://www.obtools.com/foo");
  msg.add_header("xm:header1", SOAP::Header::ROLE_NEXT);
  msg.add_header("xm:header2", "xm:role", false, true);
  msg.add_header("xm:header3", SOAP::Header::ROLE_ULTIMATE_RECEIVER, true);
  msg.add_body("xm:body");
  cout << "Constructed message:\n" << msg << endl;

  SOAP::Parser parser(cerr);
  SOAP::Message msg2(cin, parser);
  if (!!msg2)
  {
    cout << "Read message:\n" << msg2;

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
  }
  else
  {
    cerr << "Can't read message from stdin\n";
    return 2;
  }

  cout << "\nConstructed Fault:\n";
  SOAP::Fault fault(SOAP::Fault::CODE_RECEIVER, "It went wrong");
  fault.add_reason("Ca marche pas", "fr");
  fault.set_subcode("xm:whoops");
  fault.set_node("http://foo");
  fault.set_role(SOAP::RN_NEXT);
  cout << fault;

  cout << "\nConstructed VersionMismatch Fault:\n";
  SOAP::VersionMismatchFault vm_fault;
  cout << vm_fault;

  cout << "\nConstructed MustUnderstand Fault:\n";
  SOAP::MustUnderstandFault mu_fault;
  mu_fault.add_not_understood("xm:foo", "xmlns:xm", 
			      "http://www.obtools.com/foo");
  cout << mu_fault;

  return 0;  
}





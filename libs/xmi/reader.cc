//==========================================================================
// ObTools::XMI: reader.cc
//
// XMI reader class
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "xmi.h"
#include "xml.h"
using namespace ObTools::XMI;

//--------------------------------------------------------------------------
// Destructor
Reader::~Reader()
{

}

//------------------------------------------------------------------------
// Parse from given input stream
void Reader::read_from(istream& s) throw (ParseFailed)
{
  ObTools::XML::Parser parser;

  try
  {
    s >> parser;
  }
  catch (ObTools::XML::ParseFailed)
  {
    serr << "XML parse failed" << endl;
    throw ParseFailed();  
  }

  ObTools::XML::Element& root=parser.get_root();
}

//------------------------------------------------------------------------
// >> operator to read from istream
istream& ObTools::XMI::operator>>(istream& s, Reader& p) throw (ParseFailed)
{ 
  p.read_from(s);
}




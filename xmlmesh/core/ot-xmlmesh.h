//==========================================================================
// ObTools::XMLBus:Core ot-xmlbus.h
//
// Core definitions for XMLBus 
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#ifndef __OBTOOLS_XMLBUS_H
#define __OBTOOLS_XMLBUS_H

#include <string>
#include "ot-net.h"
#include "ot-mt.h"
#include "ot-xml.h"

namespace ObTools { namespace XMLBus {

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// XMLBus message
class Message
{
private:
  // Two forms of message - either XML, or text, or both.  get_xml and
  // get_text will convert if necessary, but holding both saves work
  // when just passing things through
  XML::Element *xml_message;   // XML <message> element
  string textual_message;      // Textual message (<message ...>)

  string allocate_id();

public:
  //--------------------------------------------------------------------------
  // Basic constructor from text for outgoing messages
  // ID is manufactured here
  Message(const string& subject, const string& text_content,
	  bool rsvp = false, const string& ref="");

  //--------------------------------------------------------------------------
  // Constructor from XML for outgoing messages
  // ID is manufactured here
  // Takes ownership of the XML::Element - make sure it's detached from the
  // parser!
  Message(const string& subject, XML::Element *xml_content,
	  bool rsvp = false, const string& ref="");

  //--------------------------------------------------------------------------
  // Constructor from XML <message> text for incoming messages
  Message(const string& message_text);

  //--------------------------------------------------------------------------
  // Get <message> text
  string Message::get_text();

  //--------------------------------------------------------------------------
  //Get XML content, still owned by Message, will be destroyed with it
  XML::Element& get_xml();

  //--------------------------------------------------------------------------
  //Get XML content to keep after Message is destroyed
  //Can return 0 if XML parse failed
  XML::Element *detach_xml();

  //--------------------------------------------------------------------------
  //Get subject of a message
  string get_subject();

  //--------------------------------------------------------------------------
  //Get id of a message
  string get_id();

  //--------------------------------------------------------------------------
  //Get whether the message requires a response
  bool get_rsvp();

  //--------------------------------------------------------------------------
  //Get reference of a message
  string get_ref();

  //--------------------------------------------------------------------------
  //Destructor - kills xml data if not detached
  ~Message();
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_XMLBUS_H




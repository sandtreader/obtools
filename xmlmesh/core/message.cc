//==========================================================================
// ObTools::XMLMesh:Core: message.cc
//
// Implementation of XMLMesh message
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-xmlmesh.h"
#include "ot-mt.h"
#include "ot-log.h"
#include <sstream>

namespace ObTools { namespace XMLMesh {

//--------------------------------------------------------------------------
//Static globals for ID allocation - threadsafe

static MT::Mutex id_mutex;
static int id_serial;

//--------------------------------------------------------------------------
//Allocate an ID
string Message::allocate_id()
{
  MT::Lock lock(id_mutex);
  ostringstream ids;
  ids << "OTM-" << ++id_serial;
  return ids.str();
}

//--------------------------------------------------------------------------
// Basic constructor from text for outgoing messages
// ID is manufactured here
Message::Message(const string& subject, const string& text_content,
		 bool rsvp, const string& ref)
{
  string id = allocate_id();

  // Hold in a textual form to save parsing the text
  ostringstream mss;
  mss << "<message id=\"" << id << "\" subject=\"" << subject << "\"";
  if (rsvp) mss << " rsvp=\"yes\"";
  if (ref.size()) mss << " ref=\"" << ref << "\"";
  mss << ">\n";
  mss << text_content << endl;
  mss << "</message>\n";

  textual_message = mss.str();
  xml_message = 0;  // Don't parse unless required
}

//--------------------------------------------------------------------------
// Constructor from XML for outgoing messages
// ID is manufactured here
// Takes ownership of the XML::Element - make sure it's detached from the
// parser!
Message::Message(const string& subject, XML::Element *xml_content,
		 bool rsvp, const string& ref)
{
  string id = allocate_id();

  // Manufacture an XML <message> element containing the attributes and
  // the given content
  xml_message = new XML::Element("message");
  xml_message->attrs["id"] = id;
  xml_message->attrs["subject"] = subject;
  if (rsvp) xml_message->attrs["rsvp"] = "yes";
  if (ref.size()) xml_message->attrs["ref"] = ref;
  xml_message->children.push_back(xml_content);
}

//--------------------------------------------------------------------------
// Constructor from XML <message> text for incoming messages
Message::Message(const string& message_text)
{
  textual_message = message_text;
  xml_message = 0;  // For now - will be created in get_xml if required
}

//--------------------------------------------------------------------------
// Get <message> text
string Message::get_text()
{
  // If text doesn't already exist, but xml does, get the textual version
  if (!textual_message.size() && xml_message)
  {
    ostringstream mss;
    mss << *xml_message;
    textual_message = mss.str();
  }
  
  return textual_message;
}

//--------------------------------------------------------------------------
//Get XML content, still owned by Message, will be destroyed with it
XML::Element& Message::get_xml()
{
  // If we've already got it, return immediately
  if (xml_message) return *xml_message;

  // Parse XML
  XML::Parser parser(Log::Error);
  try
  {
    parser.read_from(textual_message);
    XML::Element root = parser.get_root();
    if (root.name == "message")
    {
      // Detach it to keep
      xml_message = parser.detach_root();
    }
    else
    {
      Log::Error << "XMLMesh:: Received bogus XML root: " << root.name << endl;
    }
  }
  catch (XML::ParseFailed)
  {
    Log::Error << "XMLMesh:: Can't parse incoming message\n";
  }

  // See if we've got one now - if not, return Element::none for safety
  if (xml_message)
    return *xml_message;
  else
    return XML::Element::none;
}

//--------------------------------------------------------------------------
//Get XML content to keep after Message is destroyed
//Can return 0 if XML parse failed
XML::Element *Message::detach_xml()
{
  // Try to ensure we have some XML
  get_xml();

  // If we have it, detach and return it
  if (xml_message)
  {
    XML::Element *m = xml_message;
    xml_message = 0;
    return m;
  }
  else return 0;
}

//--------------------------------------------------------------------------
//Get subject of a message
string Message::get_subject()
{
  XML::Element& xml = get_xml();
  return xml.get_attr("subject");
}

//--------------------------------------------------------------------------
//Get id of a message
string Message::get_id()
{
  XML::Element& xml = get_xml();
  return xml.get_attr("id");
}

//--------------------------------------------------------------------------
//Get whether the message requires a response
bool Message::get_rsvp()
{
  XML::Element& xml = get_xml();
  return xml.get_attr_bool("rsvp");
}

//--------------------------------------------------------------------------
//Get reference of a message
string Message::get_ref()
{
  XML::Element& xml = get_xml();
  return xml.get_attr("ref");
}


//--------------------------------------------------------------------------
//Destructor - kills xml data if not detached
Message::~Message()
{
  if (xml_message) delete xml_message;
}

}} // namespaces






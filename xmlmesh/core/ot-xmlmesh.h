//==========================================================================
// ObTools::XMLMesh:Core ot-xmlmesh.h
//
// Core definitions for XMLMesh 
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#ifndef __OBTOOLS_XMLMESH_H
#define __OBTOOLS_XMLMESH_H

#include <string>
#include "ot-net.h"
#if !defined(_SINGLE)
#include "ot-mt.h"
#endif

#include "ot-xml.h"

namespace ObTools { namespace XMLMesh {

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// XMLMesh message
class Message
{
private:
  // Two forms of message - either XML, or text, or both.  get_xml and
  // get_text will convert if necessary, but holding both saves work
  // when just passing things through
  XML::Element *xml_message;   // XML <message> element
  string textual_message;      // Textual message (<message ...>)

  string allocate_id();

protected:
  // Create from text content
  void create(const string& subject, const string& content,
	      bool rsvp=false, const string& ref="");

public:
  //--------------------------------------------------------------------------
  // Default constructor - assumes we will set the message text later
  Message(): xml_message(0) {}

  //--------------------------------------------------------------------------
  // Basic constructor from text for outgoing messages
  // ID is manufactured here
  Message(const string& subject, const string& text_content,
	  bool rsvp = false, const string& ref=""): xml_message(0)
  { create(subject, text_content, rsvp, ref); }

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
  // Copy constructor - don't transfer ownership of XML form
  // Note, uses to_text to ensure there is a textual form, since we aren't
  // taking the XML
  Message(const Message& m):
    xml_message(0), textual_message(m.to_text()) {}

  //--------------------------------------------------------------------------
  // Assignment operator, likewise
  Message& operator=(const Message &m)
  {
    // Check for old XML
    if (xml_message) delete xml_message;

    xml_message = 0;
    textual_message = m.to_text();
  }

  //--------------------------------------------------------------------------
  // Get <message> text, but don't cache it
  string Message::to_text() const;

  //--------------------------------------------------------------------------
  // Get <message> text and cache it
  string Message::get_text();

  //--------------------------------------------------------------------------
  //Get XML content, still owned by Message, will be destroyed with it
  const XML::Element& get_xml();

  //--------------------------------------------------------------------------
  //Get XML content for modification - clears textual copy if any.
  //XML is still owned by Message, will be destroyed with it
  XML::Element& get_modifiable_xml();

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
  //Set id of a message
  void set_subject(const string& new_subject);

  //--------------------------------------------------------------------------
  //Set id of a message
  void set_id(const string& new_id);

  //--------------------------------------------------------------------------
  //Set rsvp of a message
  void set_rsvp(bool new_rsvp);

  //--------------------------------------------------------------------------
  //Set ref of a message
  void set_ref(const string& new_ref);

  //--------------------------------------------------------------------------
  //Destructor - kills xml data if not detached
  ~Message();
};

//------------------------------------------------------------------------
// << operator to write Message to ostream
ostream& operator<<(ostream& s, const Message& m);

//==========================================================================
// Standard messages - xmlmesh.ok
class OKMessage: public Message
{
public:
  //--------------------------------------------------------------------------
  // Constructor given string ref, for responses
  OKMessage(const string& _ref):
    Message("xmlmesh.ok", "<xmlmesh.ok/>", false, _ref) {}

  //--------------------------------------------------------------------------
  // Down-cast constructor from general message on receipt
  // No data of interest, so don't include anything
  OKMessage(const Message& msg): Message() {}
};

//==========================================================================
// Standard messages - xmlmesh.error

class ErrorMessage: public Message
{
public:
  // Severity of error
  enum Severity
  {
    WARNING,
    ERROR,
    SERIOUS,
    FATAL,
    BOGUS          // Used if error can't be parsed
  };

  Severity severity;
  string text;

  //--------------------------------------------------------------------------
  // Constructor for responses
  ErrorMessage(const string& _ref, Severity _severity, 
	       const string& _text);

  //--------------------------------------------------------------------------
  // Down-cast constructor from general message on receipt
  // Note: not const Message& because we may modify it by getting text
  ErrorMessage(Message& msg);

  //--------------------------------------------------------------------------
  //Test for badness
  bool operator!() { return severity == BOGUS; }
};

//------------------------------------------------------------------------
// << operator to write ErrorMessage to ostream
ostream& operator<<(ostream& s, const ErrorMessage& m);

//==========================================================================
// Standard messages - xmlmesh.subscription
class SubscriptionMessage: public Message
{
public:
  // Subscription operation
  enum Operation
  {
    JOIN,
    LEAVE,
    BOGUS          // Used if operation can't be parsed
  };

  Operation operation;
  string subject;

  //--------------------------------------------------------------------------
  // Constructor for requests
  SubscriptionMessage(Operation _operation, const string& _subject);

  //--------------------------------------------------------------------------
  // Down-cast constructor from general message on receipt
  // Note: not const Message& because we may modify it by getting text
  SubscriptionMessage(Message& msg);

  //--------------------------------------------------------------------------
  //Test for badness
  bool operator!() { return operation == BOGUS; }
};

//------------------------------------------------------------------------
// << operator to write SubscriptionMessage to ostream
ostream& operator<<(ostream& s, const SubscriptionMessage& m);

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_XMLMESH_H




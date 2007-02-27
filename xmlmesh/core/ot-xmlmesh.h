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

#include "ot-soap.h"

namespace ObTools { namespace XMLMesh {

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// XMLMesh message
class Message
{
protected:
  // Two forms of message - either XML, or text, or both.  get_xml and
  // get_text will convert if necessary, but holding both saves work
  // when just passing things through
  SOAP::Message *soap_message;  // SOAP message
  string textual_message;       // Textual message (<message ...>)

  const XML::Element& get_routing_header();

public:
  //--------------------------------------------------------------------------
  // Default constructor - assumes we will set the message text later
  Message(): soap_message(0) {}

  //--------------------------------------------------------------------------
  // Constructor from existing SOAP::Message for outgoing messages
  // ID is manufactured here, and routing header added
  // 'soap' is taken and will be disposed with message
  Message(const string& subject, SOAP::Message *soap,
	  bool rsvp, const string& ref);

  //--------------------------------------------------------------------------
  // Constructor from XML for outgoing messages
  // ID is manufactured here
  // Takes ownership of the XML::Element - make sure it's detached from the
  // parser!
  Message(const string& subject, XML::Element *xml_content,
	  bool rsvp = false, const string& ref="");

  //--------------------------------------------------------------------------
  // Constructor from partial XML for outgoing messages
  // ID is manufactured here
  // Copies text of xml element and reparses it into body
  Message(const string& subject, const XML::Element& xml_content,
	  bool rsvp = false, const string& ref="");

  //--------------------------------------------------------------------------
  // Constructor from partial XML text for outgoing messages
  // ID is manufactured here
  // body_text is the body text to be sent
  Message(const string& subject, const string& body_text,
	  bool rsvp = false, const string& ref="");

  //--------------------------------------------------------------------------
  // Constructor from XML <message> text for incoming messages
  Message(const string& message_text);

  //--------------------------------------------------------------------------
  // Copy constructor - don't transfer ownership of XML form
  // Note, uses to_text to ensure there is a textual form, since we aren't
  // taking the XML
  Message(const Message& m):
    soap_message(0), textual_message(m.to_text()) {}

  //--------------------------------------------------------------------------
  // Assignment operator, likewise
  Message& operator=(const Message &m)
  {
    // Check for old XML
    if (soap_message) delete soap_message;

    soap_message = 0;
    textual_message = m.to_text();
    return *this;
  }

  //--------------------------------------------------------------------------
  // Get <message> text, but don't cache it
  string Message::to_text() const;

  //--------------------------------------------------------------------------
  // Get <message> text and cache it
  string Message::get_text();

  //--------------------------------------------------------------------------
  //Get SOAP Message, still owned by Message, will be destroyed with it
  //Check for validity with !
  const SOAP::Message& Message::get_soap();

  //--------------------------------------------------------------------------
  //Get SOAP message for modification - clears textual copy if any
  //SOAP is still owned by Message, will be destroyed with it
  SOAP::Message& get_modifiable_soap();

  //--------------------------------------------------------------------------
  //Get XML Body content, still owned by Message, will be destroyed with it
  //Check for validity with !
  XML::Element& get_body();

  //--------------------------------------------------------------------------
  //Ditto, but specifying a particular element name
  //Check for validity with !
  XML::Element& get_body(const string& name);

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
    Message("xmlmesh.ok", new XML::Element("x:ok"), false, _ref) {}

  //--------------------------------------------------------------------------
  // Down-cast constructor from general message on receipt
  // No data of interest, so don't include anything
  OKMessage(const Message&): Message() {}
};

//==========================================================================
// Fault message
class FaultMessage: public Message
{
public:
  SOAP::Fault::Code code;
  string reason;

  //--------------------------------------------------------------------------
  // Constructor for responses
  FaultMessage(const string& ref, SOAP::Fault::Code _code,
	       const string& _reason);

  //--------------------------------------------------------------------------
  // Down-cast constructor from general message on receipt
  // Note: not const Message& because we may modify it by getting text
  FaultMessage(Message& msg);

  //--------------------------------------------------------------------------
  // Get raw SOAP Fault message for further unpacking
  SOAP::Fault& get_fault() const;

  //--------------------------------------------------------------------------
  //Test for badness
  bool operator!() { return code == SOAP::Fault::CODE_UNKNOWN; }
};

//------------------------------------------------------------------------
// << operator to write FaultMessage to ostream
ostream& operator<<(ostream& s, const FaultMessage& m);

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




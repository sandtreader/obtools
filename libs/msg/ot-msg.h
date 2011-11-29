//==========================================================================
// ObTools::Message: ot-msg.h
//
// Public definitions for ObTools::Message
// Support for generic message/RPC broker
//
// Copyright (c) 2010 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_MSG_H
#define __OBTOOLS_MSG_H

#include "ot-xml.h"
#include "ot-init.h"
#include "ot-ssl.h"

namespace ObTools { namespace Message {

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Unified message handler interface (abstract)
class Handler
{
public:
  string name;                  // Handler name
  string document_name;         // Message document name prefix
                                // (leave empty to check/generate manually)
  bool complex_result;          // Is there a complex result, or just 'ok'?
  string ns_prefix;             // Namespace prefix
  string ns_url;                // Namespace URL

  //--------------------------------------------------------------------------
  // Default constructor - the above filled in in subclass constructors
  Handler(XML::Element& cfg, const string& _doc_name="",
	  const string& _ns_prefix="", const string& _ns_url="",
	  bool _complex_result = false):
    name(cfg.name),
    document_name(_doc_name),
    complex_result(_complex_result),
    ns_prefix(_ns_prefix),
    ns_url(_ns_url)
  {}

  //--------------------------------------------------------------------------
  // Call to handle message and respond
  // Returns error string or "" if OK, in which case response is filled in
  // Response document will be pre-created with document_name-response, if set
  virtual string handle_message(const XML::Element& request,
				SSL::ClientDetails& client,
				XML::Element& response) = 0;

  //--------------------------------------------------------------------------
  // Virtual destructor (probably not needed)
  virtual ~Handler() {}
};

//==========================================================================
// Message transport interface - register message handlers against underlying
// transport (abstract)
class Transport
{
 public:
  string name;  // Short name for config

  //--------------------------------------------------------------------------
  // Constructor
  Transport(const string& _name): name(_name) {}

  //--------------------------------------------------------------------------
  // Register a handler with the given config element
  virtual void register_handler(Handler& handler, XML::Element& config) = 0;

  //--------------------------------------------------------------------------
  // Virtual destructor
  virtual ~Transport() {}
};

//==========================================================================
// Unified message broker - accepts registration of multiple message handlers
// into multiple message transports
class Broker
{
  map<string, list<Transport *> > transports;  // Transports mapped by name
  list<Handler *> handlers;                    // All registered handlers

  // Internals
  bool create_handler(XML::Element& xml);

public:
  // Registry of message handlers - provides automatic initialisation
  Init::Registry<Handler> handler_registry;

  //--------------------------------------------------------------------------
  // Constructor
  Broker() {}

  //--------------------------------------------------------------------------
  // Add a transport
  void add_transport(Transport *trans);

  //--------------------------------------------------------------------------
  // Configure from XML configuration (<message> element)
  void configure(XML::Element& config);

  //--------------------------------------------------------------------------
  // Shut down
  void shutdown();

  //--------------------------------------------------------------------------
  // Destructor
  ~Broker();
};


//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_MSG_H


















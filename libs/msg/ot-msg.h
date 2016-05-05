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
#include "ot-log.h"
#include <stdexcept>

namespace ObTools { namespace Message {

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Generic exception
struct Exception: public runtime_error
{
  Exception(const string& _error): runtime_error(_error) {}
};

//==========================================================================
// Unified message handler interface (abstract template)
// <CONTEXT> is the class used to provide context to the handler
template<class CONTEXT>
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
  Handler(const XML::Element& cfg, const string& _doc_name="",
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
  // throws Exception on any error, otherwise fills in response
  // Response document will be pre-created with document_name-response, if set
  virtual void handle_message(CONTEXT& context,
                              const XML::Element& request,
                              const SSL::ClientDetails& client,
                              XML::Element& response) = 0;

  //--------------------------------------------------------------------------
  // Virtual destructor (probably not needed)
  virtual ~Handler() {}
};

//==========================================================================
// Message transport interface - register message handlers against underlying
// transport (abstract template)
// <CONTEXT> is a Handler context
template<class CONTEXT>
class Transport
{
 public:
  string name;  // Short name for config

  //--------------------------------------------------------------------------
  // Constructor
  Transport(const string& _name): name(_name) {}

  //--------------------------------------------------------------------------
  // Register a handler with the given config element
  virtual void register_handler(Handler<CONTEXT>& handler,
                                const XML::Element& config) = 0;

  //--------------------------------------------------------------------------
  // Virtual destructor
  virtual ~Transport() {}
};

//==========================================================================
// Unified message broker - accepts registration of multiple message handlers
// into multiple message transports
// <CONTEXT> is a Handler context
template<class CONTEXT>
class Broker
{
  // Transports mapped by name
  map<string, list<Transport<CONTEXT> *> > transports;

  // All registered handlers
  list<Handler<CONTEXT> *> handlers;

  // Registry of message handlers
  Init::Registry<Handler<CONTEXT> > &handler_registry;

  // Internals
  bool create_handler(const XML::Element& xml)
  {
    Handler<CONTEXT> *mh = handler_registry.create(xml.name, xml);
    if (!mh) return false;

    // Store it
    handlers.push_back(mh);

    // Register in all transports with child elements in this one
    for(XML::Element::iterator p(xml.children); p; ++p)
    {
      XML::Element& te = *p;
      if (!te.name.empty())
      {
        // Look for all matching transports
        for(typename map<string, list<Transport<CONTEXT>*> >::iterator
            p = transports.begin(); p!=transports.end(); ++p)
        {
          if (p->first == te.name)
          {
            list<Transport<CONTEXT> *> l = p->second;
            for(typename list<Transport<CONTEXT> *>::iterator
                q = l.begin(); q!=l.end(); ++q)
            {
              Transport<CONTEXT> *trans = *q;
              trans->register_handler(*mh, te);
            }
          }
        }
      }
    }

    return true;
  }

public:
  //--------------------------------------------------------------------------
  // Constructor
  Broker(Init::Registry<Handler<CONTEXT> >& _handler_registry):
    handler_registry(_handler_registry)
  {}

  //--------------------------------------------------------------------------
  // Add a transport
  void add_transport(Transport<CONTEXT> *trans)
  {
    transports[trans->name].push_back(trans);
  }

  //--------------------------------------------------------------------------
  // Configure from XML configuration (<message> element)
  void configure(const XML::Element& config)
  {
    // Create all message handler modules in order
    for(XML::Element::iterator p(config.children); p; ++p)
    {
      const XML::Element& mhe = *p;
      if (!mhe.name.empty() && !create_handler(mhe))
      {
        Log::Error log;
        log << "Failed to create message handler from XML:\n" << mhe;
      }
    }
  }

  //--------------------------------------------------------------------------
  // Shutdown cleanly
  void shutdown()
  {
    // Delete all transports first since they depend on the handlers
    for(typename map<string, list<Transport<CONTEXT> *> >::iterator
        p = transports.begin(); p != transports.end(); ++p)
    {
      list<Transport<CONTEXT> *> l = p->second;
      for(typename list<Transport<CONTEXT> *>::iterator
          q = l.begin(); q!=l.end(); ++q)
        delete *q;
    }
    transports.clear();

    // Then the handlers
    for(typename list<Handler<CONTEXT> *>::iterator
        p = handlers.begin(); p!=handlers.end(); ++p)
      delete *p;
    handlers.clear();
  }

  //--------------------------------------------------------------------------
  // Destructor
  ~Broker()
  {
    shutdown();
  }
};


//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_MSG_H


















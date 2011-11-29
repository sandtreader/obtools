//==========================================================================
// ObTools::Message: broker.cc
//
// Implementation of message broker
//
// Copyright (c) 2010 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-msg.h"
#include "ot-log.h"

namespace ObTools { namespace Message {

//--------------------------------------------------------------------------
// Add a transport
void Broker::add_transport(Transport *trans)
{
  transports[trans->name].push_back(trans);
}

//--------------------------------------------------------------------------
// Configure from XML configuration (<messages> element)
void Broker::configure(XML::Element& config)
{
  // Create all message handler modules in order
  for(XML::Element::iterator p(config.children); p; ++p)
  {
    XML::Element& mhe = *p;
    if (!mhe.name.empty() && !create_handler(mhe))
    {
      Log::Error log;
      log << "Failed to create message handler from XML:\n" << mhe;
    }
  }
}

//--------------------------------------------------------------------------
// Create a message handler from the given configuration element
bool Broker::create_handler(XML::Element& xml)
{
  Handler *mh = handler_registry.create(xml.name, xml);
  if (!mh) return false;

  // Store it
  handlers.push_back(mh);

  // Register in all transports with child elements in this one
  for(XML::Element::iterator p(xml.children); p; ++p)
  {
    XML::Element& te = *p;
    if (!te.name.empty())
    {
      bool matched = false;

      // Look for all matching transports
      for(map<string, list<Transport *> >::iterator p = transports.begin();
          p!=transports.end(); ++p)
      {
        if (p->first == te.name)
        {
          list<Transport *> l = p->second;
          for(list<Transport *>::iterator q = l.begin(); q!=l.end(); ++q)
          {
            Transport *trans = *q;
            trans->register_handler(*mh, te);
            matched = true;
          }
        }
      }

      if (!matched)
      {
	Log::Error log;
	log << "No matching message transport: " << te.name << endl;
      }
    }
  }

  return true;
}


//--------------------------------------------------------------------------
// Shutdown cleanly
void Broker::shutdown()
{
  // Delete all transports first since they depend on the handlers
  for(map<string, list<Transport *> >::iterator p = transports.begin();
      p != transports.end(); ++p)
  {
    list<Transport *> l = p->second;
    for(list<Transport *>::iterator q = l.begin(); q!=l.end(); ++q)
      delete *q;
  }
  transports.clear();

  // Then the handlers
  for(list<Handler *>::iterator p = handlers.begin(); p!=handlers.end(); ++p)
    delete *p;
  handlers.clear();
}

//--------------------------------------------------------------------------
// Destructor
Broker::~Broker()
{
  shutdown();
}



}} // namespaces

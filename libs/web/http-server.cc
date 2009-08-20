//==========================================================================
// ObTools::Web: http-server.cc
//
// Core HTTP server implementation - subclassed to implement handlers
//
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-web.h"
#include "ot-text.h"
#include <sstream>

namespace ObTools { namespace Web {

//==========================================================================
// Generic HTTP server

//--------------------------------------------------------------------------
// Implementation of worker process method
// Called in its own thread
void HTTPServer::process(SSL::TCPSocket& s, SSL::ClientDetails&client)
{
  Log::Streams log;
  OBTOOLS_LOG_IF_DEBUG(log.debug << "HTTP: Connection from " 
		       << client << endl;)

  // Enable keepalives
  s.enable_keepalive();

  // Also set timeout on the socket, in case the client unexpectedly disappears
  s.set_timeout(timeout);

  bool persistent = false;

  try
  {
    ObTools::Net::TCPStream ss(s);

    // Possible loop in persistent connections
    do
    {
      ObTools::Web::HTTPMessage request, response;

      // Try to read a message, stop if not (probably connection dropped)
      // Allow EOF to mark end of body in absence of Content-Length
      if (!request.read(ss, true))
      {
	if (persistent)
	  log.detail << "Persistent connection from " << client 
		     << " now closed\n";
	else
	  log.error << "Can't read HTTP request from socket\n";
	return;
      }

      // Log request
      log.detail << request.version << " request: " << request.method 
		 << " from " << client << " for " 
		 << request.url << endl;
      OBTOOLS_LOG_IF_DEBUG(
        log.debug << request.headers.xml;
        if (request.body.size()) 
	  log.debug << "Body:\n" << request.body << endl;
      )

      // Set version to reflect client
      response.version = request.version;
      
      // Add our own advert
      if (version.size()) response.headers.put("server", version);
    
      // Add date
      response.headers.put_date();

      // Check version
      if (request.version == "HTTP/1.0" || request.version == "HTTP/1.1")
      {
	string conn_hdr = Text::tolower(request.headers.get("connection"));

	// Check for HTTP/1.1
	if (request.version == "HTTP/1.1")
	{
	  // Check for Connection: close - otherwise, assume persistent
	  if (conn_hdr == "close")
	  {
	    if (persistent) 
	      log.detail << "HTTP/1.1 persistent connection from " 
			 << client << " closed\n";
	    else
	      log.detail << "HTTP/1.1 non-persistent connection\n";
	    persistent = false;
	  }
	  else
	  {
	    if (persistent)
	      log.detail << "HTTP/1.1 persistent connection from "
			 << client << " continues\n";
	    else
	      log.detail << "HTTP/1.1 persistent connection started\n";
	    persistent = true;
	  }
	}
	else
	{
	  // Check for old-style HTTP/1.0 Keep-Alive
	  if (conn_hdr == "keep-alive")
	  {
	    if (persistent)
	      log.detail << "HTTP/1.0 persistent connection from "
			 << client << " continues\n";
	    else
	      log.detail << "HTTP/1.0 persistent connection started\n";

	    // Reflect it back in response
	    response.headers.put("connection", "Keep-Alive");

	    persistent = true;
	  }
	  else
	  {
	    // We stop
	    if (persistent) 
	      log.detail << "HTTP/1.0 persistent connection from " 
			 << client << " closed\n";
	    else
	      log.detail << "HTTP/1.0 non-persistent connection\n";
	    persistent = false;
	  }
	}

	// Be optimistic - saves handler doing it for simple cases
	response.code = 200;
	response.reason = "OK";
      
	// Call down to subclass implementation
	if (!handle_request(request, response, client))
	{
	  log.error << "Handler failed - sending 500\n";
	  error(response, 500, "Server Failure");
	}

	// Suppress body if a HEAD request - saves simple handlers having to
	// worry about it
	if (request.method == "HEAD") response.body.clear();
      }
      else
      {
	response.version = "HTTP/1.1";
	error(response, 505, "HTTP Version not supported");
      }

      // Log response
      log.detail << "Response: " << response.code << " " 
		 << response.reason << endl;
      OBTOOLS_LOG_IF_DEBUG(
        log.debug << response.headers.xml;
        if (response.body.size()) 
	  log.debug << "Body:\n" << response.body << endl;
	)

      // Send out response
      if (!response.write(ss)) log.error << "HTTP response failed\n";
      ss.flush();
    }
    while (persistent);

    // Shut down socket
    s.shutdown();
  }
  catch (ObTools::Net::SocketError se)
  {
    log.error << se << endl;
  }
} 

//==========================================================================
// Simple HTTP server with handler registrations

//--------------------------------------------------------------------------
// Implementation of general request handler
bool SimpleHTTPServer::handle_request(HTTPMessage& request, 
				      HTTPMessage& response,
				      SSL::ClientDetails& client)
{
  MT::RWReadLock lock(mutex);

  // Check all handlers for a URL match
  for(list<URLHandler *>::iterator p = handlers.begin(); 
      p!=handlers.end(); ++p)
  {
    URLHandler& h = **p;
    if (Text::pattern_match(h.url, request.url.get_text()))
      return h.handle_request(request, response, client);
  }

  // Not found - 404
  return error(response, 404, "Not found");
}

//--------------------------------------------------------------------------
// Destructor
SimpleHTTPServer::~SimpleHTTPServer()
{
  for(list<URLHandler *>::iterator p = handlers.begin(); 
      p!=handlers.end(); ++p)
    delete *p;
}

}} // namespaces




//==========================================================================
// ObTools::Web: http-server.cc
//
// Core HTTP server implementation - subclassed to implement handlers
//
// Copyright (c) 2006 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-web.h"
#include "ot-text.h"
#include "ot-crypto.h"
#include <sstream>
#include <algorithm>

namespace ObTools { namespace Web {

namespace
{
  const auto websocket_key_guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
}

//==========================================================================
// Generic HTTP server

//--------------------------------------------------------------------------
// Implementation of worker process method
// Called in its own thread
void HTTPServer::process(SSL::TCPSocket& s, const SSL::ClientDetails& client)
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
      bool do_websocket{false};

      // Try to read a message, stop if not (probably connection dropped)
      // Don't wait for EOF in absence of Content-Length, just assume 0
      if (!request.read(ss, false))
      {
        if (persistent)
          log.detail << "Persistent connection from " << client
                     << " now closed\n";
        else
          log.error << "Can't read HTTP request from socket\n";

        handle_close(client, s);
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
      if (!version.empty()) response.headers.put("server", version);

      // Add CORS origin header if set and they supply an Origin
      if (!cors_origin.empty() && request.headers.has("origin"))
        response.headers.put("Access-Control-Allow-Origin", cors_origin);

      // Add date
      response.headers.put_date();

      // Check version
      if (request.version == "HTTP/1.0" || request.version == "HTTP/1.1")
      {
        string conn_hdr = Text::tolower(request.headers.get("connection"));
        const auto conn_opts = Text::split(conn_hdr);

        // Check for HTTP/1.1
        if (request.version == "HTTP/1.1")
        {
          // Check for Connection: close - otherwise, assume persistent
          // WebSocket upgrades are also non-persistent
          if (find(conn_opts.begin(), conn_opts.end(), "close")
              != conn_opts.end() ||
              find(conn_opts.begin(), conn_opts.end(), "upgrade")
              != conn_opts.end())
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
          if (find(conn_opts.begin(), conn_opts.end(), "keep-alive")
              != conn_opts.end())
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

        // Check for OPTIONS request - mainly for CORS
        // (we have already set access-control header above)
        if (request.method == "OPTIONS")
        {
          response.headers.put("Allow", "GET, POST, HEAD");
          response.headers.put("Access-Control-Allow-Headers", "user-agent");
        }
        // Check for WebSocket upgrade
        else if (websocket_enabled
              && request.method == "GET"
              && find(conn_opts.begin(), conn_opts.end(), "upgrade")
              != conn_opts.end()
              && Text::tolower(request.headers.get("upgrade")) == "websocket")
        {
          log.detail << "Upgrade to WebSocket requested\n";
          if (do_websocket_handshake(request, response))
            do_websocket = true;
          else
            error(response, 400, "Bad WebSocket request");
        }
        // In all other cases call down to subclass implementation
        else if (!handle_request(request, response, client, s, ss))
        {
          log.error << "Handler failed - sending 500\n";
          error(response, 500, "Server Failure");
        }
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
      // Suppress body if a HEAD request - saves simple handlers having to
      // worry about it
      if (!response.write(ss, request.method == "HEAD"))
        log.error << "HTTP response failed\n";
      ss.flush();

      // Do websocket if required
      if (do_websocket)
      {
        handle_websocket(request, client, s, ss);
      }
      else
      {
        // Allow subclasses to generate progressive data following on, if
        // required
        generate_progressive(request, response, client, s, ss);
      }
    }
    while (persistent);

    // Clear any connection state
    handle_close(client, s);

    // Shut down socket
    s.shutdown();
  }
  catch (const ObTools::Net::SocketError& se)
  {
    log.error << se << endl;
  }
}

//--------------------------------------------------------------------------
// Implementation of general request handler
bool HTTPServer::do_websocket_handshake(const HTTPMessage& request,
                                        HTTPMessage& response)
{
  Log::Streams log;

  // Check version
  if (request.headers.get("sec-websocket-version") != "13")
  {
    log.error << "Bad WebSocket version\n";
    response.headers.put("Sec-WebSocket-Version", "13");
    return false;
  }

  string key = request.headers.get("sec-websocket-key");

  // Add GUID
  key += websocket_key_guid;

  // SHA1 hash, base64'd
  Crypto::SHA1 sha1;
  string hash = sha1.digest(key);

  Text::Base64 base64;
  response.headers.put("Sec-WebSocket-Accept", base64.encode(hash, 0));

  // Respond positively
  response.code = 101;
  response.reason = "Switching Protocols";
  response.headers.put("Connection", "Upgrade");
  response.headers.put("Upgrade", "websocket");
  return true;
}

//==========================================================================
// Simple HTTP server with handler registrations

//--------------------------------------------------------------------------
// Implementation of general request handler
bool SimpleHTTPServer::handle_request(const HTTPMessage& request,
                                      HTTPMessage& response,
                                      const SSL::ClientDetails& client,
                                      SSL::TCPSocket&, Net::TCPStream&)
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




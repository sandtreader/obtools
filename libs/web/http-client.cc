//==========================================================================
// ObTools::Web: http-client.cc
//
// HTTP client implementation
//
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-web.h"
#include "ot-text.h"
#include "ot-log.h"
#include <sstream>

namespace ObTools { namespace Web {

//--------------------------------------------------------------------------
// Constructor from URL - extracts server from host/port parts
// Handles https if ctx is set
HTTPClient::HTTPClient(URL& url, SSL::Context *_ctx, const string& _ua): 
  user_agent(_ua), ssl_ctx(_ctx)
{
  XML::Element xml;
  if (url.split(xml))
  {
    XML::XPathProcessor xpath(xml);
    string host = xpath["host"];
    
    // Check for HTTPS
    string scheme = xpath["scheme"];
    if (scheme == "https")
    {
      if (!ssl_ctx)
      {
	Log::Streams log;
	log.error << "HTTPS requested but no SSL context given\n";
      }
    }
    else
    {
      // If not HTTPS, drop context so we don't try to use it
      ssl_ctx = 0;
    }

    // Default port depends whether we're SSL or not
    int port = xpath.get_value_int("port", ssl_ctx?443:80);
    server = Net::EndPoint(host, port);
  }
  else
  {
    Log::Streams log;
    log.error << "HTTP: Can't parse URL " << url << " for host\n";
  }
}

//--------------------------------------------------------------------------
// Basic operation - send HTTP message and receive HTTP response
// Whether successfully sent (even if error received)
bool HTTPClient::fetch(HTTPMessage& request, HTTPMessage& response)
{
  Log::Streams log;

  // Remove host from URL and regenerate, to get server-relative one
  XML::Element xml;
  if(!request.url.split(xml))
  {
    log.error << "HTTP: Bad URL " << request.url << endl;
    return false;
  }

  XML::XPathProcessor xpath(xml);

  // Grab host for Host header (HTTP/1.1 like)
  request.headers.put("Host", xpath["host"]);

  // Remove scheme, to suppress host as well
  xpath.delete_elements("scheme");

  // Check path isn't empty - if so, make it '/'
  xpath.ensure_path("path");
  if (xpath["path"].empty()) xpath.set_value("path", "/");

  // Regenerate URL
  request.url = URL(xml);

  OBTOOLS_LOG_IF_DEBUG(log.debug << "HTTP " << request.method << " for " 
		       << request.url << " from " << server << endl;)

  // Force protocol
  request.version = "HTTP/1.0";

  // Add User-Agent and date
  if (user_agent.size()) request.headers.put("User-Agent", user_agent);

  OBTOOLS_LOG_IF_DUMP(request.write(log.dump);)

  // HTTP1.0 - create TCPClient for each fetch
  SSL::TCPClient client(ssl_ctx, server);
  if (!client)
  {
    log.error << "HTTP: Can't connect to " << server << endl;
    return false;
  }

  // Enable reuse and capture local address used, so P2P can turn around
  // and offer a server on here immediately;
  client.enable_reuse();
  last_local_address = client.local();

  try
  {
    Net::TCPStream ss(client);
    if (!request.write(ss))
    {
      log.error << "HTTP: Can't send request to " << server << endl;
      return false;
    }

    // Finish sending so server gets EOF
    ss.flush();
#if !defined(__WIN32__)
    client.finish();  // !!! Seems to cause loss of received data in Vista
                      // !!! and doesn't send FIN anyway
#endif

    // Read, allowing for EOF marker for end of body
    if (!response.read(ss, true))
    {
      log.error << "HTTP: Can't fetch response from " << server << endl;
      return false;
    }

    OBTOOLS_LOG_IF_DUMP(log.dump << "Response:\n";
			response.write(log.dump);)

    client.close();
  }
  catch (ObTools::Net::SocketError se)
  {
    log.error << "HTTP: " << se << endl;
    return false;
  }

  return true;
}

//--------------------------------------------------------------------------
// Simple GET operation on a URL
// Returns result code, fills in body if provided, reason code if not
int HTTPClient::get(URL url, string& body)
{
  HTTPMessage request("GET", url);
  HTTPMessage response;

  if (!fetch(request, response))
  {
    body = "Connection failed";
    return 400;
  }

  // Now extract body, if any
  if (response.body.size())
    body = response.body;
  else
    body = response.reason;

  return response.code;
}

//--------------------------------------------------------------------------
// Simple POST operation on a URL
// Returns result code, fills in response_body if provided, reason code if not
int HTTPClient::post(URL url, const string& request_body,
		     string& response_body)
{
  HTTPMessage request("POST", url);
  request.body = request_body;

  HTTPMessage response;
  if (!fetch(request, response))
  {
    response_body = "Connection failed";
    return 400;
  }

  // Now extract body, if any
  if (response.body.size())
    response_body = response.body;
  else
    response_body = response.reason;

  return response.code;
}

}} // namespaces




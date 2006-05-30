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
HTTPClient::HTTPClient(URL& url, const string& _ua): user_agent(_ua)
{
  XML::Element xml;
  if (url.split(xml))
  {
    XML::XPathProcessor xpath(xml);
    string host = xpath["host"];
    int port = xpath.get_value_int("port", 80);
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

  // Remove scheme, to suppress host as well
  XML::XPathProcessor xpath(xml);
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

  // HTTP1.0 - create TCPClient for each fetch
  Net::TCPClient client(server);
  if (!client)
  {
    log.error << "HTTP: Can't connect to " << server << endl;
    return false;
  }

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
    client.finish();

    // Read, allowing for EOF marker for end of body
    if (!response.read(ss, true))
    {
      log.error << "HTTP: Can't fetch response from " << server << endl;
      return false;
    }

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




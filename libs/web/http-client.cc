//==========================================================================
// ObTools::Web: http-client.cc
//
// HTTP client implementation
//
// Copyright (c) 2006 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-web.h"
#include "ot-text.h"
#include "ot-log.h"
#include <sstream>

namespace ObTools { namespace Web {

//--------------------------------------------------------------------------
// Constructor from URL - extracts server from host/port parts
// Handles https if ctx is set
HTTPClient::HTTPClient(const URL& url, SSL::Context *_ctx, const string& _ua,
		       int _connection_timeout, int _operation_timeout):
  user_agent(_ua), ssl_ctx(_ctx), connection_timeout(_connection_timeout),
  operation_timeout(_operation_timeout), socket(0), stream(0),
  http_1_1(false), http_1_1_close(false), progressive(false), chunked(false),
  current_chunk_length(0),
  progressive_write(false), write_chunk_length(DEFAULT_WRITE_CHUNK_LENGTH),
  cookie_jar(0)
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
// Returns detailed status code
int HTTPClient::do_fetch(HTTPMessage& request, HTTPMessage& response)
{
  Log::Streams log;

  // Remove host from URL and regenerate, to get server-relative one
  XML::Element xml;
  if(!request.url.split(xml))
  {
    log.error << "HTTP: Bad URL " << request.url << endl;
    return 1;
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

  // Set protocol
  request.version = http_1_1?"HTTP/1.1":"HTTP/1.0";

  // Set connection close if this is the last
  if (http_1_1 && http_1_1_close)
    request.headers.put("Connection", "close");

  // Add User-Agent and date
  if (user_agent.size()) request.headers.put("User-Agent", user_agent);

  // Add Authorization header if user set
  string auth_user = xpath["user"];
  if (auth_user.size())
  {
    // Basic auth = user:pass in base64
    string auth_password = xpath["password"];
    if (auth_password.size()) auth_user += ":" + auth_password;
    Text::Base64 base64;
    request.headers.put("Authorization",
			"Basic "+base64.encode(auth_user,0));
  }

  // Add cookies if we have a jar
  if (cookie_jar) cookie_jar->add_cookies_to(request);

  OBTOOLS_LOG_IF_DUMP(request.write(log.dump);)

  // Get a socket if we don't already have one
  if (!socket)
  {
    socket = new SSL::TCPClient(ssl_ctx, server, connection_timeout);

    if (!*socket)
    {
      log.error << "HTTP: Can't connect to " << server << endl;
      delete socket;
      socket = 0;
      return 100;
    }

    // Enable reuse and capture local address used, so P2P can turn around
    // and offer a server on here immediately;
    socket->enable_reuse();
    last_local_address = socket->local();

    // Reset timeout for actual operation as well
    socket->set_timeout(operation_timeout);
  }

  try
  {
    if (!stream) stream = new Net::TCPStream(*socket);
    // If progressive write, just write the headers
    // otherwise write all
    if (progressive_write ? !request.write_headers(*stream)
                          : !request.write(*stream))
    {
      log.error << "HTTP: Can't send request to " << server << endl;
      delete stream; stream = 0;
      delete socket; socket = 0;
      return 201;
    }

    stream->flush();

    if (progressive_write)
      return 0;

    return do_receive(request, response);
  }
  catch (ObTools::Net::SocketError se)
  {
    log.error << "HTTP: " << se << endl;
    delete stream; stream = 0;
    delete socket; socket = 0;
    return 203;
  }

  return 0;
}

//--------------------------------------------------------------------------
// Basic operation - send HTTP message and receive HTTP response
// Whether successfully sent (even if error received)
bool HTTPClient::fetch(HTTPMessage& request, HTTPMessage& response)
{
  return !do_fetch(request, response);
}

//--------------------------------------------------------------------------
// Basic operation - just receive HTTP response
// Returns detailed status code
int HTTPClient::do_receive(HTTPMessage& request, HTTPMessage& response)
{
  if (!socket || !*socket || !stream) return 1;
  Log::Streams log;

  try
  {
    // If progressive, just read the headers
    // otherwise, read all, allowing for EOF marker for end of body
    if (progressive?!response.read_headers(*stream)
                   :!response.read(*stream, true))
    {
      log.error << "HTTP: Can't fetch response from " << server << endl;
      delete stream; stream = 0;
      delete socket; socket = 0;
      return 202;
    }

    OBTOOLS_LOG_IF_DUMP(log.dump << "Response:\n";
                        response.write(log.dump);)

    // Take cookies if we have a jar
    if (cookie_jar) cookie_jar->take_cookies_from(response, request.url);

    if (progressive)
    {
      // Check for chunked encoding
      chunked = Text::tolower(response.headers.get("transfer-encoding"))
                  == "chunked";

      if (chunked)
        current_chunk_length = 0;  // Trigger chunk header read on first fetch
      else            // Capture the file length, if given
        current_chunk_length =
          Text::stoi(response.headers.get("content-length"));

      OBTOOLS_LOG_IF_DEBUG(log.debug << "Progressive download: "
                           << (chunked?"chunked":"continuous")
                           << " length " << current_chunk_length << endl;)
    }
    else if (!http_1_1 || http_1_1_close)
    {
      socket->close();
      delete stream; stream = 0;
      delete socket; socket = 0;
    }
  }
  catch (ObTools::Net::SocketError se)
  {
    log.error << "HTTP: " << se << endl;
    delete stream; stream = 0;
    delete socket; socket = 0;
    return 203;
  }

  return 0;
}

//--------------------------------------------------------------------------
// Get a response body from a response
// Uses just the HTTP reason for errors because servers have a habit of
// dressing up error responses in HTML
string HTTPClient::get_response_body(HTTPMessage& response)
{
  if (response.code < 300 && !response.body.empty())
    return response.body;
  else
    return response.reason;
}

//--------------------------------------------------------------------------
// Simple GET operation on a URL
// Returns result code, fills in body if provided, reason code if not
int HTTPClient::get(const URL& url, string& body)
{
  HTTPMessage request("GET", url);
  HTTPMessage response;

  int result(0);
  if ((result = do_fetch(request, response)))
  {
    body = "Connection failed";
    return -result;
  }

  body = get_response_body(response);
  return response.code;
}

//--------------------------------------------------------------------------
// Simple DELETE operation on a URL
// Returns result code, fills in body if provided, reason code if not
int HTTPClient::del(const URL& url, string& body)
{
  HTTPMessage request("DELETE", url);
  HTTPMessage response;

  int result(0);
  if ((result = do_fetch(request, response)))
  {
    body = "Connection failed";
    return -result;
  }

  body = get_response_body(response);
  return response.code;
}

//--------------------------------------------------------------------------
// Simple POST operation on a URL
// Returns result code, fills in response_body if provided, reason code if not
int HTTPClient::post(const URL& url, const string& request_body,
		     string& response_body)
{
  HTTPMessage request("POST", url);
  request.body = request_body;

  // Set standard form content-type
  request.headers.put("Content-Type", "application/x-www-form-urlencoded");

  HTTPMessage response;
  int result(0);
  if ((result = do_fetch(request, response)))
  {
    response_body = "Connection failed";
    return -result;
  }

  response_body = get_response_body(response);
  return response.code;
}

//--------------------------------------------------------------------------
// Simple PUT operation on a URL
// Returns result code, fills in response_body if provided,
// reason code if not
int HTTPClient::put(const URL& url, const string& content_type,
                    istream& is, string& response_body)
{
  HTTPMessage request("PUT", url);
  request.headers.put("Content-Type", content_type);
  if (progressive_write)
    request.headers.put("Transfer-Encoding", "chunked");

  HTTPMessage response;

  // If not progressive, create the entire body from the stream in advance
  if (!progressive_write)
  {
    vector<char> data(65536);
    while (uint64_t length = is.read(&data[0], data.size()).gcount())
      request.body.append(&data[0], length);
  }

  int result = do_fetch(request, response);
  if (result)
  {
    response_body = "Connection failed";
    return -result;
  }

  if (progressive_write)
  {
    // Write data from input stream
    vector<char> data(write_chunk_length);
    while (uint64_t length = is.read(&data[0], data.size()).gcount())
    {
      if (write(reinterpret_cast<unsigned char *>(&data[0]), length) != length)
        break;
    }

    // Terminated chunked data
    write(0, 0);

    result = do_receive(request, response);
    if (result)
    {
      response_body = "Connection failed";
      return -result;
    }
  }

  response_body = get_response_body(response);
  return response.code;
}

//--------------------------------------------------------------------------
// Read a block of data from a progressive fetch
// Returns the actual amount read
unsigned long HTTPClient::read(unsigned char *data, unsigned long length)
{
  if (!socket || !*socket || !stream) return 0;
  unsigned long n = 0;

  try
  {
    while (n<length) // Could split over multiple chunks
    {
      // Need to read a chunk header?
      if (chunked && !current_chunk_length)
      {
	string line;

	// First line might effectively be blank because it's actually the
	// end of the previous chunk
	if (!MIMEHeaders::getline(*stream, line)
	    || (line.empty() && !MIMEHeaders::getline(*stream, line)))
	  throw Net::SocketError(EOF);

	// Split at ;
	vector<string> bits = Text::split(line, ';');
	current_chunk_length = Text::xtoi(bits[0]);

	// Last chunk?
	if (!current_chunk_length) throw Net::SocketError(EOF);
      }

      // Read up to requested length, limited to that available
      uint64_t wanted = length-n;
      if (current_chunk_length && wanted > current_chunk_length)
	wanted = current_chunk_length;

      // Try to read this much or up to end of stream
      stream->read(reinterpret_cast<char *>(data)+n, wanted);
      ssize_t count = stream->gcount();
      if (!count) throw Net::SocketError(EOF);
      n += count;

      // Count down length only if specified to begin with
      if (current_chunk_length)
      {
	current_chunk_length -= count;

	// If not chunked, and now done, that's it
	if (!chunked && !current_chunk_length) throw Net::SocketError(EOF);
      }
    }

    // Read what was needed
    return n;
  }
  catch (Net::SocketError se)
  {
    // End of file - optionally close socket
    if (!http_1_1 || http_1_1_close)
    {
      socket->close();
      delete stream; stream = 0;
      delete socket; socket = 0;
    }

    return n;
  }
}

//--------------------------------------------------------------------------
// Write a block of data to a progressive upload
// Returns the actual amount written
unsigned long HTTPClient::write(unsigned char *data, unsigned long length)
{
  if (!socket || !*socket || !stream) return 0;

  try
  {
    socket->write(Text::i64tox(length));
    socket->write("\r\n");
    socket->write(data, length);
    socket->write("\r\n");

    // Return what was written
    return length;
  }
  catch (Net::SocketError se)
  {
    // End of file - optionally close socket
    if (!http_1_1 || http_1_1_close)
    {
      socket->close();
      delete stream; stream = 0;
      delete socket; socket = 0;
    }

    return 0;
  }
}


//--------------------------------------------------------------------------
// Destructor
HTTPClient::~HTTPClient()
{
  // Kill any persistent socket left over
  if (stream) delete stream;
  if (socket) delete socket;
}

}} // namespaces




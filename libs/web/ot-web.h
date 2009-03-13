//==========================================================================
// ObTools::Net: ot-web.h
//
// Public definitions for ObTools::Web
// Web protocols parsers, helpers and client/server
// 
// Copyright (c) 2005-2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#ifndef __OBTOOLS_WEB_H
#define __OBTOOLS_WEB_H

#include <stdint.h>
#include <iostream>
#include <string>
#include "ot-xml.h"
#include "ot-misc.h"
#include "ot-net.h"
#include "ot-ssl.h"
#include "ot-mt.h"
#include "ot-log.h"

namespace ObTools { namespace Web { 

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// URL (url.cc)
// Represents a Web URL
//
// Converts to and from an XML format representing the URL structure:
// The XML element has sub-elements (NOT attributes) as follows:
//   scheme    Scheme (e.g. http) - lower cased
//   host      Host/domain (if present) - lower cased
//   port      Port number (if present)
//   path      URL path (including '/' if absolute) (as specified)
//   fragment  Fragment following #
//   query     Query following ?

class URL
{
public:
  string text;

  //--------------------------------------------------------------------------
  // Constructors
  URL() {}
  URL(const string& s): text(s) {}

  //--------------------------------------------------------------------------
  // Constructor from XML format
  // Pass in any element with above sub-elements
  URL(XML::Element& xml);

  //--------------------------------------------------------------------------
  // Get the text of the URL
  string get_text() { return text; }

  //--------------------------------------------------------------------------
  // Clear the URL
  void clear() { text.clear(); }

  //--------------------------------------------------------------------------
  // Split text into XML
  // Pass in an empty element, and this will fill it up as above
  // Name of element is set to 'url'
  // Returns whether successful (valid URL)
  bool split(XML::Element& xml);

  //------------------------------------------------------------------------
  // Quick access to path of URL
  // Returns path or "" if can't read it
  string get_path();

  //------------------------------------------------------------------------
  // Quick access to query of URL
  // Returns query or "" if can't read it
  string get_query();

  //------------------------------------------------------------------------
  // Get query as a property list
  // Returns whether query was available, fills props if so
  // Note: Handles + for space and % decode in values
  bool get_query(Misc::PropertyList& props);

  //------------------------------------------------------------------------
  // Static function to URL-encode (percent-encode) a string
  // Escapes space as '+' if 'space_as_plus' is set (the default)
  static string encode(const string& s, bool space_as_plus=true);

  //------------------------------------------------------------------------
  // Static function to URL-encode (percent-encode) a set of variables
  // (expressed as a PropertyList)
  // Escapes space as '+' if 'space_as_plus' is set (the default)
  static string encode(const Misc::PropertyList& props, 
		       bool space_as_plus=true);

  //------------------------------------------------------------------------
  // Static function to URL-decode (percent-encode) a string
  // Decodes '+' as space if 'space_as_plus' is set (the default)
  static string decode(const string& s, bool space_as_plus=true);

  //------------------------------------------------------------------------
  // Static function to URL-decode (percent-encode) a multi-valued string
  // (x-www-form-urlencoded) into a property list
  // Decodes '+' as space if 'space_as_plus' is set (the default)
  static void decode(const string& s, Misc::PropertyList& props,
		     bool space_as_plus=true);

};

//------------------------------------------------------------------------
// << operator to write URL to ostream
// e.g. cout << url;
ostream& operator<<(ostream& s, const URL& u);

//==========================================================================
// MIME header block (mime-headers.cc)
// Represents a block of MIME headers - e.g. from a mail message, or an HTTP
// request.  
//
// Stores in XML, since we have convenient access & iteration methods for this.
//
// On input, headers are unfolded and each header is added as a sub-element of
// the root 'xml', with name (tag) equal to the header tag **lowercased**, 
// and content equal to the text of the header
//
// On output, header names are generated with first letter capitalised, 
// following convention.  Values longer than 64 characters are folded at 
// commas (if any) or spaces (if any)             

class MIMEHeaders
{
private:
  static const unsigned int MAX_HEADER = 8000;  // Input DoS protection
  static const unsigned int MAX_LINE   = 60;    // For output folding

public:
  XML::Element xml;

  //--------------------------------------------------------------------------
  // Constructor
  MIMEHeaders(): xml("headers") {}

  //--------------------------------------------------------------------------
  // Check for presence of a header
  bool has(const string& name) const
  { return !!xml.get_child(name); }

  //--------------------------------------------------------------------------
  // Get a specific header (first of that name)
  string get(const string& name) const
  { return xml.get_child(name).content; }

  //--------------------------------------------------------------------------
  // Insert a header
  void put(const string& name, const string& value)
  { xml.add(name, value); }

  //--------------------------------------------------------------------------
  // Remove all headers of the given name
  void remove(const string& name)
  { xml.remove_children(name); }

  //--------------------------------------------------------------------------
  // Replace a header
  void replace(const string& name, const string& value)
  { remove(name); xml.add(name, value); }

  //------------------------------------------------------------------------
  // Add a current date header to RFC 822 standard
  void put_date(const string& header="date");

  //--------------------------------------------------------------------------
  // Split multi-value headers at commas
  // Reads all headers of name 'name', and splits any with commas at
  // comma to give a flattened list of values
  list<string> get_all(const string& name);

  //--------------------------------------------------------------------------
  // Split a header value (e.g. from get or get_all) into a prime value
  // and parameters delineated by ';'
  // Parameters without a value are given the value '1'
  // The value is modified to be without parameters

  // Useful for Content-Type (HTTP), Transport (RTSP) etc.
  // e.g.
  //   Content-Type: text/html; charset=ISO-8859-1; pure
  //
  // Leaves 'text/html' in value, and property list:
  //   charset     ISO-8859-1
  //   pure        1
  static Misc::PropertyList split_parameters(string& value);

  //--------------------------------------------------------------------------
  // Parse headers from a stream
  // Returns whether successful
  // Skips the blank line delimiter, leaving stream ready to read message
  // body (if any)
  bool read(istream& in);

  //--------------------------------------------------------------------------
  // Generates headers to a stream
  // Returns whether successful (can only fail if stream fails)
  // Includes the blank line delimiter, leaving stream ready to write message
  // body (if any)
  bool write(ostream& out) const;

  //--------------------------------------------------------------------------
  // Get a line from a stream
  // Returns true if read OK - even if blank
  // Exported for the convenience of HTTP reader - see below
  static bool getline(istream& in, string& s);
};

//------------------------------------------------------------------------
// >> operator to read MIMEHeaders from istream
// e.g. cin >> url;
istream& operator>>(istream& s, MIMEHeaders& mh);

//------------------------------------------------------------------------
// << operator to write MIMEHeaders to ostream
// e.g. cout << url;
ostream& operator<<(ostream& s, const MIMEHeaders& mh);

//==========================================================================
// HTTP message (http-message.cc)
// Represents an HTTP request or response message
// Also usable for other HTTP-like protocols - e.g. RTSP
// Note we represent both request and response in the same structure because
// of bidirectional protocols like RTSP where either client or server might
// receive either at any time
// (We distinguish responses by virtue of the first word containing a '/')

// For requests, members 'method' and 'uri' are set
// For responses, members 'code' and 'reason' are set
// Use the presence of 'method' to distinguish
// Version is always set
// Headers are always read into the 'headers' MIMEHeaders structure
// Body (if any) is read into 'body'

class HTTPMessage
{
private:
  static const unsigned int READ_SIZE = 4096;  

public:
  // Request fields
  string method;
  URL url;

  // Response fields
  int code;
  string reason;

  // Shared fields
  string version;
  MIMEHeaders headers;
  string body;

  //--------------------------------------------------------------------------
  // Basic constructor
  HTTPMessage() {}

  //--------------------------------------------------------------------------
  // Constructors for requests
  HTTPMessage(const string& _method, const URL& _url, 
	      const string& _version = "HTTP/1.0"):
    method(_method), url(_url), version(_version) {}

  HTTPMessage(const string& _method, const string& _url, 
	      const string& _version = "HTTP/1.0"):
    method(_method), url(_url), version(_version) {}

  //--------------------------------------------------------------------------
  // Constructor for responses
  HTTPMessage(int _code, const string& _reason,
	      const string& _version = "HTTP/1.0"):
    code(_code), reason(_reason), version(_version) {}

  //--------------------------------------------------------------------------
  // Check for request (not response)
  bool is_request() { return !method.empty(); }

  //--------------------------------------------------------------------------
  // Read request/response and headers from a stream
  // Leave stream ready to read body (if any)
  // Returns whether successful
  bool read_headers(istream &in);

  //--------------------------------------------------------------------------
  // Read from a stream
  // Returns whether successful
  // If read_to_eof is set, it will try to read a body up to EOF if there
  // is no Content-Length header present.  Set this for HTTP, but not for
  // RTSP, where the stream continues and lack of Content-Length reliably
  // means lack of body
  bool read(istream &in, bool read_to_eof = false);

  //--------------------------------------------------------------------------
  // Write request/response and headers to a stream
  // Returns whether successful
  bool write_headers(ostream &out) const;

  //--------------------------------------------------------------------------
  // Write to a stream
  // Returns whether successful
  bool write(ostream &out) const;
};

//------------------------------------------------------------------------
// >> operator to read HTTPMessage from istream
// e.g. cin >> url;
istream& operator>>(istream& s, HTTPMessage& msg);

//------------------------------------------------------------------------
// << operator to write HTTPMessage to ostream
// e.g. cout << url;
ostream& operator<<(ostream& s, const HTTPMessage& msg);

//==========================================================================
// HTTP Client class (http-client.cc)
// Currently simple HTTP/1.0 client - one fetch per connection - but
// interface allows for a persistent HTTP/1.1 socket later
class HTTPClient
{
private:
  string user_agent;    // String to quote in User-Agent: header
  Net::EndPoint last_local_address;  // Address we connected from last, for P2P
  SSL::Context *ssl_ctx; // Optional SSL context

protected:
  Net::EndPoint server;

public:
  //--------------------------------------------------------------------------
  // Constructor from server
  HTTPClient(Net::EndPoint _server, const string& _ua=""): 
    user_agent(_ua), ssl_ctx(0), server(_server) {}

  //--------------------------------------------------------------------------
  // Constructor for SSL
  HTTPClient(Net::EndPoint _server, SSL::Context *_ctx, const string& _ua=""): 
    user_agent(_ua), ssl_ctx(_ctx), server(_server) {}

  //--------------------------------------------------------------------------
  // Constructor from URL - extracts server from host/port parts
  // Handles https if ctx is set
  HTTPClient(URL& url, SSL::Context *_ctx=0, const string& _ua="");

  //--------------------------------------------------------------------------
  // Basic operation - send HTTP message and receive HTTP response
  // Whether successfully sent (even if error received)
  bool fetch(HTTPMessage& request, HTTPMessage& response);

  //--------------------------------------------------------------------------
  // Simple GET operation on a URL
  // Returns result code, fills in body if provided, reason code if not
  int get(URL url, string& body);

  //--------------------------------------------------------------------------
  // Simple POST operation on a URL
  // Returns result code, fills in response_body if provided, 
  // reason code if not
  int post(URL url, const string& request_body, string& response_body);

  //--------------------------------------------------------------------------
  // Get the local address we last connected from (for P2P)
  Net::EndPoint get_last_local_address() { return last_local_address; }
};

//==========================================================================
// HTTP Server abstract class (http-server.cc)
// Multi-threaded server for HTTP - manages HTTP protocol state, and 
// passes request messages to subclasses
class HTTPServer: public SSL::TCPServer
{
private:
  int timeout;    // Socket inactivity timeout
  string version; // Version reported in Server: header

  //--------------------------------------------------------------------------
  // Implementation of worker process method
  void process(SSL::TCPSocket &s, SSL::ClientDetails& client);

protected:
  //--------------------------------------------------------------------------
  // Helper to generate error in response, and log it
  bool error(Web::HTTPMessage& response, int code, const string& reason)
  {
    Log::Streams log;
    log.error << "HTTP error: " << code << " " << reason << endl;
    response.code = code; response.reason = reason; return true;
  }

  //--------------------------------------------------------------------------
  // Abstract interface to handle requests
  // Return whether handled - fill in response for normal errors, only
  // return false if things are really bad, and we'll return 500
  // response is pre-initialised with 200 OK, no body
  // If the cost of generating a body is not too great, handlers may treat
  // HEAD requests just like GET, and the server will suppress the body
  virtual bool handle_request(HTTPMessage& request, HTTPMessage& response,
			      SSL::ClientDetails& client) = 0;

public:
  //--------------------------------------------------------------------------
  // Constructor to bind to any interface (basic TCP)
  // See ObTools::Net::TCPServer for details of threadpool management
  HTTPServer(int port=80, const string& _version="", int backlog=5, 
	     int min_spare=1, int max_threads=10, int _timeout=90):
    SSL::TCPServer(0, port, backlog, min_spare, max_threads),
    timeout(_timeout), version(_version) {}

  //--------------------------------------------------------------------------
  // Constructor to bind to specific address (basic TCP)
  // See ObTools::Net::TCPServer for details of threadpool management
  HTTPServer(Net::EndPoint address, const string& _version="", int backlog=5, 
	     int min_spare=1, int max_threads=10, int _timeout=90):
    SSL::TCPServer(0, address, backlog, min_spare, max_threads),
    timeout(_timeout), version(_version) {}

  //--------------------------------------------------------------------------
  // Constructor to bind to any interface, with SSL
  // See ObTools::Net::TCPServer for details of threadpool management
  HTTPServer(SSL::Context *ctx, 
	     int port=80, const string& _version="", int backlog=5, 
	     int min_spare=1, int max_threads=10, int _timeout=90):
    SSL::TCPServer(ctx, port, backlog, min_spare, max_threads),
    timeout(_timeout), version(_version) {}

  //--------------------------------------------------------------------------
  // Constructor to bind to specific address, with SSL
  // See ObTools::Net::TCPServer for details of threadpool management
  HTTPServer(SSL::Context *ctx,
	     Net::EndPoint address, const string& _version="", int backlog=5, 
	     int min_spare=1, int max_threads=10, int _timeout=90):
    SSL::TCPServer(ctx, address, backlog, min_spare, max_threads),
    timeout(_timeout), version(_version) {}

  //--------------------------------------------------------------------------
  // Virtual destructor
  virtual ~HTTPServer() {}
};

//==========================================================================
// URL handler - registers for pattern matched URLs and handles them
// Subclass to implement handler
class URLHandler
{
public:
  string url;   // URL (patterns allowed)

  //--------------------------------------------------------------------------
  // Constructor
  URLHandler(const string& _url): url(_url) {}

  //--------------------------------------------------------------------------
  // Abstract interface to handle requests
  // Return whether handled - fill in response for normal errors, only
  // return false if things are really bad, and we'll return 500
  // response is pre-initialised with 200 OK, no body
  virtual bool handle_request(HTTPMessage& request, HTTPMessage& response,
			      SSL::ClientDetails& client) = 0;

  //--------------------------------------------------------------------------
  // Virtual destructor
  virtual ~URLHandler() {}
};

//==========================================================================
// Simple HTTP server which just fields GET and/or POST requests to a list 
// of registered URLs (http-server.cc)
// Handlers are checked in order, so later ones can be defaults
class SimpleHTTPServer: public HTTPServer
{
  MT::RWMutex mutex;               // Around global state
  list<URLHandler *> handlers;

  // Implementation of general request handler
  bool handle_request(HTTPMessage& request, HTTPMessage& response,
		      SSL::ClientDetails& client);

public:
  //--------------------------------------------------------------------------
  // Constructor on all interfaces, basic TCP
  SimpleHTTPServer(int port=80, const string& _version="", int backlog=5, 
		   int min_spare=1, int max_threads=10, int _timeout=90):
    HTTPServer(port, _version, backlog, min_spare, max_threads, _timeout),
    mutex() {}

  //--------------------------------------------------------------------------
  // Constructor for specific address, basic TCP
  SimpleHTTPServer(Net::EndPoint address, const string& _version="", 
		   int backlog=5, 
		   int min_spare=1, int max_threads=10, int _timeout=90):
    HTTPServer(address, _version, backlog, min_spare, max_threads, _timeout),
    mutex() {}

  //--------------------------------------------------------------------------
  // Constructor on all interfaces, with SSL
  SimpleHTTPServer(SSL::Context *ctx,
		   int port=80, const string& _version="", int backlog=5, 
		   int min_spare=1, int max_threads=10, int _timeout=90):
    HTTPServer(ctx, port, _version, backlog, min_spare, max_threads, _timeout),
    mutex() {}

  //--------------------------------------------------------------------------
  // Constructor for specific address, basic TCP
  SimpleHTTPServer(SSL::Context *ctx,
		   Net::EndPoint address, const string& _version="", 
		   int backlog=5, 
		   int min_spare=1, int max_threads=10, int _timeout=90):
    HTTPServer(ctx, address, _version, backlog, min_spare, max_threads, 
	       _timeout), mutex() {}

  //--------------------------------------------------------------------------
  // Add a handler - will be deleted on destruction of server
  void add(URLHandler *h) 
  { MT::RWWriteLock lock(mutex); handlers.push_back(h); }

  //--------------------------------------------------------------------------
  // Remove a handler
  void remove(URLHandler *h) 
  { MT::RWWriteLock lock(mutex); handlers.remove(h); }

  //--------------------------------------------------------------------------
  // Destructor
  ~SimpleHTTPServer();
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_WEB_H




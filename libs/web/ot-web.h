//==========================================================================
// ObTools::Net: ot-web.h
//
// Public definitions for ObTools::Web
// Web protocols parsers, helpers and client/server
//
// Copyright (c) 2005-2012 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
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
#include "ot-file.h"

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
//
// Constructor and split do *not* encode/decode % and + - this must be
// done separately for each element beforehand, if required
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
  string get_text() const { return text; }
  string str() const { return text; }

  //--------------------------------------------------------------------------
  // Clear the URL
  void clear() { text.clear(); }

  //--------------------------------------------------------------------------
  // Split text into XML
  // Pass in an empty element, and this will fill it up as above
  // Name of element is set to 'url'
  // Returns whether successful (valid URL)
  bool split(XML::Element& xml) const;

  //------------------------------------------------------------------------
  // Quick access to scheme of URL
  // Returns host or "" if there isn't one
  string get_scheme() const;

  //------------------------------------------------------------------------
  // Quick access to host of URL
  // Returns host or "" if can't read it
  string get_host() const;

  //------------------------------------------------------------------------
  // Quick access to path of URL
  // Returns path or "" if can't read it
  string get_path() const;

  //------------------------------------------------------------------------
  // Quick access to query of URL
  // Returns query or "" if can't read it
  string get_query() const;

  //------------------------------------------------------------------------
  // Quick access to fragment of URL
  // Returns fragment or "" if can't read it
  string get_fragment() const;

  //------------------------------------------------------------------------
  // Get query as a property list
  // Returns whether query was available, fills props if so
  // Note: Handles + for space and % decode in values
  bool get_query(Misc::PropertyList& props) const;

  //------------------------------------------------------------------------
  // Get an individual parameter from query
  // Returns parameter value or "" if not present
  string get_query_parameter(const string& name) const;

  //------------------------------------------------------------------------
  // Resolve against a base URL
  // Returns the resolved URL
  // Handles absolute, relative and .. forms
  URL resolve(const URL& base) const;

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
  // Get all headers of name 'name'
  list<string> get_all(const string& name) const;

  //--------------------------------------------------------------------------
  // Split multi-value headers at commas
  // Reads all headers of name 'name', and splits at delimiter to give a
  // flattened list of values
  list<string> get_all_splitting(const string& name, char delimiter=',') const;

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
  static const unsigned int MAX_FIRST_LINE = 8000;  // Input DoS protection

  bool get_first_line(istream& in, string& s);

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
  bool write(ostream &out, bool headers_only = false) const;

  // The following cookie support is for server-side use - for client side
  // see CookieJar below

  //--------------------------------------------------------------------------
  // Set a cookie with the given optional domain, path and expiry time
  void set_cookie(const string& name, const string& value,
                  const string& path = "",
                  const string& domain = "",
                  Time::Stamp expires = Time::Stamp(),
                  bool secure = false,
                  bool http_only = false);

  //--------------------------------------------------------------------------
  // Get a map of all cookies, name value pairs in values_p
  void get_cookies(map<string, string>& values_p);

  //--------------------------------------------------------------------------
  // Get a single cookie value, or empty if not set
  string get_cookie(const string& name);
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
// Representation of a cookie (cookies.cc)
struct Cookie
{
  // Basic
  string name;
  string value;

  // Attributes
  Time::Stamp expires;
  string path;
  string domain;
  bool http_only;
  bool secure;

  // Storage internals
  Time::Stamp created;
  Web::URL origin;

  //--------------------------------------------------------------------------
  // Constructors
  Cookie(): http_only(false), secure(false) {}
  Cookie(const string& _name, const string& _value):
    name(_name), value(_value), http_only(false), secure(false) {}

  //--------------------------------------------------------------------------
  // Comparison (for evicting existing)
  bool operator==(const Cookie& c)
  { return name==c.name && domain==c.domain && path==c.path; }

  //--------------------------------------------------------------------------
  // Read from a Set-Cookie header value
  // Returns whether valid cookie read
  bool read_from(const string& header_value);

  //--------------------------------------------------------------------------
  // Output as a string, including attributes if attrs is set
  string str(bool attrs=false) const;
};

//==========================================================================
// Client cookie jar (cookies.cc)
class CookieJar
{
  MT::RWMutex mutex;
  list<Cookie> cookies;

 public:
  //--------------------------------------------------------------------------
  // Constructor
  CookieJar() {}

  //--------------------------------------------------------------------------
  // Get number of cookies available
  int count() { return cookies.size(); }

  //--------------------------------------------------------------------------
  // Take cookies from the given server response
  void take_cookies_from(const HTTPMessage& response, const URL& origin);

  //--------------------------------------------------------------------------
  // Add cookies to the given client request
  void add_cookies_to(HTTPMessage& request);

  //--------------------------------------------------------------------------
  // Prune expired cookies from the jar, including session cookies if session
  // ended
  void prune(bool session_ended=false);

  //--------------------------------------------------------------------------
  // Dump the cookie jar to the given stream
  void dump(ostream& sout);
};

//==========================================================================
// HTTP Client class (http-client.cc)
// Currently simple HTTP/1.0 client - one fetch per connection - but
// interface allows for a persistent HTTP/1.1 socket later
class HTTPClient
{
private:
  static const uint64_t DEFAULT_WRITE_CHUNK_LENGTH = 65536;

  string user_agent;    // String to quote in User-Agent: header
  Net::EndPoint last_local_address;  // Address we connected from last, for P2P
  SSL::Context *ssl_ctx; // Optional SSL context
  int connection_timeout;   // Connection timeout (s), disabled (0) by default
  int operation_timeout;    // Ditto, for operation
  SSL::TCPClient *socket;   // Our socket, created on first use, may persist
                            // if HTTP/1.1
  Net::TCPStream *stream;   // TCP stream on the socket
  bool http_1_1;            // Whether HTTP/1.1 is used (persistent conn's)
  bool http_1_1_close;      // Whether this the last request on persistent conn

  // Progressive download: caller fetches blocks at a time
  bool progressive;
  bool chunked;                   // Chunked read (progressive only)
  uint64_t current_chunk_length;  // Current length remaining of chunk

  // Progressive write: caller sends blocks at a time
  bool progressive_write;
  uint64_t write_chunk_length;    // Size of written chunks

  // Cookie support
  CookieJar *cookie_jar;

protected:
  Net::EndPoint server;

public:
  //--------------------------------------------------------------------------
  // Constructor from server
  HTTPClient(Net::EndPoint _server, const string& _ua="",
	     int _connection_timeout=0, int _operation_timeout=0):
    user_agent(_ua), ssl_ctx(0), connection_timeout(_connection_timeout),
    operation_timeout(_operation_timeout), socket(0), stream(0),
    http_1_1(false), http_1_1_close(false), progressive(false), chunked(false),
    current_chunk_length(0),
    progressive_write(false), write_chunk_length(DEFAULT_WRITE_CHUNK_LENGTH),
    cookie_jar(0),
    server(_server) {}

  //--------------------------------------------------------------------------
  // Constructor for SSL
  HTTPClient(Net::EndPoint _server, SSL::Context *_ctx, const string& _ua="",
	     int _connection_timeout=0, int _operation_timeout=0):
    user_agent(_ua), ssl_ctx(_ctx), connection_timeout(_connection_timeout),
    operation_timeout(_operation_timeout), socket(0), stream(0),
    http_1_1(false), http_1_1_close(false), progressive(false), chunked(false),
    current_chunk_length(-1),
    progressive_write(false), write_chunk_length(DEFAULT_WRITE_CHUNK_LENGTH),
    cookie_jar(0),
    server(_server) {}

  //--------------------------------------------------------------------------
  // Constructor from URL - extracts server from host/port parts
  // Handles https if ctx is set
  HTTPClient(const URL& url, SSL::Context *_ctx=0, const string& _ua="",
	     int _connection_timeout=0, int _operation_timeout=0);

  //--------------------------------------------------------------------------
  // Enable HTTP/1.1 persistent connections
  void enable_persistence() { http_1_1 = true; }

  //--------------------------------------------------------------------------
  // Force close on next request on persistent connections
  void close_persistence() { http_1_1_close = true; }

  //--------------------------------------------------------------------------
  // Enable progressive download - initial fetch is just for headers, then
  // call read() to get data
  void enable_progressive() { progressive=true; }

  //--------------------------------------------------------------------------
  // Disable progressive download
  void disable_progressive() { progressive=false; }

  //--------------------------------------------------------------------------
  // Enable progressive upload - initial fetch is just for headers, then
  // call write() to get data
  void enable_progressive_upload(unsigned long chunk_length
                                 = DEFAULT_WRITE_CHUNK_LENGTH)
  {
    write_chunk_length = chunk_length;
    progressive_write = true;
  }

  //--------------------------------------------------------------------------
  // Disable progressive upload
  void disable_progressive_upload() { progressive_write = false; }

  //--------------------------------------------------------------------------
  // Set cookie jar (referenced, not taken) = 0 to disable cookies
  void set_cookie_jar(CookieJar *jar) { cookie_jar = jar; }

  //--------------------------------------------------------------------------
  // Basic operation - send HTTP message and receive HTTP response
  // Returns detailed status code
  int do_fetch(HTTPMessage& request, HTTPMessage& response);

  //--------------------------------------------------------------------------
  // Basic operation - send HTTP message and receive HTTP response
  // Returns whether successfully sent (even if error received)
  bool fetch(HTTPMessage& request, HTTPMessage& response);

  //--------------------------------------------------------------------------
  // Basic operation - just receive HTTP response
  // Returns detailed status code
  int do_receive(HTTPMessage& request, HTTPMessage& response);

  //--------------------------------------------------------------------------
  // Simple GET operation on a URL
  // Returns result code, fills in body if provided, reason code if not
  int get(const URL& url, string& body);

  //--------------------------------------------------------------------------
  // Simple DELETE operation on a URL
  // Returns result code, fills in body if provided, reason code if not
  int del(const URL& url, string& body);

  //--------------------------------------------------------------------------
  // Simple POST operation on a URL
  // Returns result code, fills in response_body if provided,
  // reason code if not
  int post(const URL& url, const string& request_body, string& response_body);

  //--------------------------------------------------------------------------
  // Simple PUT operation on a URL
  // Returns result code, fills in response_body if provided,
  // reason code if not
  int put(const URL& url, const string& content_type,
          istream& is, string& response_body);

  //--------------------------------------------------------------------------
  // Read a block of data from a progressive fetch
  // Returns the actual amount read
  unsigned long read(unsigned char *data, unsigned long length);

  //--------------------------------------------------------------------------
  // Write a block of data to a progressive upload
  // Returns the actual amount written
  unsigned long write(unsigned char *data, unsigned long length);

  //--------------------------------------------------------------------------
  // Get the local address we last connected from (for P2P)
  Net::EndPoint get_last_local_address() { return last_local_address; }

  //--------------------------------------------------------------------------
  // Destructor
  ~HTTPClient();
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
  string cors_origin;  // Pattern for Access-Control-Allow-Origin header

  //--------------------------------------------------------------------------
  // Implementation of worker process method
  void process(SSL::TCPSocket &s, const SSL::ClientDetails& client);

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
  virtual bool handle_request(const HTTPMessage& request,
                              HTTPMessage& response,
			      const SSL::ClientDetails& client,
			      SSL::TCPSocket& socket,
			      Net::TCPStream& stream) = 0;

  //--------------------------------------------------------------------------
  // Interface to generate progressive data after initial response headers
  // and initial body (if any)
  // Simply continue sending data to the socket or stream until done
  // Does nothing by default
  virtual void generate_progressive(const HTTPMessage& /*request*/,
                                    HTTPMessage& /*response*/,
				    const SSL::ClientDetails& /*client*/,
				    SSL::TCPSocket& /*socket*/,
        		            Net::TCPStream& /*stream*/) {}

  //--------------------------------------------------------------------------
  // Interface to clear per-connection state
  // Does nothing by default
  virtual void handle_close(const SSL::ClientDetails& /*client*/,
                            SSL::TCPSocket& /*socket*/) {}

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
  // Set origin pattern for CORS (Access-Control-Allow-Origin header)
  // pattern defaults to '*' = any origin
  void set_cors_origin(const string& pattern = "*")
  {
    cors_origin = pattern;
  }

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
  virtual bool handle_request(const HTTPMessage& request,
                              HTTPMessage& response,
			      const SSL::ClientDetails& client) = 0;

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
  bool handle_request(const HTTPMessage& request, HTTPMessage& response,
		      const SSL::ClientDetails& client, SSL::TCPSocket& socket,
		      Net::TCPStream& stream);

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
  // Constructor for specific address, with SSL
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
// HTTP cache
// Maintains a directory with a subdirectory for each domain, then MD5-ed
// Currently ignores Expires etc. and simply keeps for the cache_time given
// URLs for filenames
class Cache
{
  File::Directory directory;
  SSL::Context *ssl_ctx;
  string user_agent;

  // Internal
  bool get_paths(const URL& url, File::Directory& domain_dir_p,
		 File::Path& file_path_p, File::Path& status_path_p);

 public:
  //--------------------------------------------------------------------------
  // Constructor
  // UA is used if specified, otherwise a default is used
  Cache(const File::Directory& _dir, SSL::Context *_ssl_ctx = 0,
	const string& _ua="");

  //--------------------------------------------------------------------------
  // Fetch a file from the given URL, or from cache
  // If check_for_updates is set, uses conditional GET to check whether a
  // new version exists, if the item's update-time has passed since last check
  // Returns whether file is available, writes file location to path_p if so
  bool fetch(const URL& url, File::Path& path_p, bool check_for_updates=false);

  //--------------------------------------------------------------------------
  // Fetch an object from the given URL, or from cache, as a string
  // Returns whether file was fetched, writes file contents to contents_p if so
  bool fetch(const URL& url, string& contents_p, bool check_for_updates=false);

  //--------------------------------------------------------------------------
  // Set the update check interval for a given URL
  // interval is in Time::Duration constructor format
  // (URL must already have been fetched)
  bool set_update_interval(const URL& url, const string& interval);

  //--------------------------------------------------------------------------
  // Clear any cached file for a given URL
  void forget(const URL& url);

  //--------------------------------------------------------------------------
  // Update the cache in background
  // Runs a single time through the entire cache, checking for updates on files
  // with update intervals set
  void update();

};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_WEB_H




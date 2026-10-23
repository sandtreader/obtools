//==========================================================================
// ObTools::Web: compression.cc
//
// Gzip compression of HTTP responses
//
// Copyright (c) 2026 Paul Clark.
//==========================================================================

#include "ot-web.h"
#include "ot-text.h"

namespace ObTools { namespace Web {

namespace
{
  //------------------------------------------------------------------------
  // Get a header with its given name or lower-cased, as read from the wire
  string get_header(const MIMEHeaders& headers, const string& name)
  {
    if (headers.has(name)) return headers.get(name);
    return headers.get(Text::tolower(name));
  }

  //------------------------------------------------------------------------
  // Check whether a header is present with its given name or lower-cased
  bool has_header(const MIMEHeaders& headers, const string& name)
  {
    return headers.has(name) || headers.has(Text::tolower(name));
  }

  //------------------------------------------------------------------------
  // Check whether a media type is already compressed
  bool is_compressed_type(const string& content_type)
  {
    auto type = Text::tolower(content_type);
    const auto semi = type.find(';');
    if (semi != string::npos) type.erase(semi);
    type = Text::canonicalise_space(type);

    if (type == "image/svg+xml") return false;
    return !type.compare(0, 6, "image/")
        || !type.compare(0, 6, "video/")
        || !type.compare(0, 6, "audio/")
        || type == "application/zip"
        || type == "application/gzip"
        || type == "application/x-gzip";
  }
}

//--------------------------------------------------------------------------
// Gzip compress a response body in place if suitable - see ot-web.h
// Returns whether it was compressed
bool compress_response(const HTTPMessage& request, HTTPMessage& response,
                       size_t min_size, int level)
{
  if (!min_size || response.body.size() < min_size) return false;
  if (request.method == "HEAD") return false;
  if (!request.accepts_encoding("gzip")) return false;

  // Leave anything already encoded or with an explicit length (which may
  // be followed by progressive data)
  if (has_header(response.headers, "Content-Encoding")
   || has_header(response.headers, "Content-Length")
   || has_header(response.headers, "Transfer-Encoding"))
    return false;

  if (is_compressed_type(get_header(response.headers, "Content-Type")))
    return false;

  string compressed;
  if (!GZip::compress(response.body, compressed, level)) return false;
  response.body.swap(compressed);
  response.headers.put("Content-Encoding", "gzip");

  // Add to Vary, keeping its case if read from the wire
  const auto vary_name = (!response.headers.has("Vary")
                          && response.headers.has("vary")) ? "vary" : "Vary";
  const auto vary = response.headers.get(vary_name);
  response.headers.replace(vary_name,
                           vary.empty() ? "Accept-Encoding"
                                        : vary + ", Accept-Encoding");
  return true;
}

}} // namespaces

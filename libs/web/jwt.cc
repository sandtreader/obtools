//==========================================================================
// ObTools::Web: jwt.cc
//
// JSON Web Token implementation
//
// Copyright (c) 2021 Paul Clark
//==========================================================================

#include "ot-web.h"
#include "ot-log.h"
#include "ot-crypto.h"

namespace ObTools { namespace Web {

//------------------------------------------------------------------------
// Construct from a 3 part string
JWT::JWT(const string& text)
{
  vector<string> bits = Text::split(text, '.');
  if (bits.size() == 3)
  {
    header_b64 = bits[0];
    payload_b64 = bits[1];
    signature_b64 = bits[2];

    Text::Base64URL base64;
    try
    {
      string header_json;
      if (!base64.decode(header_b64, header_json))
      {
        Log::Error log;
        log << "Bad base64URL in JWT header: " << header_b64 << endl;
        return;
      }

      istringstream iss1(header_json);
      JSON::Parser parser1(iss1);
      header = parser1.read_value();

      string payload_json;
      if (!base64.decode(payload_b64, payload_json))
      {
        Log::Error log;
        log << "Bad base64URL in JWT payload: " << payload_b64 << endl;
        return;
      }
      istringstream iss2(payload_json);
      JSON::Parser parser2(iss2);
      payload = parser2.read_value();

    }
    catch (const JSON::Exception& e)
    {
      Log::Error log;
      log << "JWT JSON parse failed: " << e.error << endl;
    }
  }
  else
  {
    Log::Error log;
    log << "Bad JWT format " << text << endl;
  }
}

//------------------------------------------------------------------------
// Construct from JSON payload for writing
JWT::JWT(const JSON::Value& _payload): payload(_payload)
{
  // Standard header
  header = JSON::Value(JSON::Value::OBJECT);
  header.set("typ", "JWT");
  header.set("alg", "HS256");

  // Encode header & payload
  Text::Base64URL base64;
  header_b64 = base64.encode(header.str());
  payload_b64 = base64.encode(payload.str());
}

//------------------------------------------------------------------------
// Verify signature
bool JWT::verify(const string& secret)
{
  if (header["typ"].as_str() != "JWT")
  {
    Log::Error log;
    log << "Bad JWT type " << header["typ"].as_str() << endl;
    return false;
  }

  if (header["alg"].as_str() != "HS256")
  {
    Log::Error log;
    log << "Unimplemented JWT algorithm " << header["alg"].as_str() << endl;
    return false;
  }

  Crypto::HMACSHA256 hs256(secret);
  string digest = hs256.digest(header_b64+"."+payload_b64);
  Text::Base64URL base64;
  if (signature_b64 != base64.encode(digest))
  {
    Log::Error log;
    log << "Bad JWT signature\n";
    return false;
  }

  return true;
}

//------------------------------------------------------------------------
// Get expiry
Time::Stamp JWT::get_expiry()
{
  auto expiry = payload["exp"].as_int();
  if (expiry)
    return Time::Stamp(expiry);
  else
    return Time::Stamp();
}

//------------------------------------------------------------------------
// Check expiry
bool JWT::expired()
{
  // Is expiry specified?  Check it
  auto expiry = payload["exp"].as_int();
  if (expiry && time(NULL) > expiry) return true;
  return false;
}

//------------------------------------------------------------------------
// Sign it
void JWT::sign(const string& secret)
{
  Crypto::HMACSHA256 hs256(secret);
  string digest = hs256.digest(header_b64+"."+payload_b64);
  Text::Base64URL base64;
  signature_b64 = base64.encode(digest);
}

//------------------------------------------------------------------------
// Get string version
string JWT::str()
{
  return header_b64 + "." + payload_b64 + "." + signature_b64;
}



}} // namespaces




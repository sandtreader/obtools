<?php
// Native PHP implementation of ot-xmlmesh client
// Cpoyright (c) Paul Clark 2008.  All rights reserved.
// This code comes with NO WARRANTY and is subject to licence agreement

// Replaces previous C module

//------------------------------------------------------------------------
//Globals
$xmlmesh_host = "localhost";
$xmlmesh_port = 29167;
$xmlmesh_timeout = 5;
$xmlmesh_last_error = "";
$xmlmesh_socket = null;  // Persistent socket within page only

//------------------------------------------------------------------------
//Internal - send/receive XML message, return (optional) result in response
//Returns whether completed
function _xmlmesh_transaction($subject, $request, &$response, $rsvp)
{
  global $xmlmesh_host, $xmlmesh_port, $xmlmesh_timeout, $xmlmesh_last_error;
  global $xmlmesh_socket;

  // Open TCP socket to server if not already open
  if (!$xmlmesh_socket)
    $xmlmesh_socket = fsockopen($xmlmesh_host, $xmlmesh_port, $errno, $errstr, 
				$xmlmesh_timeout);
  $fp = $xmlmesh_socket;
  if ($fp) 
  {
    // Set timeout for response
    stream_set_timeout($fp, $xmlmesh_timeout);

    // Invent unique ID
    $id = uniqid(rand(), true);

    // Check RSVP flag
    $xrsvp = $rsvp?" x:rsvp='true'":"";

    // Build SOAP envelope
    $soap = "<env:Envelope xmlns:env='http://www.w3.org/2003/05/soap-envelope'"
          . " xmlns:x='http://obtools.com/ns/xmlmesh'>\n"
          . "  <env:Header>\n"
          . "    <x:routing env:mustUnderstand='true' env:relay='true'"
          . "     env:role='http://www.w3.org/2003/05/soap-envelope/role/next'"
          . "     x:id='$id' x:subject='$subject'$xrsvp/>\n"
          . "  </env:Header>\n"
          . "  <env:Body>\n"
          . "    $request\n"
          . "  </env:Body>\n"
          . "</env:Envelope>\n";

    // Write OTMP header
    fwrite($fp, "OTMS");                     // Tag
    fwrite($fp, pack("N", strlen($soap)));   // Length
    fwrite($fp, pack("N", 0));               // Flags
    fwrite($fp, $soap);                      // Message

    // If not requiring a response, that's it
    if (!$rsvp) return true;

    // Read OTMP header back
    $header = "";
    $len = 0;
    while ($len < 12 && !feof($fp)) 
    {
      $header .= fread($fp, 12-$len);
      $len = strlen($header);
    }

    if ($len < 12) 
    {
      $xmlmesh_last_error = "Truncated header in response";
      fclose($fp);
      $xmlmesh_socket = null;
      return false;
    }

    $harr = unpack("a4tag/Nlength/Nflags", $header);

    if ($harr["tag"] != "OTMS")
    {
      $xmlmesh_last_error = "Bad tag in response";
      fclose($fp);
      $xmlmesh_socket = null;
      return false;
    }

    $length = $harr["length"];

    $response = "";
    $len = 0;
    while ($len < $length && !feof($fp)) 
    {
      $response .= fread($fp, $length-$len);
      $len = strlen($response);
    }

    if ($len < $length) 
    {
      $xmlmesh_last_error = "Truncated message in response";
      fclose($fp);
      $xmlmesh_socket = null;
      return false;
    }
    $dom = new DOMDocument();
    $dom->loadXML($response);
    if (!$dom)
    {
      $xmlmesh_last_error = "Can't parse XML result";
      return true;
    }
    $xpath = new DOMXPath( $dom );
    $xpath->registerNamespace( "env","http://www.w3.org/2003/05/soap-envelope");
    $xpath->registerNamespace( "x",  "http://obtools.com/ns/xmlmesh");
    // Look for a Fault response
    $nodes = $xpath->query("//env:Fault/env:Reason/env:Text");
    if ($nodes->length)
    {
      $item = $nodes->item(0);
      $xmlmesh_last_error = $item->nodeValue;
    }

    return true;
  }
  else
  {
    $xmlmesh_last_error = $errstr;
    return false;
  }
}

//------------------------------------------------------------------------
//Send a message with no response
//Returns whether successfully sent
function xmlmesh_send($subject, $message)
{
  $response = "";
  return _xmlmesh_transaction($subject, $message, $response, false);
}

//------------------------------------------------------------------------
//Send a request and get a response
//Returns full response XML (including SOAP)
//Returns empty string and sets xmlmesh_last_error if error
function xmlmesh_request($subject, $request)
{
  $response = "";
  if (_xmlmesh_transaction($subject, $request, $response, true))
    return $response;
  else
    return "";
}

//------------------------------------------------------------------------
//Send a request and check for simple xmlmesh.ok response
//Returns whether successful.  xmlmesh_last_error is set if any error
function xmlmesh_simple_request($subject, $request)
{
  global $xmlmesh_last_error;
  $response = "";
  if (_xmlmesh_transaction($subject, $request, $response, true))
  {
    // Create XML DOM & xpath
    if ( phpversion() >= "5" )
    {
      $dom = new DOMDocument();
      $dom->loadXML($response);
      if (!$dom)
      {
        $xmlmesh_last_error = "Can't parse XML result";
        return false;
      }
      $xpath = new DOMXPath( $dom );
      $xpath->registerNamespace( "env","http://www.w3.org/2003/05/soap-envelope");
      $xpath->registerNamespace( "x",  "http://obtools.com/ns/xmlmesh");
      $nodes = $xpath->query("//x:ok");
      if ($nodes->length)
      {
        return true;
      }
      else
      {
        // Look for a Fault response
        $nodes = $xpath->query("//env:Fault/env:Reason/env:Text");

        if ($nodes->length)
	{
	  $item = $nodes->item(0);
  	  $xmlmesh_last_error = $item->nodeValue;
	}
        else
  	  $xmlmesh_last_error = "Unknown fault";
      
        return false;
      }
    }
    else 
    {
      $dom = domxml_open_mem($response);
      if (!$dom)
      {
        $xmlmesh_last_error = "Can't parse XML result";
        return false;
      }

      // Look for an <x:ok> response
      $xpath = $dom->xpath_new_context();
      xpath_register_ns($xpath, "env","http://www.w3.org/2003/05/soap-envelope");
      xpath_register_ns($xpath, "x",  "http://obtools.com/ns/xmlmesh");
      $obj = $xpath->xpath_eval("//x:ok");

      if ($obj->nodeset)
      {
        return true;
      }
      else
      {
        // Look for a Fault response
        $obj = $xpath->xpath_eval("//env:Fault/env:Reason/env:Text");
        $nodes = $obj->nodeset;
        if ($nodes) 
    	  $xmlmesh_last_error = $nodes[0]->get_content();
        else
	  $xmlmesh_last_error = "Unknown fault";
      
        return false;
      }
    }
  }
  else return false;
}

?>

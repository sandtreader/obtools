package ObTools::XMLMesh;
use strict;
use warnings;

# Native perl implementation of ot-xmlmesh client
# Copyright (c) Paul Clark 2017.  All rights reserved.
# This code comes with NO WARRANTY and is subject to licence agreement

use Exporter qw(import);
use IO::Socket;
use Data::UUID;
use XML::LibXML;

our @EXPORT_OK = qw(send request simple_request); 

our $xmlmesh_host = "localhost";
our $xmlmesh_port = 29167;
our $xmlmesh_timeout = 5;
our $xmlmesh_last_error = "";

# ------------------------------------------------------------------------
# Internal - send/receive XML message, return (optional) result in response
# Returns whether completed
sub _xmlmesh_transaction
{
  my ($subject, $request, $rsvp) = @_;
  my $xmlmesh_socket;

  $xmlmesh_last_error = "";

  # Open TCP socket to server if not already open
  if (!$xmlmesh_socket) {
    $xmlmesh_socket = IO::Socket::INET->new(
      Proto    => 'tcp',
      PeerPort => $xmlmesh_port,
      PeerAddr => $xmlmesh_host,
      Timeout => $xmlmesh_timeout,
    ) or die "Could not create socket: $!\n";
  }

  my $fp = $xmlmesh_socket;

  if ($fp)
  {
    # Invent unique ID
    my $uniqid = Data::UUID->new;
    my $id = $uniqid->to_string($uniqid->create());
    $id =~ s/-//g;

    # Check RSVP flag
    my $xrsvp = $rsvp?" x:rsvp='true'":"";

    # Build SOAP envelope
    my $soap = "<env:Envelope xmlns:env='http://www.w3.org/2003/05/soap-envelope'"
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

    # Write OTMP header
    print $fp  "OTMS";                     # Tag
    print $fp pack("N", length($soap));   # Length
    print $fp  pack("N", 0);               # Flags
    print $fp $soap;                      # Message

    # If not requiring a response, that's it
    if (!$rsvp) {
      undef $xmlmesh_socket;
      return 1;
    }

    # Read OTMP header back
    my $header = "";
    my $len = 0;
    my $append = "";
    while ($len < 12 && !eof($fp))
    {
      read($fp,$append,12-$len);
      $header .= $append;
      $len = length($header);
    }

    if ($len < 12)
    {
      $xmlmesh_last_error = "Truncated header in response";
      close($fp);
      undef $xmlmesh_socket;
      return 0;
    }

		my ($tag, $resplength, $flags) = unpack("a4NN", $header);

    if ($tag ne "OTMS")
    {
      $xmlmesh_last_error = "Bad tag in response";
      close($fp);
      undef $xmlmesh_socket;
      return 0;
    }

    my $length = $resplength;

    my $response = "";
    $len = 0;
    $append = "";
    while ($len < $length && !eof($fp))
    {
      read($fp,$append,$length-$len,$len);
      $response .= $append;
      $len = length($response);
    }

    if ($len < $length)
    {
      $xmlmesh_last_error = "Truncated message in response";
      close($fp);
      undef $xmlmesh_socket;
      return 0;
    }
    my $dom = XML::LibXML->load_xml(string => $response);

    if (!$dom)
    {
      $xmlmesh_last_error = "Can't parse XML result";
      return 0;
    }

    my $xpath = XML::LibXML::XPathContext->new($dom);
    $xpath->registerNs( "env","http://www.w3.org/2003/05/soap-envelope");
    $xpath->registerNs( "x",  "http://obtools.com/ns/xmlmesh");

    #  Look for a Fault response
    my @nodes = $xpath->findnodes("//env:Fault/env:Reason/env:Text");
    if ($nodes[0]) {
      $xmlmesh_last_error .= $nodes[0]->textContent();
      return 0;
    }

    return 1;
  }
  else
  {
    $xmlmesh_last_error = "Socket error";
    return 0;
  }
}

# ------------------------------------------------------------------------
# Send a message with no response
# Returns whether successfully sent
sub send
{
  my ($subject, $message) = @_;
  return _xmlmesh_transaction($subject, $message, 0);
}

# ------------------------------------------------------------------------
# Send a request and get a response
# Returns full response XML (including SOAP)
# Returns empty string and sets xmlmesh_last_error if error
sub request
{
  my ($subject, $request) = @_;
  my $response = _xmlmesh_transaction($subject, $request, 1);
  return $response if $response;
  return "";
}

# ------------------------------------------------------------------------
# Send a request and check for simple xmlmesh.ok response
# Returns whether successful.  xmlmesh_last_error is set if any error
sub simple_request
{
  my ($subject,$request) = @_;
  my $response = _xmlmesh_transaction($subject, $request,1);
  return $response;
}
1;

<?php

require_once("../ot-xmlmesh.php");

class TestOtxmlmesh extends PHPUnit_Framework_TestCase
{
  public function setup()
  {
    global $xmlmesh_host,$xmlmesh_last_error;
    $xmlmesh_host = "testhost";
  }

  public function testGlobalHostWorks()
  {
    global $xmlmesh_host;
    $xmlmesh_host = "unrouteable";
    $emessage = "";
    try {
       $result = xmlmesh_request("foo", "<foo/>");
    } catch (Exception $e)
    {
      $emessage = $e->getMessage();
    }
    $this->assertEquals("fsockopen(): php_network_getaddresses: getaddrinfo failed: Name or service not known",$emessage);
  }

  public function testSimpleRequestFail()
  {
    global $xmlmesh_host,$xmlmesh_last_error;
    $xmlmesh_host = "testhost";
    $result = xmlmesh_simple_request("foo", "<foo/>");
    $this->assertEquals(false,$result);
  }

  public function testSimpleRequestFailErrorSet()
  {
    global $xmlmesh_host,$xmlmesh_last_error;
    $xmlmesh_host = "testhost";
    $result = xmlmesh_simple_request("foo", "<foo/>");
    $this->assertEquals("Nothing to handle this request",$xmlmesh_last_error);
  }

  public function testRequestFail()
  {
    global $xmlmesh_host,$xmlmesh_last_error;
    $xmlmesh_host = "testhost";
    $result = xmlmesh_request("foo", "<foo/>");
    $this->assertEquals(false,$result);
  }

  public function testRequestFailErrorSet()
  {
    global $xmlmesh_host,$xmlmesh_last_error;
    $xmlmesh_host = "testhost";
    $result = xmlmesh_request("foo", "<foo/>");
    $this->assertEquals("Nothing to handle this request",$xmlmesh_last_error);
  }

  public function testSimpleRequest()
  {
    global $xmlmesh_host,$xmlmesh_last_error;
    $xmlmesh_host = "testhost";
    $testb = time();
    $req = "<ps:add-bundle parent='orbiss/test' name='$testb'/>";
    $result = xmlmesh_simple_request("packetship.cache.modify.request", $req);
    $this->assertEquals(true,$result);
    $this->assertEquals("",$xmlmesh_last_error);
    return $testb;
  }

  /**
   *  @depends testSimpleRequest
  **/
  public function testRequest($testb)
  {
    global $xmlmesh_host,$xmlmesh_last_error;
    $xmlmesh_host = "testhost";
    $req = "<ps:cache-info-request><ps:bundle id='orbiss/test/$testb'><ps:metadata/><ps:content/></ps:bundle></ps:cache-info-request>";
    $result = xmlmesh_request("packetship.cache.info.request", $req);
    $this->assertEquals("",$xmlmesh_last_error);
    $this->assertRegExp("/id=\"orbiss\/test\/$testb\"/",$result);
  }
}
?>

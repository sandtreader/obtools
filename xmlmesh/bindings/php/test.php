<?php
  include ("ot-xmlmesh.php");
    global $xmlmesh_host;
    $xmlmesh_host = "testhost";

  if (xmlmesh_simple_request("foo", "<foo/>"))
    print "OK\n";
  else
    print "Error: $xmlmesh_last_error\n";

  if (xmlmesh_request("foo", "<foo/>"))
    print "OK\n";
  else
    print "Error: $xmlmesh_last_error\n";

  if (xmlmesh_request("foo", "<foo/>"))
    print "OK\n";
  else
    print "Error: $xmlmesh_last_error\n";
?>

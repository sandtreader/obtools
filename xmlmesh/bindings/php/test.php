<?
  include ("ot-xmlmesh.php");

  if (xmlmesh_simple_request("foo", "<foo/>"))
    print "OK\n";
  else
    print "Error: $xmlmesh_last_error\n";
?>

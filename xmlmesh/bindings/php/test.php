<?
  $result = xmlmesh_request("this.isnt.going.to.work", "<foo/>");
  header("Content-type: text/plain");
  print $result;
?>

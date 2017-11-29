#!/usr/bin/perl -w

use strict;
use ObTools::XMLMesh;

my $subject = "packetship.location.alarm.triggered";

my $response = ObTools::XMLMesh::send($subject,"<ps:alarm xmlns:ps='http://packetship.com/ns' active='true'/>");
sleep(3);
$response = ObTools::XMLMesh::send($subject,"<ps:alarm xmlns:ps='http://packetship.com/ns' active='false'/>");
print $ObTools::XMLMesh::xmlmesh_last_error."\n";

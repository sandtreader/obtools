#!/usr/bin/perl -w

if ($#ARGV < 0)
{
  print <<EOF;
Licence adder (c) xMill Consulting 2003

Pipe filter, adds licence terms to files containing // @@@ markers

Usage:
  cat file | add-licence <licence-text-file> [<licensee>] > file.new
EOF
  exit;
}

$licencefile = shift;
$licensee = shift;

open LFILE, $licencefile or die "Can't open licence file";

while (<>)
{
    if (/^[\/\#]{1,2} @@@/)
    {
      @line = split;
      $comment = $line[0];
      while (<LFILE>)
      {
	  s/\@LICENSEE\@/$licensee/g;
	  print "$comment ", $_;
      }
    }
    else
    {
	print;
    }
}


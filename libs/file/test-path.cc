//==========================================================================
// ObTools::File: test-path.cc
//
// Test harness for file library path manipulations
//
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-file.h"
#include <iostream>

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  string p = "D:\\\350\257\267\350\276\223\345\205\245\345\205\263\351\224\256\345\255\227\\CarrotPurse/purse.cfg.xml";

  if (argc >= 2) p = argv[1];
  File::Path path(p);
  cout << "Path: " << path << endl;
  cout << "  is_absolute: " << (path.is_absolute()?"Yes":"No") << endl;
  cout << "      dirname: " << path.dirname() << endl;
  cout << "     leafname: " << path.leafname() << endl;
  cout << "    extension: " << path.extension() << endl;
  cout << "     basename: " << path.basename() << endl;
  cout << "       exists: " << (path.exists()?"Yes":"No") << endl;
  cout << "       is_dir: " << (path.is_dir()?"Yes":"No") << endl;
  cout << "     readable: " << (path.readable()?"Yes":"No") << endl;
  cout << "    writeable: " << (path.writeable()?"Yes":"No") << endl;
  cout << "       length: " << path.length() << endl;
  cout << "         mode: " << File::Path::itoo(path.mode()) << endl;
#if !defined(__WIN32__)
  cout << "        owner: " << File::Path::user_id_to_name(path.owner()) << endl;
  cout << "        group: " << File::Path::group_id_to_name(path.group()) << endl;
#endif

  // Try to read it
  File::InStream f(p);
  if (f)
    cout << "File opens OK\n";
  else
    cout << "File won't open\n";
  
  // Check for resolve request
  if (argc >= 3)
  {
    File::Path path2(argv[2]);
    cout << "\nResolving: " << path2  << endl;
    
    File::Path rpath = path.resolve(path2);
    cout << "Resolved to: " << rpath << endl;
  }

  return 0;  
}





//==========================================================================
// ObTools::Web:: test-url.cc
//
// Test harness for URL functions
//
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-web.h"
using namespace std;

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  if (argc < 2) 
  {
    cout << "Supply a URL!\n";
    return 2;
  }

  string urls(argv[1]);
  ObTools::Web::URL url(urls);

  cout << "\n---\nSplitting URL: " << url << endl;

  ObTools::XML::Element root("url");
  if (!url.split(root))
  {
    cerr << "Parse failed\n";
    return 2;
  }

  cout << "\n--- XML form\n";
  cout << root;

  cout << "Path: " << url.get_path() << endl;
  cout << "Query: " << url.get_query() << endl;
  ObTools::Misc::PropertyList props;
  url.get_query(props);
  cout << "Split query:\n";
  props.dump(cout);

  cout << "Re-encoded query:\n";
  cout << url.encode(props) << endl;

  cout << "\n--- Regenerated\n";
  ObTools::Web::URL url2(root);
  cout << url2 << endl;
 
  return 0;  
}





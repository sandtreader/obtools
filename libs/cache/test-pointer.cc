//==========================================================================
// ObTools::Cache: test-pointer.cc
//
// Test harness for pointer cache
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-cache.h"
using namespace std;
using namespace ObTools;
using namespace ObTools::Cache;

//--------------------------------------------------------------------------
// Main

class Wombat
{
public:
  string name;
  int n;

  Wombat(const string& _name, int _n): name(_name), n(_n) {}
};

ostream& operator<<(ostream& s, const Wombat& w)
{
  s << "Wombat '" << w.name << "'(" << w.n << ")";
}

int main()
{
  LRUEvictionPointerCache<string, Wombat> cache(5);

  cache.add("foo", new Wombat("Foo", 1));
  cache.add("bar", new Wombat("Bar", 2));

  // Iterator cache 
  for(LRUEvictionPointerCache<string, Wombat>::iterator p = cache.begin();
      p!=cache.end();
      ++p)
  { cout << p.id() << ": " << *p << endl; }
  cout << endl;

  Wombat *f = cache.lookup("foo");
  if (f)
  {
    cout << "Foo: " << *f << endl;
    cache.remove("foo");
  }
  else
    cout << "No Foo\n";

  Wombat *b = cache.detach("bar");
  if (b)
  {
    cout << "Bar: " << *b << endl;
    if (cache.contains("bar"))
      cout << "Bar still there!\n";
    else
      cout << "Bar detached OK\n";
    delete b;
  }
  else
    cout << "No Bar\n";

  return 0;  
}





//==========================================================================
// ObTools::Net: test-single-server.cc
//
// Test harness for TCP single-threaded server functions
//
// Copyright (c) 2007 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-net.h"
#include <stdlib.h>

using namespace std;

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  int port = 11111;
  if (argc > 1) port = atoi(argv[1]);

#if defined(__WIN32__)
  winsock_initialise();
#endif

  cout << "Starting server on port " << port << endl;
  ObTools::Net::TCPSingleServer server(port);

  for(;;)
  {
    ObTools::Net::TCPSocket *s = server.wait(5);

    if (s)
    {
      ObTools::Net::EndPoint client = s->remote();
      cout << "Connection from " << client << endl;

      try
      {
        // Just output and reflect what we get
        string buf;
        while (*s >> buf)
        {
          cout << buf;
          *s << "<< " << buf << "\n";
        }

        cout << "Connection from " << client << " ended\n";
      }
      catch (ObTools::Net::SocketError se)
      {
        cerr << se << endl;
        return 2;
      }

      delete s;
    }
    else
    {
      cerr << "Can't listen on port " << port << endl;
      return 4;
    }
  }

  return 0;
}





//==========================================================================
// ObTools::Net: test-socket.cc
//
// Test harness for network library socket functions
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-net.h"

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    cerr << "Give a hostname\n" << endl;
    return 2;
  }

  char *host = argv[1];

  ObTools::Net::IP_Address addr(host);
  if (!addr)
  {
    cerr << "Can't resolve host: " << host << endl;
    return 1;
  }

  cout << "Host: " << addr << " (" << addr.get_hostname() << ")" << endl;

  ObTools::Net::TCP_Client client(addr, 80);

  if (!client)
  {
    cerr << "Can't connect to host\n";
    return 1;
  }

  try
  {
    // Write HTTP request
    char *out = "GET / HTTP/1.0\r\n\r\n";
    client.write(out, strlen(out));

    char buf[81];
    int size;
    while ((size = client.read(buf, 80)) > 0)
    {
      buf[size] = 0;
      cout << buf;
    }
  }
  catch (ObTools::Net::SocketError se)
  {
    cerr << "Socket error: " << strerror(se.error) << endl;
    return 1;
  }
   
  return 0;  
}





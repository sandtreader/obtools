//==========================================================================
// ObTools::Net: winsock.cc
//
// Starting up and closing down WinSock - Windows only, of course
//
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#if defined(__WIN32__)
#include "ot-net.h"

//--------------------------------------------------------------------------
// Winsock shutdown
static void winsock_shutdown()
{
  WSACleanup();
}

//--------------------------------------------------------------------------
// Winsock initialisation
bool winsock_initialise()
{
  WORD version = MAKEWORD(2,0);
  WSADATA wsa_data;
  int err = WSAStartup(version, &wsa_data);

  if (err) return false;

  atexit(winsock_shutdown);
  return true;
}

#endif

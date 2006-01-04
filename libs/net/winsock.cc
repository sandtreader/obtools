//==========================================================================
// ObTools::Net: winsock.cc
//
// Starting up and closing down WinSock - Windows only, of course
//
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#if !defined(__WIN32__)
#error Do not use this except in Windows builds
#endif

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


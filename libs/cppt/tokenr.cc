//==========================================================================
// ObTools::CPPT: tokenr.cc
//
// Lexical token recogniser - used to spot script tags in source
// Matches input against a list of possible tokens, attempts to find
// longest possible match
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-cppt.h"
using namespace ObTools::CPPT;

//------------------------------------------------------------------------
//Process a character
//Returns whether a token is available - note this will happen on the
//character _after_ the end of the token
//State of token validity also returned
bool TokenRecogniser::process_char(char c, TokenState& state)
{
  int npossibles = 0;
  bool at_least_one_is_valid = false;

  // See whether this character is the 'index'th character of any possible
  // token
  for(list<string>::iterator p = tokens.begin();
      p!=tokens.end();
      p++)
  {
    string::size_type size = p->size();
    if (size > index && (*p)[index] == c)
    {
      npossibles++;

      //Would this be complete?
      if (size == index+1) at_least_one_is_valid = true;
    }
  }

  // Default state
  state = TOKEN_READING;

  if (npossibles)
  {
    // Clear token on first character
    if (!index++) current_token = "";

    // Keep this character
    current_token += c;

    if (at_least_one_is_valid) 
    { 
      // If only one possiblity, we're done
      if (npossibles == 1) 
      {
	state = TOKEN_VALID;
	index = 0;  // Clear for next time
	longest_valid = 0;
      }
      else
	// Remember we had this, in case we have to backtrack
	longest_valid=index;
    }

    return true;
  }
  else
  {
    // Not in any token - first simple check to see if anything happened 
    // before, and just carry on reading if not
    if (!index) return false;

    // Clear index for next time
    index = 0;

    // We're into a string but did we ever catch anything valid before?
    if (longest_valid)
    {
      // Some of it was valid.
      // NOTE:  We assume this must be the last character, not some time
      // ago - in other words, valid tokens where one is a substring of the
      // other must only differ by a single character
      state = TOKEN_VALID;
      longest_valid = 0;
      return false;
    }
    else
    {
      // It's all bogus - hand it back
      state = TOKEN_INVALID;
      return false;
    }
  }
}





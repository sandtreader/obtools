//==========================================================================
// ObTools::Text: pattern.cc
//
// Pattern 'glob' functions 
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-text.h"
#include <ctype.h>

namespace ObTools { namespace Text {

//--------------------------------------------------------------------------
// Matches a pattern against a string
// Returns whether it matches
// Pattern can contain:
//	*	Matches any number of characters, or none
//	?	Matches a single character
//	[abc]	Matches any character in set. Ranges x-y allowed
//	[!abc]  Matches	any character not in set.  Ranges allowed
//	\	Escapes following character special character
//
//	   cased gives whether case sensitive match (true)
bool pattern_match(const char *pattern, const char *text, bool cased)
{
  const char *p, *q;
  char c, d;

  p = pattern;
  q = text;
  
  for (;;)
  { 
    c = *p++;
    d = *q++;
    
    if (!cased)
    {
      c = ::tolower(c);
      d = ::tolower(d);
    }
    
    switch (c)
    {
      case '\0': 		/* Finished - text must have finished too */
        return (bool)!d;

      case '\\': 		/* Literal - must match next character */
        c = *p++;
        if (!cased) c = ::tolower(c);
        if (c!=d) return false;
        break;

      case '?':			/* Single char - musn't have finished text */
        if (!d) return false;
        break;
        
      case '*':			/* Span - match up to next pattern character,
      				   or recurse if another special */
        c = *p;
        if (c != '\\' && c != '?' && c != '*' && c != '[')
        {
          if (!cased) c = ::tolower(c);
          while (d != c)
          {
            if (!d) return false;  /* Ran out of text */
            d = *q++;
            if (!cased) d = ::tolower(d);
          }
        }
        
        /* Recurse to ensure we can match the rest of the pattern somewhere
           along here */
        q--;
        do
        {
          if (pattern_match(p, q, cased)) return true;
        } while (*q++);

        return false;

      case '[':
      {
        const char *endp;
        bool invert, found;

        endp = p;
        if (*endp == '!') endp++;

        for (;;)
        {
          if (*endp == '\0') goto dft;        /* no matching ] */
          if (*endp == '\\') endp++;
          if (*++endp == ']') break;
        }
        invert = false;
        if (*p == '!')
        {
          invert=true;
          p++;
        }
        found = false;
        c = *p++;
          
        do
        {
          if (c == '\\') c = *p++;
          if (!cased) c = ::tolower(c);

          if (*p == '-' && p[1] != ']')
          { 
            char e;
            if (*++p == '\\') p++;
              
            e=*p;
            if (!cased) e = ::tolower(e);
            if (d >= c && d <= e) found = true;

            p++;
          }
          else
          {
            if (d == c) found = true;
          }
        } while ((c = *p++) != ']');

        if (found == invert) return false;
        break;
      }

      dft:
      default:
        if (d != c) return false;
        break;
    }
  }
}

// More C++ friendly version
bool pattern_match(const string& pattern, const string& text, bool cased)
{ 
  return pattern_match(pattern.c_str(), text.c_str(), cased); 
}


}} // namespaces

//==========================================================================
// ObTools::CLI: group.cc
//
// Command Group implementation
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-cli.h"
#include "ot-text.h"
#include <ctype.h>

namespace ObTools { namespace CLI {

//------------------------------------------------------------------------
//Add a command
void CommandGroup::add(Command *command)
{
  string t = Text::tolower(command->word);  // Lower case for comparison
  t = Text::canonicalise_space(t);          // Sort out spaces, if any

  // Look for first word, and remainder - if there is a remainder, it's
  // a multi-part thing, and we need to create a command-group for it
  string word = Text::remove_word(t);

  if (t.size())
  {
    CommandGroup *group = 0;

    // Look for existing group
    map<string, Command *>::iterator p = commands.find(word);
    if (p!=commands.end())
    {
      CommandGroup *cg = dynamic_cast<CommandGroup *>(p->second);
      if (cg) group=cg;
    }

    // Create group if not already there
    if (!group)
    {
      group = new CommandGroup(word);
      commands[word] = group;
    }

    // Remove prefix and add to group
    command->word = t;
    group->add(command);
  }
  else commands[word] = command;
}

//------------------------------------------------------------------------
//Handle a command
void CommandGroup::handle(string args, istream& sin, ostream& sout)
{
  // Find first word of args
  string word = Text::remove_word(args);

  // Lower-case it for comparison (makes command words case-insensitive)
  word = Text::tolower(word);

  // Look it up
  map<string, Command *>::iterator p = commands.find(word);
  if (p!=commands.end())
  {
    Command *c = p->second;
    c->handle(args, sin, sout);
  }
  else if (word=="?" || word=="help")
  {
    // Show help
    show_help(sout);
  }
  else
  {
    sout << "Unrecognised command: " << word << endl;
  }
}

//------------------------------------------------------------------------
//List help for the group
void CommandGroup::show_help(ostream& sout)
{
  size_t maxword = 0;

  // Find length of longest word
  for(map<string, Command *>::iterator p = commands.begin();
      p!=commands.end();
      ++p)
  {
    Command *c = p->second;
    size_t n = c->word.size();
    if (n > maxword) maxword=n;
  }

  for(map<string, Command *>::iterator p = commands.begin();
      p!=commands.end();
      ++p)
  {
    Command *c = p->second;
    string pad(maxword-c->word.size()+4, ' ');
    sout << c->word;
    if (c->help.size()) sout << pad << c->help;
    sout << endl;
  }
}

//------------------------------------------------------------------------
//Destructor
CommandGroup::~CommandGroup()
{
  for(map<string, Command *>::iterator p = commands.begin();
      p!=commands.end();
      ++p)
    delete p->second;
}


}} // namespaces

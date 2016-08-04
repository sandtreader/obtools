//==========================================================================
// ObTools::ReGen: master.cc
//
// Master file - reads master blocks and merges with user file
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-regen.h"
using namespace ObTools::ReGen;

//==========================================================================
//Master block

//--------------------------------------------------------------------------
// Add a line
void Block::add_line(LineType lt, const string& t)
{
  lines.push_back(new BlockLine(lt, t));
}

//--------------------------------------------------------------------------
// Destructor
Block::~Block()
{
  for(list<BlockLine *>::iterator p = lines.begin();
      p!=lines.end();
      ++p)
    delete *p;
}

//--------------------------------------------------------------------------
// Dump to output stream
void Block::dump(ostream& sout)
{
  const char *prefix = "|";
  for(list<BlockLine *>::iterator p = lines.begin();
      p!=lines.end();
      ++p)
  {
    BlockLine *bl = *p;

    if (bl->type == LINE_USER_END) prefix="|";
    sout << prefix << bl->text << endl;
    if (bl->type == LINE_USER_START) prefix="-";
  }
}

//==========================================================================
//Master file

//--------------------------------------------------------------------------
// Constructor
MasterFile::MasterFile(istream &in, const char *mark)
  :MarkedFile(in, mark)
{
  Block *current_block = 0;

  // Scan for blocks
  while (read_line())
  {
    LineType lt = line_type();

    //Open new blocks before adding, to include open
    if (lt == LINE_OPEN)
    {
      string tag = line_tag();
      if (tag.size())
      {
        current_block = new Block;
        blocks.push_back(current_block);
        blockmap[tag] = current_block;
      }
    }

    //Add lines if required
    if (current_block) current_block->add_line(lt, line_text());

    //Close old blocks after adding, to include close
    if (lt == LINE_CLOSE) current_block = 0;
  }
}

//--------------------------------------------------------------------------
// Dump blocks to given stream
void MasterFile::dump(ostream& sout)
{
  for(list<Block *>::iterator p = blocks.begin();
      p!=blocks.end();
      ++p)
  {
    sout << "###############\n";
    (*p)->dump(sout);
  }
}

//--------------------------------------------------------------------------
// Get block of given tag
Block *MasterFile::find_block(const string& tag)
{
  map<string,Block *>::iterator p=blockmap.find(tag);
  if (p!=blockmap.end())
    return p->second;
  else
    return 0;
}

//--------------------------------------------------------------------------
// Merge with given user file, to given stream
void MasterFile::merge(MarkedFile &ufile, ostream& sout, int flags)
{
  bool use_lines = true;
  Block *b = 0;                       // Current master block (if any)
  list<BlockLine *>::iterator bp;     // current line within master block

  //Read lines from user file and merge blocks from master
  while (ufile.read_line())
  {
    switch (ufile.line_type())
    {
      case LINE_NORMAL:
        // Just spool it if required
        if (use_lines) sout << ufile.line_text() << endl;
        break;

      case LINE_OPEN:
      {
        // Block starting - get and check tag
        string tag = ufile.line_tag();
        if (tag.size())
        {
          b = find_block(tag);
          if (b)
          {
            // Dump out all lines until a USTART or end
            for(bp = b->lines.begin(); bp!= b->lines.end(); ++bp)
            {
              BlockLine *bl = *bp;
              if (bl->type == LINE_USER_START) break;
              sout << bl->text << endl;
            }

            // Mark block as used
            b->used = true;

            // Suppress user lines
            use_lines = false;
          }
          else
          {
            // Suppress user lines if they want to delete orphans
            if (flags & MERGE_DELETE_ORPHANS)
              use_lines = false;
            else
            {
              // Copy out all user lines until CLOSE, verbatim
              do
              {
                sout << ufile.line_text() << endl;
              } while (ufile.read_line() && ufile.line_type() != LINE_CLOSE);

              // Include close as well
              sout << ufile.line_text() << endl;
            }
          }
        }
        break;
      }

      case LINE_CLOSE:
        // Ensure any remaining master lines are used - for example
        // if they have deleted a user cutout
        if (b)
        {
          for(;bp!= b->lines.end(); ++bp)
          {
            BlockLine *bl = *bp;
            sout << bl->text << endl;
          }
        }

        //Clean up
        b = 0;
        use_lines = true;
        break;

      case LINE_USER_START:
        // User cut-out starting - skip over lines in master until USER_END
        if (b)
        {
          for(;bp!= b->lines.end() && (*bp)->type != LINE_USER_END; ++bp)
            ;

          //Skip USER_END line, too - we'll use their version
          if (bp!=b->lines.end()) bp++;

          // Allow user lines and use their copy of this one
          use_lines = true;
          sout << ufile.line_text() << endl;
        }
        break;

      case LINE_USER_END:
        // User cut-out ends - continue with master until another USTART
        if (b)
        {
          // Use their version of this line
          sout << ufile.line_text() << endl;

          // Dump out all lines until a USTART or end
          for(;bp!= b->lines.end(); ++bp)
          {
            BlockLine *bl = *bp;
            sout << bl->text << endl;
            if (bl->type == LINE_USER_START)
              break;
          }

          // suppress user lines again
          use_lines = false;
        }
        break;
    }
  }

  // Append any remaining unused blocks from the master
  if (!(flags & MERGE_SUPPRESS_NEW))
  {
    for(list<Block *>::iterator p = blocks.begin();
        p!=blocks.end();
        ++p)
    {
      b = *p;
      if (!b->used)
      {
        sout << endl;
        for(bp = b->lines.begin(); bp!= b->lines.end(); ++bp)
        {
          BlockLine *bl = *bp;
          sout << bl->text << endl;
        }
      }

      // Reset for next time
      b->used = false;
    }
  }
}

//--------------------------------------------------------------------------
// Destructor
MasterFile::~MasterFile()
{
  for(list<Block *>::iterator p = blocks.begin();
      p!=blocks.end();
      ++p)
    delete *p;
}


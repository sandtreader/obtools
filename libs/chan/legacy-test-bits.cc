//==========================================================================
// ObTools::Net: test-bits.cc
//
// Test harness for bitstream reader/writers
//
// Copyright (c) 2007 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-chan.h"
using namespace std;

//--------------------------------------------------------------------------
// Main

int main()
{
  string buf;
  ObTools::Channel::StringWriter bw(buf);
  ObTools::Channel::BitWriter bitw(bw);

  bitw.write_bit(1);
  bitw.write_bit(0);
  bitw.write_bool(true);
  bitw.write_bool(false);
  bitw.write_bits(8, 0x1F);
  bitw.write_bit(0);
  bitw.write_bit(0);
  bitw.write_bit(0);
  bitw.write_bit(1);
  bitw.write_bit(0);
  bitw.write_bit(1);
  bitw.write_bit(0);
  bitw.flush();


  ObTools::Channel::StringReader br(buf);
  cout << "Bytes written: " << bw.get_offset() << ": "
       << hex << br.read_nbo_16() << endl;
  br.rewind();

  ObTools::Channel::BitEGReader bitr(br);

  cout << "One: "  << bitr.read_bit() << endl;
  cout << "Zero: " << bitr.read_bit() << endl;
  cout << "True: "  << (bitr.read_bool()?"true":"false") << endl;
  cout << "False: "  << (bitr.read_bool()?"true":"false") << endl;
  cout << "8 bits, 0x1F: " << bitr.read_bits(8) << endl;
  cout << "Exp-Golomb 9: " << bitr.read_exp_golomb() << endl;
  cout << "Padding bits: " << bitr.read_bits(5) << endl;

  try
  {
    bitr.read_bit();
  }
  catch (const ObTools::Channel::Error& e)
  {
    cout << "Buffer ended OK: " << e << endl;
    return 0;
  }

  cerr << "Buffer didn't end correctly\n";
  return 2;
}





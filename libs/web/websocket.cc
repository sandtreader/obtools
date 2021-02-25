//==========================================================================
// ObTools::Web: websocket.cc
//
// HTTP WebSocket frame implementation
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-web.h"
#include "ot-text.h"
#include "ot-chan.h"

namespace ObTools { namespace Web {


//==========================================================================
// WebSocket frame, binary structure

//------------------------------------------------------------------------
// Read a WebSocket frame
// Returns whether successfully read
bool WebSocketFrame::read(Net::TCPStream& stream)
{
  Channel::StreamReader reader(stream);
  try
  {
    unsigned char fin_op = reader.read_byte();
    fin = (fin_op & 0x80) != 0;
    opcode = static_cast<Opcode>(fin_op & 0x0F);

    unsigned char mask_len = reader.read_byte();
    bool masked = (mask_len & 0x80) != 0;
    uint64_t len = mask_len & 0x7f;
    if (len == 126)
      len = reader.read_nbo_16();
    else if (len == 127)
      len = reader.read_nbo_64();

    unsigned char mask_key[4];
    if (masked) reader.read(mask_key, 4);

    // Read payload
    reader.read(payload, len);

    // Unmask if set
    if (masked)
    {
      for(uint64_t i=0; i<len; i++)
        payload[i] ^= mask_key[i%4];
    }

    return true;
  }
  catch (Channel::Error e)
  {
    Log::Error log;
    log << "WebSocket read failed: " << e.text << " (" << e.error << ")\n";
    return false;
  }
}

//------------------------------------------------------------------------
// Write a WebSocket frame
// Returns whether successfully written
bool WebSocketFrame::write(Net::TCPStream& stream) const
{
  Channel::StreamWriter writer(stream);

  try
  {
    unsigned char fin_op = static_cast<int>(opcode);
    if (fin) fin_op |= 0x80;
    writer.write_byte(fin_op);

    auto len = payload.size();
    if (len < 126)
      writer.write_byte(len);
    else if (len <= 0xFFFF)
    {
      writer.write_byte(126);
      writer.write_nbo_16(len);
    }
    else
    {
      writer.write_byte(127);
      writer.write_nbo_64(len);
    }

    // Note server-generated frames are never masked
    writer.write(payload);

    return true;
  }
  catch (Channel::Error e)
  {
    Log::Error log;
    log << "WebSocket write failed: " << e.text << " (" << e.error << ")\n";
    return false;
  }
}

//------------------------------------------------------------------------
// Dump a WebSocket frame to the given channel, optionally dumping payload
// too
void WebSocketFrame::dump(ostream& sout, bool dump_payload) const
{
  sout << "WebSocket Frame " << (fin?"FIN ":"")
       << static_cast<int>(opcode) << " (";
  switch (opcode)
  {
    case Opcode::continuation: sout << "continuation"; break;
    case Opcode::text:         sout << "text"; break;
    case Opcode::binary:       sout << "binary"; break;
    case Opcode::close:        sout << "close"; break;
    case Opcode::ping:         sout << "ping"; break;
    case Opcode::pong:         sout << "pong"; break;
    default:                   sout << "UNKNOWN";
  }

  sout << ") len " << payload.size() << endl;

  if (dump_payload)
  {
    if (opcode == Opcode::text)
      sout << "  [" << payload << "]\n";
    else
    {
      Misc::Dumper dumper(sout);
      dumper.dump(payload);
    }
  }
}

//==========================================================================
// WebSocket server protocol helper

//------------------------------------------------------------------------
// Read a message - blocks waiting for a message (which may be multiple
// fragmented frames).  Returns whether a valid message received
bool WebSocketServer::read(string& msg)
{
  msg.clear();
  for(;;) // loop after ping, fragmentation
  {
    WebSocketFrame frame;
    if (!frame.read(stream)) return false;

    OBTOOLS_LOG_IF_DEBUG(Log::Debug log; log << "WS received:\n";
                         frame.dump(log, true);)

    switch (frame.opcode)
    {
      case WebSocketFrame::Opcode::continuation:
      case WebSocketFrame::Opcode::text:
      case WebSocketFrame::Opcode::binary:
      {
        // Add to existing
        msg += frame.payload;

        // If last frame, return it
        if (frame.fin) return true;
      }
      break;

      case WebSocketFrame::Opcode::close:
      {
        Log::Detail log;
        log << "WebSocket close received\n";

        // Send the same back
        write(frame);
        return false;
      }

      case WebSocketFrame::Opcode::ping:
      {
        // Send back a pong with same payload
        WebSocketFrame pong(WebSocketFrame::Opcode::pong);
        pong.payload = frame.payload;
        write(pong);
      }
      break;

      case WebSocketFrame::Opcode::pong:
        // Just ignore
      break;

      default:
      {
        Log::Error log;
        log << "Unexpected WebSocket frame: ";
        frame.dump(log);
      }
    }
  }
}

//------------------------------------------------------------------------
// Write a frame, with mutex on stream, and flush it
// Returns whether successfully written
bool WebSocketServer::write(const WebSocketFrame& frame)
{
  MT::Lock lock(stream_mutex);
  if (!frame.write(stream)) return false;
  stream.flush();
  return true;
}

//------------------------------------------------------------------------
// Write a textual message.  Returns whether able to write
bool WebSocketServer::write(const string& msg)
{
  WebSocketFrame frame(WebSocketFrame::Opcode::text);
  frame.payload = msg;

  OBTOOLS_LOG_IF_DEBUG(Log::Debug log; log << "WS sending:\n";
                       frame.dump(log, true);)

  return write(frame);
}

//------------------------------------------------------------------------
// Write a binary message.  Returns whether able to write
bool WebSocketServer::write_binary(string& msg)
{
  WebSocketFrame frame(WebSocketFrame::Opcode::binary);
  frame.payload = msg;

  OBTOOLS_LOG_IF_DEBUG(Log::Debug log; log << "WS sending:\n";
                       frame.dump(log, true);)

  return write(frame);
}

//------------------------------------------------------------------------
// Send a close
void WebSocketServer::close()
{
  WebSocketFrame frame(WebSocketFrame::Opcode::close);
  OBTOOLS_LOG_IF_DEBUG(Log::Debug log; log << "WS closing\n";)
  write(frame);
}

}} // namespaces




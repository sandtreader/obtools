//==========================================================================
// ObTools::Web: gzip.cc
//
// Gzip compression using zlib
//
// Copyright (c) 2026 Paul Clark.
//==========================================================================

#include "ot-web.h"
#if !defined(PLATFORM_WEB)
#include <zlib.h>
#endif

namespace ObTools { namespace Web {

//--------------------------------------------------------------------------
// Compress 'in' into 'out' as a gzip stream, with zlib level 1-9
// Returns whether successful
bool GZip::compress(const string& in, string& out, int level)
{
#if defined(PLATFORM_WEB)
  (void)in; (void)out; (void)level;
  return false;
#else
  z_stream zs{};
  // windowBits 15 + 16 gives a gzip wrapper
  if (deflateInit2(&zs, level, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY)
      != Z_OK)
    return false;

  zs.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(in.data()));
  zs.avail_in = in.size();

  // Output all in one go into a buffer of the maximum size
  out.resize(deflateBound(&zs, in.size()));
  zs.next_out = reinterpret_cast<Bytef *>(&out[0]);
  zs.avail_out = out.size();

  const auto ret = deflate(&zs, Z_FINISH);
  deflateEnd(&zs);
  if (ret != Z_STREAM_END)
  {
    out.clear();
    return false;
  }

  out.resize(zs.total_out);
  return true;
#endif
}

}} // namespaces

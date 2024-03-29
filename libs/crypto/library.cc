//==========================================================================
// ObTools::Crypto: library.cc
//
// Overall library initialisation/shutdown
//
// Copyright (c) 2006 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-crypto.h"
#include "ot-mt.h"
#define SSL OpenSSL
#include "openssl/evp.h"
#include "openssl/engine.h"
#undef SSL

// Temporary bodge to ignore deprecations in OpenSSL 3.0.
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

// Definition of type for dynamic locks, outside namespaces
struct CRYPTO_dynlock_value
{
  ObTools::MT::Mutex mutex;
};

namespace ObTools { namespace Crypto {

//--------------------------------------------------------------------------
// OpenSSL locking callbacks - required for multithreaded code
// See: threads(3)

// Array of static mutexes
static vector<MT::Mutex *> mutexes;

#if OPENSSL_VERSION_NUMBER < 0x10100000L
// Force C call standard
extern "C"
{

// Locking function
static void openssl_locking_function(int mode, int n, const char *, int)
{
  if (mode & CRYPTO_LOCK)
    mutexes[n]->lock();
  else
    mutexes[n]->unlock();
}

// Thread id function.
static unsigned long openssl_id_function(void)
{
#if defined(PLATFORM_WINDOWS)
  // Use pointer value from ptw32_handle_t - this is highly sensitive to
  // library organisation and pointer sizes, but it's all that's available
  return (unsigned long)(pthread_self().p);
#else
  // Not sure this is always safe to cast, but doing it otherwise would
  // be horrendous (TLS unique integer, left as exercise to the reader!)
  return (unsigned long)pthread_self();
#endif
}

// Dynamic lock creation function
static struct CRYPTO_dynlock_value *openssl_dyn_create_function(const char *,
                                                                int)
{
  return new struct CRYPTO_dynlock_value();
}

// Dynamic locking function.
static void openssl_dyn_lock_function(int mode, struct CRYPTO_dynlock_value *l,
                                      const char *, int)
{
  if (mode & CRYPTO_LOCK)
    l->mutex.lock();
  else
    l->mutex.unlock();
}

// Destroy dynamic crypto lock function
static void openssl_dyn_destroy_function(struct CRYPTO_dynlock_value *l,
                                         const char *, int)
{
  delete l;
}

} // extern "C"
#endif

//--------------------------------------------------------------------------
// Constructor
Library::Library()
{
  if (should_initialise())
  {
    // Create as many locks as the library requires
    for(int n=CRYPTO_num_locks(); n--;)
      mutexes.push_back(new MT::Mutex());

#if OPENSSL_VERSION_NUMBER < 0x10100000L
    // Register callbacks
    CRYPTO_set_locking_callback(openssl_locking_function);
    CRYPTO_set_id_callback(openssl_id_function);
    CRYPTO_set_dynlock_create_callback(openssl_dyn_create_function);
    CRYPTO_set_dynlock_lock_callback(openssl_dyn_lock_function);
    CRYPTO_set_dynlock_destroy_callback(openssl_dyn_destroy_function);
#endif

    OpenSSL_add_all_algorithms();

    // Initiliase engine with default digests
    ENGINE_load_builtin_engines();
    ENGINE_register_all_complete();
  }
}

//--------------------------------------------------------------------------
// Destructor
Library::~Library()
{
  // Clean up unless no-one has initialised before
  if (!should_initialise())
  {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    // Clear all callbacks
    CRYPTO_set_dynlock_create_callback(NULL);
    CRYPTO_set_dynlock_lock_callback(NULL);
    CRYPTO_set_dynlock_destroy_callback(NULL);
    CRYPTO_set_locking_callback(NULL);
    CRYPTO_set_id_callback(NULL);
#endif

    // Empty the mutex array
    for(vector<MT::Mutex *>::iterator p=mutexes.begin(); p!=mutexes.end();++p)
      delete *p;
    mutexes.clear();

    // Tidy up library
    EVP_cleanup();
  }
}

}} // namespaces


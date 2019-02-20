#include "core.h"
#include "crypto_generichash.h"
#include "crypto_onetimeauth.h"
#include "crypto_scalarmult.h"
#include "crypto_stream_chacha20.h"
#include "crypto_stream_salsa20.h"
#include "randombytes.h"
#include "runtime.h"
#include "utils.h"
#include "private/implementations.h"
#include "private/mutex.h"

#include <base/lock.h>
#include <base/log.h>

static int initialized;

static Genode::Lock *_sodium_lock;

extern "C"
int sodium_init(void)
{
    _sodium_runtime_get_cpu_features();
    randombytes_stir();
    _sodium_alloc_init();
    //_crypto_onetimeauth_poly1305_pick_best_implementation();
    //_crypto_scalarmult_curve25519_pick_best_implementation();
    //_crypto_stream_chacha20_pick_best_implementation();
    //_crypto_stream_salsa20_pick_best_implementation();
    initialized = 1;
    return 0;
}


int
sodium_crit_enter(void)
{
    _sodium_lock->lock();
    return 0;
}

int
sodium_crit_leave(void)
{
    _sodium_lock->unlock();
    return 0;
}


extern "C"
int sodium_set_misuse_handler(void (*handler)(void))
{
	(void)handler;
	Genode::error(__func__, " not implemented");
	return 0;
}

extern "C"
void _sodium_misuse(char const *file, int line)
{
	Genode::error("libsodium breakage at ", file, ":", line);
}

/*
 * \brief  Salsa20 RNG seeded with Jitterentropy
 * \author Emery Hemingway
 * \date   2015-09-13
 */

#include "crypto_core_salsa20.h"
#include "crypto_auth_hmacsha512256.h"
#include "crypto_stream_salsa20.h"
#include "randombytes.h"
#include "randombytes_salsa20_random.h"
#include "utils.h"

#define SALSA20_RANDOM_BLOCK_SIZE crypto_core_salsa20_OUTPUTBYTES
#define SHA512_BLOCK_SIZE 128U
#define SHA512_MIN_PAD_SIZE (1U + 16U)
#define COMPILER_ASSERT(X) (void) sizeof(char[(X) ? 1 : -1])

typedef struct Salsa20Random_ {
    unsigned char     key[crypto_stream_salsa20_KEYBYTES];
    unsigned char     rnd32[16U * SALSA20_RANDOM_BLOCK_SIZE];
    uint64_t          nonce;
    size_t            rnd32_outleft;
    struct rand_data *jent_collector;
    int               initialized;
    int               getrandom_available;
} Salsa20Random;

static Salsa20Random stream = {
    .rnd32_outleft = (size_t) 0U,
    .initialized = 0,
    .getrandom_available = 0
};

static void
randombytes_salsa20_random_init(void)
{
    /* XXX: check error */
    jent_entropy_init();

    stream.jent_collector = jent_entropy_collector_alloc(0, 0);
    jent_read_entropy(stream.jent_collector, (char*)&stream.nonce, sizeof(stream.nonce));
    /* assert(stream.nonce != (uint64_t) 0U); */
}

static void
randombytes_salsa20_random_rekey(const unsigned char * const mix)
{
    unsigned char *key = stream.key;
    size_t         i;

    for (i = (size_t) 0U; i < sizeof stream.key; i++) {
        key[i] ^= mix[i];
    }
}

void
randombytes_salsa20_random_stir(void)
{
    const unsigned char s[crypto_auth_hmacsha512256_KEYBYTES] = {
        'T', 'h', 'i', 's', 'I', 's', 'J', 'u', 's', 't', 'A', 'T',
        'h', 'i', 'r', 't', 'y', 'T', 'w', 'o', 'B', 'y', 't', 'e',
        's', 'S', 'e', 'e', 'd', '.', '.', '.'
    };
    unsigned char  m0[crypto_auth_hmacsha512256_BYTES +
                      2U * SHA512_BLOCK_SIZE - SHA512_MIN_PAD_SIZE];
    unsigned char *k0 = m0 + crypto_auth_hmacsha512256_BYTES;
    size_t         sizeof_k0 = sizeof m0 - crypto_auth_hmacsha512256_BYTES;

    memset(stream.rnd32, 0, sizeof stream.rnd32);
    stream.rnd32_outleft = (size_t) 0U;
    if (stream.initialized == 0) {
        randombytes_salsa20_random_init();
        stream.initialized = 1;
    }

    jent_read_entropy(stream.jent_collector, (char*)m0, sizeof m0);

    COMPILER_ASSERT(sizeof stream.key == crypto_auth_hmacsha512256_BYTES);
    crypto_auth_hmacsha512256(stream.key, k0, sizeof_k0, s);
    COMPILER_ASSERT(sizeof stream.key <= sizeof m0);
    randombytes_salsa20_random_rekey(m0);
    sodium_memzero(m0, sizeof m0);
}

static void
randombytes_salsa20_random_stir_if_needed(void)
{
    if (stream.initialized == 0) {
        randombytes_salsa20_random_stir();
    }
}

static uint32_t
randombytes_salsa20_random_getword(void)
{
    uint32_t val;

    COMPILER_ASSERT(sizeof stream.rnd32 >= (sizeof stream.key) + (sizeof val));
    COMPILER_ASSERT(((sizeof stream.rnd32) - (sizeof stream.key))
                    % sizeof val == (size_t) 0U);
    if (stream.rnd32_outleft <= (size_t) 0U) {
        randombytes_salsa20_random_stir_if_needed();
        COMPILER_ASSERT(sizeof stream.nonce == crypto_stream_salsa20_NONCEBYTES);
        crypto_stream_salsa20((unsigned char *) stream.rnd32,
                              (unsigned long long) sizeof stream.rnd32,
                              (unsigned char *) &stream.nonce,
                              stream.key);
        //assert(ret == 0);
        stream.rnd32_outleft = (sizeof stream.rnd32) - (sizeof stream.key);
        randombytes_salsa20_random_rekey(&stream.rnd32[stream.rnd32_outleft]);
        stream.nonce++;
    }
    stream.rnd32_outleft -= sizeof val;
    memcpy(&val, &stream.rnd32[stream.rnd32_outleft], sizeof val);
    memset(&stream.rnd32[stream.rnd32_outleft], 0, sizeof val);

    return val;
}

int
randombytes_salsa20_random_close(void)
{
    stream.initialized = 0;
    return 0;
}

uint32_t
randombytes_salsa20_random(void)
{
    return randombytes_salsa20_random_getword();
}

void
randombytes_salsa20_random_buf(void * const buf, const size_t size)
{
    size_t i;

    randombytes_salsa20_random_stir_if_needed();
    COMPILER_ASSERT(sizeof stream.nonce == crypto_stream_salsa20_NONCEBYTES);
#ifdef ULONG_LONG_MAX
    /* coverity[result_independent_of_operands] */
    /* assert(size <= ULONG_LONG_MAX); */
#endif
    crypto_stream_salsa20((unsigned char *) buf, (unsigned long long) size,
                          (unsigned char *) &stream.nonce, stream.key);
    /* assert(ret == 0); */
    for (i = 0U; i < sizeof size; i++) {
        stream.key[i] ^= ((const unsigned char *) (const void *) &size)[i];
    }
    stream.nonce++;
    crypto_stream_salsa20_xor(stream.key, stream.key, sizeof stream.key,
                              (unsigned char *) &stream.nonce, stream.key);
}

const char *
randombytes_salsa20_implementation_name(void)
{
    return "salsa20";
}

struct randombytes_implementation randombytes_sysrandom_implementation = {
    SODIUM_C99(.implementation_name =) randombytes_salsa20_implementation_name,
    SODIUM_C99(.random =) randombytes_salsa20_random,
    SODIUM_C99(.stir =) randombytes_salsa20_random_stir,
    SODIUM_C99(.uniform =) NULL,
    SODIUM_C99(.buf =) randombytes_salsa20_random_buf,
    SODIUM_C99(.close =) randombytes_salsa20_random_close
};

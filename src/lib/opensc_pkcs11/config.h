/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* CVC directory */
#define CVCDIR ""

/* Default PC/SC provider */
#define DEFAULT_PCSC_PROVIDER "pcsc-lite.lib.so"

/* Define if CryptoTokenKit is to be enabled */
/* #undef ENABLE_CRYPTOTOKENKIT */

/* Enable CT-API support */
/* #undef ENABLE_CTAPI */

/* Enable the use of external user interface program to request DNIe user pin
   */
/* #undef ENABLE_DNIE_UI */

/* Use glib2 libraries and header files */
/* #undef ENABLE_GIO2 */

/* Enable minidriver support */
/* #undef ENABLE_MINIDRIVER */

/* Use notification libraries and header files */
/* #undef ENABLE_NOTIFY */

/* Have OpenCT libraries and header files */
/* #undef ENABLE_OPENCT */

/* Use OpenPACE libraries and header files */
/* #undef ENABLE_OPENPACE */

/* Have OpenSSL libraries and header files */
/* #undef ENABLE_OPENSSL */

/* Define if PC/SC is to be enabled */
#define ENABLE_PCSC 1

/* Use readline libraries and header files */
/* #undef ENABLE_READLINE */

/* Enable shared libraries */
/* #undef ENABLE_SHARED */

/* Enable secure messaging support */
#define ENABLE_SM 1

/* Use zlib libraries and header files */
/* #undef ENABLE_ZLIB */

/* Define to 1 if you have the declaration of `strlcat', and to 0 if you
   don't. */
#define HAVE_DECL_STRLCAT 1

/* Define to 1 if you have the declaration of `strlcpy', and to 0 if you
   don't. */
#define HAVE_DECL_STRLCPY 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you don't have `vprintf' but do have `_doprnt.' */
/* #undef HAVE_DOPRNT */

/* Define to 1 if you have the <eac/eac.h> header file. */
/* #undef HAVE_EAC_EAC_H */

/* Define to 1 if you have the <endian.h> header file. */
/* #undef HAVE_ENDIAN_H */

/* Define to 1 if you have the <errno.h> header file. */
#define HAVE_ERRNO_H 1

/* Define to 1 if you have the `explicit_bzero' function. */
/* #undef HAVE_EXPLICIT_BZERO */

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define to 1 if you have the `getline' function. */
#define HAVE_GETLINE 1

/* Define to 1 if you have the <getopt.h> header file. */
#define HAVE_GETOPT_H 1

/* Define to 1 if you have the `getopt_long' function. */
#define HAVE_GETOPT_LONG 1

/* Define to 1 if you have the `getpass' function. */
#define HAVE_GETPASS 1

/* Define to 1 if you have the `gettimeofday' function. */
#define HAVE_GETTIMEOFDAY 1

/* Define to 1 if you have the <gio/gio.h> header file. */
/* #undef HAVE_GIO_GIO_H */

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `memset' function. */
#define HAVE_MEMSET 1

/* Define to 1 if you have the `memset_s' function. */
/* #undef HAVE_MEMSET_S */

/* Define to 1 if you have the `mkdir' function. */
#define HAVE_MKDIR 1

/* Define to 1 if you have the <openssl/crypto.h> header file. */
/* #undef HAVE_OPENSSL_CRYPTO_H */

/* Define to 1 if you have the <pcsclite.h> header file. */
#define HAVE_PCSCLITE_H 1

/* Define if you have POSIX threads libraries and header files. */
#define HAVE_PTHREAD 1

/* Have PTHREAD_PRIO_INHERIT. */
#define HAVE_PTHREAD_PRIO_INHERIT 1

/* Define to 1 if you have the <readline/readline.h> header file. */
/* #undef HAVE_READLINE_READLINE_H */

/* Define to 1 if you have the `sigaction' function. */
#define HAVE_SIGACTION 1

/* Define to 1 if `stat' has the bug that it succeeds when given the
   zero-length file name argument. */
#define HAVE_STAT_EMPTY_STRING_BUG 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strdup' function. */
#define HAVE_STRDUP 1

/* Define to 1 if you have the `strerror' function. */
#define HAVE_STRERROR 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strnlen' function. */
#define HAVE_STRNLEN 1

/* Define to 1 if you have the <sys/endian.h> header file. */
#define HAVE_SYS_ENDIAN_H 1

/* Define to 1 if you have the <sys/mman.h> header file. */
#define HAVE_SYS_MMAN_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have <sys/wait.h> that is POSIX.1 compatible. */
#define HAVE_SYS_WAIT_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the `vprintf' function. */
#define HAVE_VPRINTF 1

/* Define to 1 if you have the <wcautil.h> header file. */
/* #undef HAVE_WCAUTIL_H */

/* Define to 1 if you have the <winscard.h> header file. */
#define HAVE_WINSCARD_H 1

/* Define to 1 if you have the <zlib.h> header file. */
/* #undef HAVE_ZLIB_H */

/* Define to 1 if `lstat' dereferences a symlink specified with a trailing
   slash. */
/* #undef LSTAT_FOLLOWS_SLASHED_SYMLINK */

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#define LT_OBJDIR ".libs/"

/* Define to 1 if assertions should be disabled. */
/* #undef NDEBUG */

/* Enabled OpenSC features */
#define OPENSC_FEATURES " locking pcsc(pcsc-lite.lib.so)"

/* OpenSC version Git describe revision */
#define OPENSC_SCM_REVISION "OpenSC-<version not available>, rev: b4a8d6d, commit-time: 2022-06-30 12:11:14 +0200"

/* OpenSC version fix component */
#define OPENSC_VERSION_FIX 0

/* OpenSC version major component */
#define OPENSC_VERSION_MAJOR 0

/* OpenSC version minor component */
#define OPENSC_VERSION_MINOR 22

/* OpenSC file version revision */
#define OPENSC_VERSION_REVISION 0

/* OpenSC version-info Comments */
#define OPENSC_VS_FF_COMMENTS "Provided under the terms of the GNU Lesser General Public License (LGPLv2.1+)."

/* OpenSC version-info CompanyName value */
#define OPENSC_VS_FF_COMPANY_NAME "OpenSC Project"

/* OpenSC version-info UpdateURL */
#define OPENSC_VS_FF_COMPANY_URL "https://github.com/OpenSC"

/* OpenSC version-info LegalCopyright value */
#define OPENSC_VS_FF_LEGAL_COPYRIGHT "OpenSC Project"

/* OpenSC version-info ProductName */
#define OPENSC_VS_FF_PRODUCT_NAME "OpenSC smartcard framework"

/* OpenSC version-info UpdateURL */
#define OPENSC_VS_FF_PRODUCT_UPDATES "https://github.com/OpenSC/OpenSC/releases"

/* OpenSC version-info ProductURL */
#define OPENSC_VS_FF_PRODUCT_URL "https://github.com/OpenSC/OpenSC"

/* Size of OpenSSL secure memory in bytes, must be a power of 2 */
/* #undef OPENSSL_SECURE_MALLOC_SIZE */

/* Name of package */
#define PACKAGE "opensc"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "https://github.com/OpenSC/OpenSC/issues"

/* Define to the full name of this package. */
#define PACKAGE_NAME "OpenSC"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "OpenSC 0.22.0-rc1"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "opensc"

/* Define to the home page for this package. */
#define PACKAGE_URL "https://github.com/OpenSC/OpenSC"

/* Define to the version of this package. */
#define PACKAGE_VERSION "0.22.0-rc1"

/* Sufficient version of PCSC-Lite with all the required features */
#define PCSCLITE_GOOD 1

/* Define to necessary symbol if this constant uses a non-standard name on
   your system. */
/* #undef PTHREAD_CREATE_JOINABLE */

/* The size of `void *', as computed by sizeof. */
#define SIZEOF_VOID_P 8

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define if you are on Cygwin */
/* #undef USE_CYGWIN */

/* Version number of package */
#define VERSION "0.22.0-rc1"

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif

/* CVC directory */
#define X509DIR ""

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `int' if <sys/types.h> doesn't define. */
/* #undef gid_t */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

/* Define to `int' if <sys/types.h> doesn't define. */
/* #undef uid_t */

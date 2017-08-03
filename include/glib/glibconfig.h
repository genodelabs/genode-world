#ifndef __G_LIBCONFIG_H__
#define __G_LIBCONFIG_H__

/* Genode includes */
#include <base/fixed_stdint.h>

/* Libc includes */
//#include <float.h>
#include <sys/limits.h>

/* Glib includes */
#include <glib/gmacros.h>

G_BEGIN_DECLS

#define G_MINFLOAT	FLT_MIN
#define G_MAXFLOAT	FLT_MAX
#define G_MINDOUBLE	DBL_MIN
#define G_MAXDOUBLE	DBL_MAX
#define G_MINSHORT	SHRT_MIN
#define G_MAXSHORT	SHRT_MAX
#define G_MAXUSHORT	USHRT_MAX
#define G_MININT	INT_MIN
#define G_MAXINT	INT_MAX
#define G_MAXUINT	UINT_MAX
#define G_MINLONG	LONG_MIN
#define G_MAXLONG	LONG_MAX
#define G_MAXULONG	ULONG_MAX

typedef genode_int8_t   gint8;
typedef genode_uint8_t  guint8;
typedef genode_int16_t  gint16;
typedef genode_uint16_t guint16;
typedef genode_int32_t  gint32;
typedef genode_uint32_t guint32;
typedef genode_int64_t  gint64;
typedef genode_uint64_t guint64;

#define G_MAXSIZE	G_MAXUINT64
#define G_MINSSIZE	G_MININT64
#define G_MAXSSIZE	G_MAXINT64

typedef unsigned long gsize;
typedef          long gssize;
typedef          long goffset;
#define G_MINOFFSET	G_MININT64
#define G_MAXOFFSET	G_MAXINT64

#define G_GOFFSET_MODIFIER      G_GINT64_MODIFIER
#define G_GOFFSET_FORMAT        G_GINT64_FORMAT
#define G_GOFFSET_CONSTANT(val) G_GINT64_CONSTANT(val)

#define GLIB_SIZEOF_VOID_P 8
#define GLIB_SIZEOF_LONG   8
#define GLIB_SIZEOF_SIZE_T 8

#define GPOINTER_TO_INT(p)	((gint64)   (p))
#define GPOINTER_TO_UINT(p)	((guint64)  (p))

#define GINT_TO_POINTER(i)	((gpointer)  (i))
#define GUINT_TO_POINTER(u)	((gpointer)  (u))

#define G_GSIZE_MODIFIER ""
#define G_GSSIZE_MODIFIER ""
#define G_GSIZE_FORMAT "u"
#define G_GSSIZE_FORMAT "i"
#define G_GINT16_FORMAT "i"
#define G_GUINT16_FORMAT "u"
#define G_GINT32_FORMAT "i"
#define G_GUINT32_FORMAT "u"
#define G_GINT64_FORMAT "i"
#define G_GUINT64_FORMAT "u"
#define G_BYTE_ORDER G_LITTLE_ENDIAN

#define GLIB_SYSDEF_POLLIN =1
#define GLIB_SYSDEF_POLLOUT =4
#define GLIB_SYSDEF_POLLPRI =2
#define GLIB_SYSDEF_POLLHUP =16
#define GLIB_SYSDEF_POLLERR =8
#define GLIB_SYSDEF_POLLNVAL =32

typedef char GPid;

#define G_GINT64_CONSTANT(val)	(G_GNUC_EXTENSION (val##LL))
#define G_GUINT64_CONSTANT(val)	(G_GNUC_EXTENSION (val##ULL))

#define G_VA_COPY	va_copy

#define GINT16_TO_LE(val)	((gint16) (val))
#define GUINT16_TO_LE(val)	((guint16) (val))
#define GINT16_TO_BE(val)	((gint16) GUINT16_SWAP_LE_BE (val))
#define GUINT16_TO_BE(val)	(GUINT16_SWAP_LE_BE (val))
#define GINT32_TO_LE(val)	((gint32) (val))
#define GUINT32_TO_LE(val)	((guint32) (val))
#define GINT32_TO_BE(val)	((gint32) GUINT32_SWAP_LE_BE (val))
#define GUINT32_TO_BE(val)	(GUINT32_SWAP_LE_BE (val))
#define GINT64_TO_LE(val)	((gint64) (val))
#define GUINT64_TO_LE(val)	((guint64) (val))
#define GINT64_TO_BE(val)	((gint64) GUINT64_SWAP_LE_BE (val))
#define GUINT64_TO_BE(val)	(GUINT64_SWAP_LE_BE (val))
#define GLONG_TO_LE(val)	((glong) GINT32_TO_LE (val))
#define GULONG_TO_LE(val)	((gulong) GUINT32_TO_LE (val))
#define GLONG_TO_BE(val)	((glong) GINT32_TO_BE (val))
#define GULONG_TO_BE(val)	((gulong) GUINT32_TO_BE (val))
#define GINT_TO_LE(val)		((gint) GINT32_TO_LE (val))
#define GUINT_TO_LE(val)	((guint) GUINT32_TO_LE (val))
#define GINT_TO_BE(val)		((gint) GINT32_TO_BE (val))
#define GUINT_TO_BE(val)	((guint) GUINT32_TO_BE (val))
#define GSIZE_TO_LE(val)	((gsize) GUINT32_TO_LE (val))
#define GSSIZE_TO_LE(val)	((gssize) GINT32_TO_LE (val))
#define GSIZE_TO_BE(val)	((gsize) GUINT32_TO_BE (val))
#define GSSIZE_TO_BE(val)	((gssize) GINT32_TO_BE (val))
#define G_BYTE_ORDER G_LITTLE_ENDIAN

#define G_GINT64_MODIFIER "I64"

G_END_DECLS

#endif /* GLIBCONFIG_H */

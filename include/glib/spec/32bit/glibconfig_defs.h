#ifndef __G_LIBCONFIG_DEFS_H__
#define __G_LIBCONFIG_DEFS_H__

#define GLIB_SIZEOF_VOID_P 4
#define GLIB_SIZEOF_LONG   4
#define GLIB_SIZEOF_SIZE_T 4
#define GLIB_SIZEOF_SSIZE_T 4

#define G_MAXSIZE	G_MAXUINT
#define G_MINSSIZE	G_MININT
#define G_MAXSSIZE	G_MAXINT

#define G_GSIZE_MODIFIER ""
#define G_GSSIZE_MODIFIER ""
#define G_GSIZE_FORMAT "u"
#define G_GSSIZE_FORMAT "i"

#define G_GINT64_CONSTANT(val)	(G_GNUC_EXTENSION (val##LL))
#define G_GUINT64_CONSTANT(val)	(G_GNUC_EXTENSION (val##ULL))

#define G_GINT64_MODIFIER "ll"
#define G_GINT64_FORMAT "lli"
#define G_GUINT64_FORMAT "llu"

#define GLONG_TO_LE(val)	((glong) GINT32_TO_LE (val))
#define GULONG_TO_LE(val)	((gulong) GUINT32_TO_LE (val))
#define GLONG_TO_BE(val)	((glong) GINT32_TO_BE (val))
#define GULONG_TO_BE(val)	((gulong) GUINT32_TO_BE (val))

#define GSIZE_TO_LE(val)	((gsize) GUINT32_TO_LE (val))
#define GSSIZE_TO_LE(val)	((gssize) GINT32_TO_LE (val))
#define GSIZE_TO_BE(val)	((gsize) GUINT32_TO_BE (val))
#define GSSIZE_TO_BE(val)	((gssize) GINT32_TO_BE (val))

#define G_DIR_SEPARATOR '/'
#define G_DIR_SEPARATOR_S "/"
#define G_SEARCHPATH_SEPARATOR ':'
#define G_SEARCHPATH_SEPARATOR_S ":"

#endif /* __G_LIBCONFIG_DEFS_H__ */

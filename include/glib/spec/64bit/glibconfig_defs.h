#ifndef __G_LIBCONFIG_DEFS_H__
#define __G_LIBCONFIG_DEFS_H__

#define GLIB_SIZEOF_VOID_P 8
#define GLIB_SIZEOF_LONG   8
#define GLIB_SIZEOF_SIZE_T 8
#define GLIB_SIZEOF_SSIZE_T 8

#define G_MAXSIZE	G_MAXUINT64
#define G_MINSSIZE	G_MININT64
#define G_MAXSSIZE	G_MAXINT64

#define G_GSIZE_MODIFIER "l"
#define G_GSSIZE_MODIFIER "l"
#define G_GSIZE_FORMAT "lu"
#define G_GSSIZE_FORMAT "li"

#define G_GINT64_CONSTANT(val)	(G_GNUC_EXTENSION (val##L))
#define G_GUINT64_CONSTANT(val)	(G_GNUC_EXTENSION (val##UL))

#define G_GINT64_MODIFIER "l"
#define G_GINT64_FORMAT "li"
#define G_GUINT64_FORMAT "lu"

#define GLONG_TO_LE(val)	((glong) GINT64_TO_LE (val))
#define GULONG_TO_LE(val)	((gulong) GUINT64_TO_LE (val))
#define GLONG_TO_BE(val)	((glong) GINT64_TO_BE (val))
#define GULONG_TO_BE(val)	((gulong) GUINT64_TO_BE (val))

#define GSIZE_TO_LE(val)	((gsize) GUINT64_TO_LE (val))
#define GSSIZE_TO_LE(val)	((gssize) GINT64_TO_LE (val))
#define GSIZE_TO_BE(val)	((gsize) GUINT64_TO_BE (val))
#define GSSIZE_TO_BE(val)	((gssize) GINT64_TO_BE (val))

#define G_DIR_SEPARATOR '/'
#define G_DIR_SEPARATOR_S "/"
#define G_SEARCHPATH_SEPARATOR ':'
#define G_SEARCHPATH_SEPARATOR_S ":"

#endif /* __G_LIBCONFIG_DEFS_H__ */

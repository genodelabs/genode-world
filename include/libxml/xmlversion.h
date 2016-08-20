#ifndef __XML_VERSION_H__
#define __XML_VERSION_H__

#include <libxml/xmlexports.h>

#define LIBXML_VERSION 20904
#define LIBXML_VERSION_STRING "20904"
#define LIBXML_VERSION_EXTRA "-GITv2.9.4-3-gd8083bf"

#define LIBXML_ATTR_FORMAT(fmt,args) __attribute__((__format__(__printf__,fmt,args)))
#define LIBXML_ATTR_ALLOC_SIZE(x)

#define ATTRIBUTE_UNUSED __attribute__((unused))

#endif

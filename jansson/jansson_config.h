
#ifndef JANSSON_CONFIG_H
#define JANSSON_CONFIG_H

#ifdef HAVE_CONFIG_H
#    include "config.h"
#endif

#ifdef __cplusplus
#define JSON_INLINE inline
#else
#ifdef PHP_WIN32
#define JSON_INLINE __inline
#else
#define JSON_INLINE PHP_JSON_INLINE
#endif
#endif

#ifdef PHP_WIN32
#define JSON_INTEGER_IS_LONG_LONG 1
#else
#define JSON_INTEGER_IS_LONG_LONG PHP_HAVE_JSON_LONG_LONG
#endif

#ifdef PHP_WIN32
#define JSON_HAVE_LOCALECONV 1
#else
#define JSON_HAVE_LOCALECONV PHP_HAVE_JSON_LOCALECONV
#endif

#endif

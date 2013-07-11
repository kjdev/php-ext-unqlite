dnl config.m4 for extension unqlite

dnl Check PHP version:
AC_MSG_CHECKING(PHP version)
if test ! -z "$phpincludedir"; then
    PHP_VERSION=`grep 'PHP_VERSION ' $phpincludedir/main/php_version.h | sed -e 's/.*"\([[0-9\.]]*\)".*/\1/g' 2>/dev/null`
elif test ! -z "$PHP_CONFIG"; then
    PHP_VERSION=`$PHP_CONFIG --version 2>/dev/null`
fi

if test x"$PHP_VERSION" = "x"; then
    AC_MSG_WARN([none])
else
    PHP_MAJOR_VERSION=`echo $PHP_VERSION | sed -e 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\1/g' 2>/dev/null`
    PHP_MINOR_VERSION=`echo $PHP_VERSION | sed -e 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\2/g' 2>/dev/null`
    PHP_RELEASE_VERSION=`echo $PHP_VERSION | sed -e 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\3/g' 2>/dev/null`
    AC_MSG_RESULT([$PHP_VERSION])
fi

if test $PHP_MAJOR_VERSION -lt 5; then
    AC_MSG_ERROR([need at least PHP 5 or newer])
fi

dnl UnQLite Extension
PHP_ARG_ENABLE(unqlite, whether to enable unqlite support,
[  --enable-unqlite      Enable unqlite support])

if test "$PHP_UNQLITE" != "no"; then
    dnl Checks for header files.
    AC_CHECK_HEADERS([locale.h])

    dnl Checks for typedefs, structures, and compiler characteristics.
    AC_TYPE_INT32_T
    AC_TYPE_LONG_LONG_INT

    AC_C_INLINE
    case $ac_cv_c_inline in
        yes) json_inline=inline;;
        no) json_inline=;;
        *) json_inline=$ac_cv_c_inline;;
    esac
    AC_DEFINE_UNQUOTED(PHP_JSON_INLINE,$json_inline, [ ])

    dnl Checks for library functions.
    AC_CHECK_FUNCS([strtoll localeconv])

    case "$ac_cv_type_long_long_int$ac_cv_func_strtoll" in
        yesyes) json_have_long_long=1;;
        *) json_have_long_long=0;;
    esac
    AC_DEFINE_UNQUOTED(PHP_HAVE_JSON_LONG_LONG,$json_have_long_long, [ ])

    case "$ac_cv_header_locale_h$ac_cv_func_localeconv" in
        yesyes) json_have_localeconv=1;;
        *) json_have_localeconv=0;;
    esac
    AC_DEFINE_UNQUOTED(PHP_HAVE_JSON_LOCALECONV,$json_have_localeconv, [ ])

    dnl Source: jansson
    JANSSON_SOURCES="jansson/dump.c jansson/hashtable.c jansson/memory.c jansson/strbuffer.c jansson/utf.c jansson/error.c jansson/load.c jansson/pack_unpack.c jansson/strconv.c jansson/value.c"

    dnl Source: UnQLite
    UNQLITE_SOURCES="unqlite/unqlite.c"

    dnl PHP Extension
    PHP_NEW_EXTENSION(unqlite, unqlite.c unqlite_db.c unqlite_kvs.c unqlite_doc.c unqlite_json.c unqlite_exception.c $UNQLITE_SOURCES $JANSSON_SOURCES, $ext_shared)

    ifdef([PHP_INSTALL_HEADERS],
    [
        PHP_INSTALL_HEADERS([ext/unqlite/], [php_unqlite.h])
    ], [
        PHP_ADD_MAKEFILE_FRAGMENT
    ])
fi

dnl coverage
PHP_ARG_ENABLE(coverage, whether to enable coverage support,
[  --enable-coverage     Enable coverage support], no, no)

if test "$PHP_COVERAGE" != "no"; then
    EXTRA_CFLAGS="--coverage"
    PHP_SUBST(EXTRA_CFLAGS)
fi

/*
 * Copyright (c) 2013 kjdev
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef PHP_UNQLITE_H
#define PHP_UNQLITE_H

#include "unqlite/unqlite.h"

#define UNQLITE_EXT_VERSION "0.1.0"

#define UNQLITE_NS "UnQLite"
#define UNQLITE_ZEND_METHOD(classname, name) \
    ZEND_METHOD(UnQLite_##classname, name)
#define UNQLITE_ZEND_ME(classname, name, arg_info, flags) \
    ZEND_ME(UnQLite_##classname, name, arg_info, flags)
#define UNQLITE_ZEND_MALIAS(classname, name, alias, arg_info, flags) \
    ZEND_MALIAS(UnQLite_##classname, name, alias, arg_info, flags)

extern zend_module_entry unqlite_module_entry;
#define phpext_unqlite_ptr &unqlite_module_entry

#ifdef PHP_WIN32
#    define PHP_UNQLITE_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#    define PHP_UNQLITE_API __attribute__ ((visibility("default")))
#else
#    define PHP_UNQLITE_API
#endif

#ifdef ZTS
#    include "TSRM.h"
#endif

#ifdef ZTS
#    define UNQLITE_G(v) TSRMG(unqlite_globals_id, zend_unqlite_globals *, v)
#else
#    define UNQLITE_G(v) (unqlite_globals.v)
#endif

#endif  /* PHP_UNQLITE_H */

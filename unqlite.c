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

#ifdef HAVE_CONFIG_H
#    include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "php_main.h"
#include "ext/standard/info.h"

#include "php_unqlite.h"
#include "unqlite_db.h"
#include "unqlite_kvs.h"
#include "unqlite_doc.h"
#include "unqlite_exception.h"

ZEND_DECLARE_MODULE_GLOBALS(unqlite)

ZEND_INI_BEGIN()
ZEND_INI_END()

/*
static void
php_unqlite_init_globals(zend_unqlite_globals *unqlite_globals)
{}
*/

#define UNQLITE_LONG_CONSTANT(name, val) \
    REGISTER_NS_LONG_CONSTANT(UNQLITE_NS, name, val, CONST_CS|CONST_PERSISTENT)
#define UNQLITE_STRING_CONSTANT(name, val) \
    REGISTER_NS_STRING_CONSTANT(UNQLITE_NS, name, val, CONST_CS|CONST_PERSISTENT)

ZEND_MINIT_FUNCTION(unqlite)
{
    /* ZEND_INIT_MODULE_GLOBALS(unqlite, php_unqlite_init_globals, NULL); */

    php_unqlite_db_class_register(TSRMLS_C);
    php_unqlite_kvs_class_register(TSRMLS_C);
    php_unqlite_doc_class_register(TSRMLS_C);
    php_unqlite_exception_class_register(TSRMLS_C);

    UNQLITE_LONG_CONSTANT("CONFIG_DISABLE_AUTO_COMMIT",
                          UNQLITE_CONFIG_DISABLE_AUTO_COMMIT);

    UNQLITE_LONG_CONSTANT("OPEN_READONLY", UNQLITE_OPEN_READONLY);
    UNQLITE_LONG_CONSTANT("OPEN_READWRITE", UNQLITE_OPEN_READWRITE);
    UNQLITE_LONG_CONSTANT("OPEN_CREATE", UNQLITE_OPEN_CREATE);
    UNQLITE_LONG_CONSTANT("OPEN_EXCLUSIVE", UNQLITE_OPEN_EXCLUSIVE);
    UNQLITE_LONG_CONSTANT("OPEN_TEMP_DB", UNQLITE_OPEN_TEMP_DB);
    UNQLITE_LONG_CONSTANT("OPEN_NOMUTEX", UNQLITE_OPEN_NOMUTEX);
    UNQLITE_LONG_CONSTANT("OPEN_OMIT_JOURNALING", UNQLITE_OPEN_OMIT_JOURNALING);
    UNQLITE_LONG_CONSTANT("OPEN_IN_MEMORY", UNQLITE_OPEN_IN_MEMORY);
    UNQLITE_LONG_CONSTANT("OPEN_MMAP", UNQLITE_OPEN_MMAP);

    UNQLITE_LONG_CONSTANT("CURSOR_MATCH_EXACT", UNQLITE_CURSOR_MATCH_EXACT);
    UNQLITE_LONG_CONSTANT("CURSOR_MATCH_LE", UNQLITE_CURSOR_MATCH_LE);
    UNQLITE_LONG_CONSTANT("CURSOR_MATCH_GE", UNQLITE_CURSOR_MATCH_GE);

    UNQLITE_LONG_CONSTANT("CURSOR_FIRST", PHP_UNQLITE_CURSOR_FIRST);
    UNQLITE_LONG_CONSTANT("CURSOR_LAST", PHP_UNQLITE_CURSOR_LAST);

    UNQLITE_STRING_CONSTANT("DB_MEMORY", ":mem:");

    REGISTER_INI_ENTRIES();

    return SUCCESS;
}

ZEND_MSHUTDOWN_FUNCTION(unqlite)
{
    UNREGISTER_INI_ENTRIES();

    return SUCCESS;
}

ZEND_MINFO_FUNCTION(unqlite)
{
    php_info_print_table_start();
    php_info_print_table_row(2, "unqlite support", "enabled");
    php_info_print_table_row(2, "Extension Version", UNQLITE_EXT_VERSION);
    php_info_print_table_end();
}

zend_module_entry unqlite_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "unqlite",
    NULL,
    ZEND_MINIT(unqlite),
    ZEND_MSHUTDOWN(unqlite),
    NULL,
    NULL,
    ZEND_MINFO(unqlite),
#if ZEND_MODULE_API_NO >= 20010901
    UNQLITE_EXT_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_UNQLITE
ZEND_GET_MODULE(unqlite)
#endif

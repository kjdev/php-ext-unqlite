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

#ifndef UNQLITE_KVS_H
#define UNQLITE_KVS_H

#include "php_unqlite.h"

#define PHP_UNQLITE_CURSOR_FIRST 1
#define PHP_UNQLITE_CURSOR_LAST  2

extern PHP_UNQLITE_API zend_class_entry *php_unqlite_kvs_ce;

typedef struct {
    zend_object std;
    zval *link;
} php_unqlite_kvs_t;

PHP_UNQLITE_API int php_unqlite_kvs_class_register(TSRMLS_D);
PHP_UNQLITE_API int php_unqlite_kvs_class_init(php_unqlite_kvs_t *intern
                                               TSRMLS_DC);

#endif  /* UNQLITE_KVS_H */

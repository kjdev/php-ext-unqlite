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

#include "unqlite_db.h"
#include "unqlite_kvs.h"
#include "unqlite_exception.h"

ZEND_EXTERN_MODULE_GLOBALS(unqlite)

zend_class_entry *php_unqlite_kvs_ce;
static zend_object_handlers php_unqlite_kvs_handlers;

static zend_class_entry *php_unqlite_kvs_cursor_ce;
static zend_object_handlers php_unqlite_kvs_cursor_handlers;
typedef struct {
    zend_object std;
    zval *link;
    unqlite_kv_cursor *cursor;
} php_unqlite_kvs_cursor_t;

ZEND_BEGIN_ARG_INFO_EX(arginfo_unqlite_kvs___construct, 0, 0, 1)
    ZEND_ARG_INFO(0, db)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_unqlite_kvs_store, 0, 0, 2)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_unqlite_kvs_append, 0, 0, 2)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_unqlite_kvs_fetch, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_unqlite_kvs_remove, 0, 0, 1)
    ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_unqlite_kvs_cursor, 0, 0, 1)
    ZEND_ARG_INFO(0, option)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_unqlite_kvs_cursor___construct, 0, 0, 1)
    ZEND_ARG_INFO(0, kvs)
    ZEND_ARG_INFO(0, option)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_unqlite_kvs_cursor_seek, 0, 0, 2)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, option)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_unqlite_kvs_none, 0, 0, 0)
ZEND_END_ARG_INFO()

#define php_error_out_of_memory() \
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "unable to allocate memory")

static inline unqlite *
php_unqlite_get_db(zval *link TSRMLS_DC)
{
    php_unqlite_db_t *intern;

    intern = (php_unqlite_db_t *)zend_object_store_get_object(link TSRMLS_CC);
    if (intern && intern->db) {
        return intern->db;
    }

    return NULL;
}

#define UNQLITE_KVS_DB(self) php_unqlite_get_db((self)->link TSRMLS_CC)

#define UNQLITE_KVS(self)                                     \
    self = (php_unqlite_kvs_t *)zend_object_store_get_object( \
        getThis() TSRMLS_CC)

static void
php_unqlite_kvs_error(php_unqlite_kvs_t *intern, int flags TSRMLS_DC)
{
    if (!intern || !intern->link) {
        return;
    }

    php_unqlite_error(UNQLITE_KVS_DB(intern), flags TSRMLS_CC);
}

UNQLITE_ZEND_METHOD(Kvs, __construct)
{
    int rc;
    php_unqlite_kvs_t *intern;
    zval *link;
#if ZEND_MODULE_API_NO >= 20090626
    zend_error_handling error_handling;
    zend_replace_error_handling(EH_THROW, NULL, &error_handling TSRMLS_CC);
#else
    php_set_error_handling(EH_THROW, NULL TSRMLS_CC);
#endif

    rc = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O",
                               &link, php_unqlite_db_ce);

#if ZEND_MODULE_API_NO >= 20090626
    zend_restore_error_handling(&error_handling TSRMLS_CC);
#else
    php_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
#endif
    if (rc == FAILURE) {
        return;
    }

    UNQLITE_KVS(intern);

    intern->link = link;
    zval_add_ref(&intern->link);

    php_unqlite_kvs_class_init(intern TSRMLS_CC);
}

UNQLITE_ZEND_METHOD(Kvs, store)
{
    php_unqlite_kvs_t *intern;
    char *key, *val;
    int rc, key_len, val_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",
                              &key, &key_len, &val, &val_len) == FAILURE) {
        return;
    }

    UNQLITE_KVS(intern);

    rc = unqlite_kv_store(UNQLITE_KVS_DB(intern), key, key_len, val, val_len);
    if (rc == UNQLITE_OK) {
        RETURN_TRUE;
    }

    /* fail */
    php_unqlite_kvs_error(intern, E_WARNING TSRMLS_CC);

    /* rollback the transaction  */
    if (rc != UNQLITE_BUSY && rc != UNQLITE_NOTIMPLEMENTED) {
        if (unqlite_rollback(UNQLITE_KVS_DB(intern)) != UNQLITE_OK) {
            php_unqlite_kvs_error(intern, E_WARNING TSRMLS_CC);
        }
    }

    RETURN_FALSE;
}

UNQLITE_ZEND_METHOD(Kvs, append)
{
    php_unqlite_kvs_t *intern;
    char *key, *val;
    int rc, key_len, val_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",
                              &key, &key_len, &val, &val_len) == FAILURE) {
        return;
    }

    UNQLITE_KVS(intern);

    rc = unqlite_kv_append(UNQLITE_KVS_DB(intern), key, key_len, val, val_len);
    if (rc == UNQLITE_OK) {
        RETURN_TRUE;
    }

    /* fail */
    php_unqlite_kvs_error(intern, E_WARNING TSRMLS_CC);

    /* rollback the transaction  */
    if (rc != UNQLITE_BUSY && rc != UNQLITE_NOTIMPLEMENTED) {
        if (unqlite_rollback(UNQLITE_KVS_DB(intern)) != UNQLITE_OK) {
            php_unqlite_kvs_error(intern, E_WARNING TSRMLS_CC);
        }
    }

    RETURN_FALSE;
}

UNQLITE_ZEND_METHOD(Kvs, fetch)
{
    php_unqlite_kvs_t *intern;
    char *key;
    int key_len;
    char *buf = NULL;
    unqlite_int64 len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
                              &key, &key_len) == FAILURE) {
        return;
    }

    UNQLITE_KVS(intern);

    if (unqlite_kv_fetch(UNQLITE_KVS_DB(intern),
                         key, key_len, NULL, &len) != UNQLITE_OK) {
        RETURN_NULL();
    }

    buf = (char *)emalloc(len + 1);
    if (!buf) {
        php_error_out_of_memory();
        RETURN_NULL();
    }

    if (unqlite_kv_fetch(UNQLITE_KVS_DB(intern),
                         key, key_len, buf, &len) == UNQLITE_OK) {
        if (buf) {
            RETVAL_STRINGL(buf, len, 1);
            efree(buf);
        } else {
            RETURN_NULL();
        }
        return;
    }

    /* fail */
    php_unqlite_kvs_error(intern, E_WARNING TSRMLS_CC);

    if (buf) {
        efree(buf);
    }

    RETURN_NULL();
}

UNQLITE_ZEND_METHOD(Kvs, remove)
{
    php_unqlite_kvs_t *intern;
    char *key;
    int key_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
                              &key, &key_len) == FAILURE) {
        return;
    }

    UNQLITE_KVS(intern);

    if (unqlite_kv_delete(UNQLITE_KVS_DB(intern), key, key_len) == UNQLITE_OK) {
        RETURN_TRUE;
    }

    /* fail */
    php_unqlite_kvs_error(intern, E_WARNING TSRMLS_CC);

    /* rollback the transaction  */
    if (unqlite_rollback(UNQLITE_KVS_DB(intern)) != UNQLITE_OK) {
        php_unqlite_kvs_error(intern, E_WARNING TSRMLS_CC);
    }

    RETURN_FALSE;
}

UNQLITE_ZEND_METHOD(Kvs, begin)
{
    php_unqlite_kvs_t *intern;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    UNQLITE_KVS(intern);

    if (unqlite_begin(UNQLITE_KVS_DB(intern)) == UNQLITE_OK) {
        RETURN_TRUE;
    }

    php_unqlite_kvs_error(intern, E_WARNING TSRMLS_CC);

    RETURN_FALSE;
}

UNQLITE_ZEND_METHOD(Kvs, commit)
{
    php_unqlite_kvs_t *intern;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    UNQLITE_KVS(intern);

    if (unqlite_commit(UNQLITE_KVS_DB(intern)) == UNQLITE_OK) {
        RETURN_TRUE;
    }

    php_unqlite_kvs_error(intern, E_WARNING TSRMLS_CC);

    RETURN_FALSE;
}

UNQLITE_ZEND_METHOD(Kvs, rollback)
{
    php_unqlite_kvs_t *intern;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    UNQLITE_KVS(intern);

    if (unqlite_rollback(UNQLITE_KVS_DB(intern)) == UNQLITE_OK) {
        RETURN_TRUE;
    }

    php_unqlite_kvs_error(intern, E_WARNING TSRMLS_CC);

    RETURN_FALSE;
}

UNQLITE_ZEND_METHOD(Kvs, cursor)
{
    php_unqlite_kvs_t *intern;
    php_unqlite_kvs_cursor_t *cursor;
    long pos = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l",
                              &pos) == FAILURE) {
        return;
    }

    UNQLITE_KVS(intern);

    object_init_ex(return_value, php_unqlite_kvs_cursor_ce);
    cursor = zend_object_store_get_object(return_value TSRMLS_CC);
    if (!cursor) {
        UNQLITE_EXCEPTION("The UnQLite\\KvsCursor object has not been "
                          "correctly initialized by its constructor");
        return;
    }

    cursor->link = intern->link;
    zval_add_ref(&cursor->link);

    if (unqlite_kv_cursor_init(UNQLITE_KVS_DB(cursor),
                               &cursor->cursor) != UNQLITE_OK ){
        UNQLITE_EXCEPTION("unable to init UnQLite cursor");
        return;
    }

    /* Point to the record */
    if (pos == PHP_UNQLITE_CURSOR_FIRST) {
        unqlite_kv_cursor_first_entry(cursor->cursor);
    } else if (pos == PHP_UNQLITE_CURSOR_LAST) {
        unqlite_kv_cursor_last_entry(cursor->cursor);
    }
}

#define UNQLITE_KVS_CURSOR(self)                                     \
    self = (php_unqlite_kvs_cursor_t *)zend_object_store_get_object( \
        getThis() TSRMLS_CC)

static void
php_unqlite_kvs_cursor_error(php_unqlite_kvs_cursor_t *intern,
                             int flags TSRMLS_DC)
{
    if (!intern || !intern->link) {
        return;
    }

    php_unqlite_error(UNQLITE_KVS_DB(intern), flags TSRMLS_CC);
}

UNQLITE_ZEND_METHOD(KvsCursor, __construct)
{
    int rc;
    php_unqlite_kvs_cursor_t *intern;
    php_unqlite_kvs_t *kvs;
    zval *link;
    long pos = -1;
#if ZEND_MODULE_API_NO >= 20090626
    zend_error_handling error_handling;
    zend_replace_error_handling(EH_THROW, NULL, &error_handling TSRMLS_CC);
#else
    php_set_error_handling(EH_THROW, NULL TSRMLS_CC);
#endif

    rc = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O|l",
                               &link, php_unqlite_kvs_ce, &pos);

#if ZEND_MODULE_API_NO >= 20090626
    zend_restore_error_handling(&error_handling TSRMLS_CC);
#else
    php_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
#endif
    if (rc == FAILURE) {
        return;
    }

    UNQLITE_KVS_CURSOR(intern);

    kvs = (php_unqlite_kvs_t *)zend_object_store_get_object(link TSRMLS_CC);
    intern->link = kvs->link;
    zval_add_ref(&intern->link);

    if (unqlite_kv_cursor_init(UNQLITE_KVS_DB(intern),
                               &intern->cursor) != UNQLITE_OK ){
        UNQLITE_EXCEPTION("unable to init UnQLite cursor");
        return;
    }

    /* Point to the record */
    if (pos == PHP_UNQLITE_CURSOR_FIRST) {
        unqlite_kv_cursor_first_entry(intern->cursor);
    } else if (pos == PHP_UNQLITE_CURSOR_LAST) {
        unqlite_kv_cursor_last_entry(intern->cursor);
    }
}

UNQLITE_ZEND_METHOD(KvsCursor, first)
{
    php_unqlite_kvs_cursor_t *intern;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    UNQLITE_KVS_CURSOR(intern);

    if (unqlite_kv_cursor_first_entry(intern->cursor) == UNQLITE_OK) {
        RETURN_TRUE;
    }

    RETURN_FALSE;
}

UNQLITE_ZEND_METHOD(KvsCursor, last)
{
    php_unqlite_kvs_cursor_t *intern;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    UNQLITE_KVS_CURSOR(intern);

    if (unqlite_kv_cursor_last_entry(intern->cursor) == UNQLITE_OK) {
        RETURN_TRUE;
    }

    RETURN_FALSE;
}

UNQLITE_ZEND_METHOD(KvsCursor, next)
{
    php_unqlite_kvs_cursor_t *intern;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    UNQLITE_KVS_CURSOR(intern);

    if (unqlite_kv_cursor_next_entry(intern->cursor) == UNQLITE_OK) {
        RETURN_TRUE;
    }

    RETURN_FALSE;
}

UNQLITE_ZEND_METHOD(KvsCursor, prev)
{
    php_unqlite_kvs_cursor_t *intern;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    UNQLITE_KVS_CURSOR(intern);

    if (unqlite_kv_cursor_prev_entry(intern->cursor) == UNQLITE_OK) {
        RETURN_TRUE;
    }

    RETURN_FALSE;
}

UNQLITE_ZEND_METHOD(KvsCursor, exists)
{
    php_unqlite_kvs_cursor_t *intern;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    UNQLITE_KVS_CURSOR(intern);

    if (unqlite_kv_cursor_valid_entry(intern->cursor)) {
        RETURN_TRUE;
    }

    RETURN_FALSE;
}

UNQLITE_ZEND_METHOD(KvsCursor, remove)
{
    php_unqlite_kvs_cursor_t *intern;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    UNQLITE_KVS_CURSOR(intern);

    if (unqlite_kv_cursor_delete_entry(intern->cursor) == UNQLITE_OK) {
        RETURN_TRUE;
    }

    RETURN_FALSE;
}

UNQLITE_ZEND_METHOD(KvsCursor, seek)
{
    php_unqlite_kvs_cursor_t *intern;
    char *key;
    int key_len;
    long opt = UNQLITE_CURSOR_MATCH_EXACT;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l",
                              &key, &key_len, &opt) == FAILURE) {
        return;
    }

    UNQLITE_KVS_CURSOR(intern);

    if (unqlite_kv_cursor_seek(intern->cursor, key, key_len,
                               opt) == UNQLITE_OK) {
        RETURN_TRUE;
    }

    RETURN_FALSE;
}

UNQLITE_ZEND_METHOD(KvsCursor, key)
{
    php_unqlite_kvs_cursor_t *intern;
    char *buf = NULL;
    int len = 0;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    UNQLITE_KVS_CURSOR(intern);

    if (unqlite_kv_cursor_key(intern->cursor, NULL, &len) != UNQLITE_OK) {
        RETURN_FALSE;
    }

    buf = (char *)emalloc(len + 1);
    if (!buf) {
        php_error_out_of_memory();
        RETURN_FALSE;
    }

    if (unqlite_kv_cursor_key(intern->cursor, buf, &len) == UNQLITE_OK) {
        if (buf) {
            RETVAL_STRINGL(buf, len, 1);
            efree(buf);
        } else {
            RETVAL_EMPTY_STRING();
        }
        return;
    }

    /* fail */
    php_unqlite_kvs_cursor_error(intern, E_WARNING TSRMLS_CC);

    if (buf) {
        efree(buf);
    }

    RETURN_FALSE;
}

UNQLITE_ZEND_METHOD(KvsCursor, data)
{
    php_unqlite_kvs_cursor_t *intern;
    char *buf = NULL;
    unqlite_int64 len = 0;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    UNQLITE_KVS_CURSOR(intern);

    if (unqlite_kv_cursor_data(intern->cursor, NULL, &len) != UNQLITE_OK) {
        RETURN_NULL();
    }

    buf = (char *)emalloc(len + 1);
    if (!buf) {
        php_error_out_of_memory();
        RETURN_NULL();
    }

    if (unqlite_kv_cursor_data(intern->cursor, buf, &len) == UNQLITE_OK) {
        if (buf) {
            RETVAL_STRINGL(buf, len, 1);
            efree(buf);
        } else {
            RETVAL_EMPTY_STRING();
        }
        return;
    }

    /* fail */
    php_unqlite_kvs_cursor_error(intern, E_WARNING TSRMLS_CC);

    if (buf) {
        efree(buf);
    }

    RETURN_NULL();
}

static zend_function_entry php_unqlite_kvs_methods[] = {
    UNQLITE_ZEND_ME(Kvs, __construct, arginfo_unqlite_kvs___construct,
                    ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    UNQLITE_ZEND_ME(Kvs, store, arginfo_unqlite_kvs_store, ZEND_ACC_PUBLIC)
    UNQLITE_ZEND_ME(Kvs, append, arginfo_unqlite_kvs_append, ZEND_ACC_PUBLIC)
    UNQLITE_ZEND_ME(Kvs, fetch, arginfo_unqlite_kvs_fetch, ZEND_ACC_PUBLIC)
    UNQLITE_ZEND_ME(Kvs, remove, arginfo_unqlite_kvs_remove, ZEND_ACC_PUBLIC)
    UNQLITE_ZEND_ME(Kvs, cursor, arginfo_unqlite_kvs_cursor, ZEND_ACC_PUBLIC)
    UNQLITE_ZEND_ME(Kvs, begin, arginfo_unqlite_kvs_none, ZEND_ACC_PUBLIC)
    UNQLITE_ZEND_ME(Kvs, commit, arginfo_unqlite_kvs_none, ZEND_ACC_PUBLIC)
    UNQLITE_ZEND_ME(Kvs, rollback, arginfo_unqlite_kvs_none, ZEND_ACC_PUBLIC)
    UNQLITE_ZEND_MALIAS(Kvs, delete, remove,
                        arginfo_unqlite_kvs_remove, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

static void
php_unqlite_kvs_free_storage(void *object TSRMLS_DC)
{
    php_unqlite_kvs_t *intern = (php_unqlite_kvs_t *)object;

    if (!intern) {
        return;
    }

    if (intern->link) {
        zval_ptr_dtor(&intern->link);
    }

    zend_object_std_dtor(&intern->std TSRMLS_CC);
    efree(object);
}

static inline zend_object_value
php_unqlite_kvs_new_ex(zend_class_entry *ce, php_unqlite_kvs_t **ptr TSRMLS_DC)
{
    php_unqlite_kvs_t *intern;
    zend_object_value retval;
#if ZEND_MODULE_API_NO < 20100525
    zval *tmp;
#endif

    intern = emalloc(sizeof(php_unqlite_kvs_t));
    memset(intern, 0, sizeof(php_unqlite_kvs_t));
    if (ptr) {
        *ptr = intern;
    }

    zend_object_std_init(&intern->std, ce TSRMLS_CC);

#if ZEND_MODULE_API_NO >= 20100525
    object_properties_init(&intern->std, ce);
#else
    zend_hash_copy(intern->std.properties, &ce->default_properties,
                   (copy_ctor_func_t)zval_add_ref, (void *)&tmp, sizeof(zval *));
#endif

    retval.handle = zend_objects_store_put(
        intern, (zend_objects_store_dtor_t)zend_objects_destroy_object,
        (zend_objects_free_object_storage_t)php_unqlite_kvs_free_storage,
        NULL TSRMLS_CC);
    retval.handlers = &php_unqlite_kvs_handlers;

    intern->link = NULL;

    return retval;
}

static inline zend_object_value
php_unqlite_kvs_new(zend_class_entry *ce TSRMLS_DC)
{
    return php_unqlite_kvs_new_ex(ce, NULL TSRMLS_CC);
}

static zend_function_entry php_unqlite_kvs_cursor_methods[] = {
    UNQLITE_ZEND_ME(KvsCursor, __construct,
                    arginfo_unqlite_kvs_cursor___construct,
                    ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    UNQLITE_ZEND_ME(KvsCursor, first, arginfo_unqlite_kvs_none, ZEND_ACC_PUBLIC)
    UNQLITE_ZEND_ME(KvsCursor, last, arginfo_unqlite_kvs_none, ZEND_ACC_PUBLIC)
    UNQLITE_ZEND_ME(KvsCursor, next, arginfo_unqlite_kvs_none, ZEND_ACC_PUBLIC)
    UNQLITE_ZEND_ME(KvsCursor, prev, arginfo_unqlite_kvs_none, ZEND_ACC_PUBLIC)
    UNQLITE_ZEND_ME(KvsCursor, seek,
                    arginfo_unqlite_kvs_cursor_seek, ZEND_ACC_PUBLIC)
    UNQLITE_ZEND_ME(KvsCursor, exists, arginfo_unqlite_kvs_none, ZEND_ACC_PUBLIC)
    UNQLITE_ZEND_ME(KvsCursor, remove, arginfo_unqlite_kvs_none, ZEND_ACC_PUBLIC)
    UNQLITE_ZEND_ME(KvsCursor, key, arginfo_unqlite_kvs_none, ZEND_ACC_PUBLIC)
    UNQLITE_ZEND_ME(KvsCursor, data, arginfo_unqlite_kvs_none, ZEND_ACC_PUBLIC)
    UNQLITE_ZEND_MALIAS(KvsCursor, delete, remove,
                        arginfo_unqlite_kvs_none, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

static void
php_unqlite_kvs_cursor_free_storage(void *object TSRMLS_DC)
{
    php_unqlite_kvs_cursor_t *intern = (php_unqlite_kvs_cursor_t *)object;

    if (!intern) {
        return;
    }

    if (intern->cursor) {
        unqlite_kv_cursor_release(UNQLITE_KVS_DB(intern), intern->cursor);
    }

    if (intern->link) {
        zval_ptr_dtor(&intern->link);
    }

    zend_object_std_dtor(&intern->std TSRMLS_CC);
    efree(object);
}

static inline zend_object_value
php_unqlite_kvs_cursor_new_ex(zend_class_entry *ce,
                              php_unqlite_kvs_cursor_t **ptr TSRMLS_DC)
{
    php_unqlite_kvs_cursor_t *intern;
    zend_object_value retval;
#if ZEND_MODULE_API_NO < 20100525
    zval *tmp;
#endif

    intern = emalloc(sizeof(php_unqlite_kvs_cursor_t));
    memset(intern, 0, sizeof(php_unqlite_kvs_cursor_t));
    if (ptr) {
        *ptr = intern;
    }

    zend_object_std_init(&intern->std, ce TSRMLS_CC);

#if ZEND_MODULE_API_NO >= 20100525
    object_properties_init(&intern->std, ce);
#else
    zend_hash_copy(intern->std.properties, &ce->default_properties,
                   (copy_ctor_func_t)zval_add_ref, (void *)&tmp, sizeof(zval *));
#endif

    retval.handle = zend_objects_store_put(
        intern, (zend_objects_store_dtor_t)zend_objects_destroy_object,
        (zend_objects_free_object_storage_t)php_unqlite_kvs_cursor_free_storage,
        NULL TSRMLS_CC);
    retval.handlers = &php_unqlite_kvs_cursor_handlers;

    intern->link = NULL;
    intern->cursor = NULL;

    return retval;
}

static inline zend_object_value
php_unqlite_kvs_cursor_new(zend_class_entry *ce TSRMLS_DC)
{
    return php_unqlite_kvs_cursor_new_ex(ce, NULL TSRMLS_CC);
}

PHP_UNQLITE_API int
php_unqlite_kvs_class_init(php_unqlite_kvs_t *intern TSRMLS_DC)
{
    return SUCCESS;
}

PHP_UNQLITE_API int
php_unqlite_kvs_class_register(TSRMLS_D)
{
    zend_class_entry kvs_ce;
    zend_class_entry kvs_cursor_ce;

    /* Kvs */
    INIT_CLASS_ENTRY(kvs_ce, ZEND_NS_NAME(UNQLITE_NS, "Kvs"),
                     php_unqlite_kvs_methods);

    kvs_ce.create_object = php_unqlite_kvs_new;

    php_unqlite_kvs_ce = zend_register_internal_class(&kvs_ce TSRMLS_CC);
    if (php_unqlite_kvs_ce == NULL) {
        return FAILURE;
    }

    memcpy(&php_unqlite_kvs_handlers, zend_get_std_object_handlers(),
           sizeof(zend_object_handlers));

    php_unqlite_kvs_handlers.clone_obj = NULL;

    /* KvsCursor */
    INIT_CLASS_ENTRY(kvs_cursor_ce, ZEND_NS_NAME(UNQLITE_NS, "KvsCursor"),
                     php_unqlite_kvs_cursor_methods);

    kvs_cursor_ce.create_object = php_unqlite_kvs_cursor_new;

    php_unqlite_kvs_cursor_ce = zend_register_internal_class(&kvs_cursor_ce
                                                             TSRMLS_CC);
    if (php_unqlite_kvs_cursor_ce == NULL) {
        return FAILURE;
    }

    memcpy(&php_unqlite_kvs_cursor_handlers,
           zend_get_std_object_handlers(), sizeof(zend_object_handlers));

    php_unqlite_kvs_cursor_handlers.clone_obj = NULL;

    return SUCCESS;
}

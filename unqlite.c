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
#include "zend_exceptions.h"
#include "ext/standard/info.h"
/*#include "ext/standard/php_var.h"*/

#include "php_unqlite.h"
#include "unqlite/unqlite.h"

#define PHP_UNQLITE_CURSOR_FIRST 1
#define PHP_UNQLITE_CURSOR_LAST  2

#define ZEND_UNQLITE_METHOD(classname, name) \
  ZEND_METHOD(UnQLite_##classname, name)
#define ZEND_UNQLITE_ME(classname, name, arg_info, flags) \
  ZEND_ME(UnQLite_##classname, name, arg_info, flags)

#define _ERR(_flag,...) php_error_docref(NULL TSRMLS_CC, _flag, __VA_ARGS__)

ZEND_DECLARE_MODULE_GLOBALS(unqlite)

static zend_class_entry *php_unqlite_kv_ce;
static zend_object_handlers php_unqlite_kv_handlers;
typedef struct {
    zend_object std;
    int init;
    unqlite *db;
} php_unqlite_kv_t;

static zend_class_entry *php_unqlite_kv_cursor_ce;
static zend_object_handlers php_unqlite_kv_cursor_handlers;
typedef struct {
    zend_object std;
    zval *link;
    unqlite_kv_cursor *cursor;
} php_unqlite_kv_cursor_t;

ZEND_BEGIN_ARG_INFO_EX(arginfo_unqlite_none, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_unqlite_key_val, 0, 0, 2)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_unqlite_key, 0, 0, 1)
    ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_unqlite_option, 0, 0, 0)
    ZEND_ARG_INFO(0, option)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_unqlite_key_option, 0, 0, 1)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, option)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_unqlite_kv___construct, 0, 0, 1)
    ZEND_ARG_INFO(0, filename)
    ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_unqlite_kv_cursor___construct, 0, 0, 1)
    ZEND_ARG_INFO(0, kv)
    ZEND_ARG_INFO(0, option)
ZEND_END_ARG_INFO()

#define php_error_out_of_memory() \
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to allocate memory")

static void
php_unqlite_kv_error(php_unqlite_kv_t *intern, int flags TSRMLS_DC)
{
    const char *msg = NULL;
    int len = 0;

    if (!intern || !intern->init || !intern->db) {
        return;
    }

    unqlite_config(intern->db, UNQLITE_CONFIG_ERR_LOG, &msg, &len);
    if (msg) {
        php_error_docref(NULL TSRMLS_CC, flags, msg);
    } else {
        php_error_docref(NULL TSRMLS_CC, flags, "Unqlite error");
    }
}

static inline unqlite *
php_unqlite_kv_get_db(zval *link TSRMLS_DC)
{
    php_unqlite_kv_t *intern;

    intern = (php_unqlite_kv_t *)zend_object_store_get_object(link TSRMLS_CC);
    if (intern && intern->db) {
        return intern->db;
    }

    return NULL;
}

#define UNQLITE_KV(name)                                     \
    name = (php_unqlite_kv_t *)zend_object_store_get_object( \
        getThis() TSRMLS_CC)
#define UNQLITE_KV_CHECKED(name)                        \
    UNQLITE_KV(name);                                   \
    if (!((name)->init)) {                              \
        php_error_docref(NULL TSRMLS_CC, E_WARNING,     \
                         "The UnQLite object has not "  \
                         "been correctly initialized"); \
        RETURN_FALSE;                                   \
    }

ZEND_UNQLITE_METHOD(Kv, __construct)
{
    int rc;
    php_unqlite_kv_t *intern;
    char *filename, *fullpath = NULL;
    int filename_len;
    long mode = UNQLITE_OPEN_CREATE;
#if ZEND_MODULE_API_NO >= 20090626
    zend_error_handling error_handling;
    zend_replace_error_handling(EH_THROW, NULL, &error_handling TSRMLS_CC);
#else
    php_set_error_handling(EH_THROW, NULL TSRMLS_CC);
#endif

    rc = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "p|l",
                               &filename, &filename_len, &mode);

#if ZEND_MODULE_API_NO >= 20090626
    zend_restore_error_handling(&error_handling TSRMLS_CC);
#else
    php_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
#endif
    if (rc == FAILURE) {
        return;
    }

    UNQLITE_KV(intern);

    if (intern->init) {
        zend_throw_exception(zend_exception_get_default(TSRMLS_C),
                             "Already initialized DB Object", 0 TSRMLS_CC);
    }

    if (strlen(filename) != filename_len) {
        return;
    }

    if (memcmp(filename, ":mem:", sizeof(":mem:")) != 0) {
        if (!(fullpath = expand_filepath(filename, NULL TSRMLS_CC))) {
            zend_throw_exception(zend_exception_get_default(TSRMLS_C),
                                 "Unable to expand filepath", 0 TSRMLS_CC);
            return;
        }
#if PHP_API_VERSION < 20100412
        if (PG(safe_mode) &&
            (!php_checkuid(fullpath, NULL, CHECKUID_CHECK_FILE_AND_DIR))) {
            zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C),
                                    0 TSRMLS_CC,
                                    "safe_mode prohibits opening %s", fullpath);
            efree(fullpath);
            return;
        }
#endif
    } else {
        fullpath = estrdup(filename);
    }

    if (mode <= 0) {
        mode = UNQLITE_OPEN_CREATE;
    }

    if (unqlite_open(&(intern->db), fullpath, mode) != UNQLITE_OK) {
        zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C),
                                0 TSRMLS_CC, "Unable to open database: %s",
                                fullpath);
        if (fullpath) {
            efree(fullpath);
        }
        return;
    }

    intern->init = 1;

    if (fullpath) {
        efree(fullpath);
    }
}

ZEND_UNQLITE_METHOD(Kv, close)
{
    php_unqlite_kv_t *intern;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    UNQLITE_KV(intern);

    if (intern->init) {
        if (unqlite_close(intern->db) != UNQLITE_OK) {
            php_unqlite_kv_error(intern, E_ERROR TSRMLS_CC);
            RETURN_FALSE;
        }
        intern->init = 0;
    }

    RETURN_TRUE;
}

ZEND_UNQLITE_METHOD(Kv, store)
{
    php_unqlite_kv_t *intern;
    char *key, *val;
    int rc, key_len, val_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",
                              &key, &key_len, &val, &val_len) == FAILURE) {
        return;
    }

    UNQLITE_KV_CHECKED(intern);

    rc = unqlite_kv_store(intern->db, key, key_len, val, val_len);
    if (rc == UNQLITE_OK) {
        RETURN_TRUE;
    }

    /* fail */
    php_unqlite_kv_error(intern, E_WARNING TSRMLS_CC);

    /* rollback the transaction  */
    if (rc != UNQLITE_BUSY && rc != UNQLITE_NOTIMPLEMENTED) {
        if (unqlite_rollback(intern->db) != UNQLITE_OK) {
            php_unqlite_kv_error(intern, E_WARNING TSRMLS_CC);
        }
    }

    RETURN_FALSE;
}

ZEND_UNQLITE_METHOD(Kv, append)
{
    php_unqlite_kv_t *intern;
    char *key, *val;
    int rc, key_len, val_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",
                              &key, &key_len, &val, &val_len) == FAILURE) {
        return;
    }

    UNQLITE_KV_CHECKED(intern);

    rc = unqlite_kv_append(intern->db, key, key_len, val, val_len);
    if (rc == UNQLITE_OK) {
        RETURN_TRUE;
    }

    /* fail */
    php_unqlite_kv_error(intern, E_WARNING TSRMLS_CC);

    /* rollback the transaction  */
    if (rc != UNQLITE_BUSY && rc != UNQLITE_NOTIMPLEMENTED) {
        if (unqlite_rollback(intern->db) != UNQLITE_OK) {
            php_unqlite_kv_error(intern, E_WARNING TSRMLS_CC);
        }
    }

    RETURN_FALSE;
}

ZEND_UNQLITE_METHOD(Kv, fetch)
{
    php_unqlite_kv_t *intern;
    char *key;
    int key_len;
    char *buf;
    unqlite_int64 len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
                              &key, &key_len) == FAILURE) {
        return;
    }

    UNQLITE_KV_CHECKED(intern);

    if (unqlite_kv_fetch(intern->db, key, key_len, NULL, &len) != UNQLITE_OK) {
        RETURN_FALSE;
    }

    buf = (char *)emalloc(len + 1);
    if (!buf) {
        php_error_out_of_memory();
        RETURN_FALSE;
    }

    if (unqlite_kv_fetch(intern->db, key, key_len, buf, &len) == UNQLITE_OK) {
        if (buf) {
            RETVAL_STRINGL(buf, len, 1);
            efree(buf);
        } else {
            RETVAL_EMPTY_STRING();
        }
        return;
    }

    /* fail */
    php_unqlite_kv_error(intern, E_WARNING TSRMLS_CC);

    if (buf) {
        efree(buf);
    }

    RETURN_FALSE;
}

ZEND_UNQLITE_METHOD(Kv, delete)
{
    php_unqlite_kv_t *intern;
    char *key;
    int key_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
                              &key, &key_len) == FAILURE) {
        return;
    }

    UNQLITE_KV_CHECKED(intern);

    if (unqlite_kv_delete(intern->db, key, key_len) == UNQLITE_OK) {
        RETURN_TRUE;
    }

    /* fail */
    php_unqlite_kv_error(intern, E_WARNING TSRMLS_CC);

    /* rollback the transaction  */
    if (unqlite_rollback(intern->db) != UNQLITE_OK) {
        php_unqlite_kv_error(intern, E_WARNING TSRMLS_CC);
    }

    RETURN_FALSE;
}

ZEND_UNQLITE_METHOD(Kv, cursor)
{
    php_unqlite_kv_t *intern;
    php_unqlite_kv_cursor_t *cursor;
    long pos = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l",
                              &pos) == FAILURE) {
        return;
    }

    UNQLITE_KV_CHECKED(intern);

    object_init_ex(return_value, php_unqlite_kv_cursor_ce);

    cursor = zend_object_store_get_object(return_value TSRMLS_CC);

    if (!cursor) {
        zend_throw_exception(zend_exception_get_default(TSRMLS_C),
                             "The UnQLite\\KvCursor object has not been "
                             "correctly initialized by its constructor",
                             0 TSRMLS_CC);
        return;
    }

    cursor->link = getThis();
    zval_add_ref(&cursor->link);

    if (unqlite_kv_cursor_init(php_unqlite_kv_get_db(cursor->link TSRMLS_CC),
                               &cursor->cursor) != UNQLITE_OK ){
        zend_throw_exception(zend_exception_get_default(TSRMLS_C),
                             "Unable to init cursor", 0 TSRMLS_CC);
    }

    /* Point to the record */
    if (pos == PHP_UNQLITE_CURSOR_FIRST) {
        unqlite_kv_cursor_first_entry(cursor->cursor);
    } else if (pos == PHP_UNQLITE_CURSOR_LAST) {
        unqlite_kv_cursor_last_entry(cursor->cursor);
    }
}

ZEND_UNQLITE_METHOD(Kv, config)
{
    php_unqlite_kv_t *intern;
    long opt;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
                              &opt) == FAILURE) {
        return;
    }

    UNQLITE_KV_CHECKED(intern);

    if (opt == UNQLITE_CONFIG_DISABLE_AUTO_COMMIT) {
        if (unqlite_config(intern->db, opt) == UNQLITE_OK) {
            RETURN_TRUE;
        }
        php_unqlite_kv_error(intern, E_WARNING TSRMLS_CC);
    } else {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "unknown config option");
    }

    RETURN_FALSE;
}


ZEND_UNQLITE_METHOD(Kv, begin)
{
    php_unqlite_kv_t *intern;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    UNQLITE_KV_CHECKED(intern);

    if (unqlite_begin(intern->db) == UNQLITE_OK) {
        RETURN_TRUE;
    }

    php_unqlite_kv_error(intern, E_ERROR TSRMLS_CC);

    RETURN_FALSE;
}

ZEND_UNQLITE_METHOD(Kv, commit)
{
    php_unqlite_kv_t *intern;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    UNQLITE_KV_CHECKED(intern);

    if (unqlite_commit(intern->db) == UNQLITE_OK) {
        RETURN_TRUE;
    }

    php_unqlite_kv_error(intern, E_ERROR TSRMLS_CC);

    RETURN_FALSE;
}

ZEND_UNQLITE_METHOD(Kv, rollback)
{
    php_unqlite_kv_t *intern;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    UNQLITE_KV_CHECKED(intern);

    if (unqlite_rollback(intern->db) == UNQLITE_OK) {
        RETURN_TRUE;
    }

    php_unqlite_kv_error(intern, E_ERROR TSRMLS_CC);

    RETURN_FALSE;
}

#define UNQLITE_KV_CURSOR(name)                                     \
    name = (php_unqlite_kv_cursor_t *)zend_object_store_get_object( \
        getThis() TSRMLS_CC)
#define php_unqlite_kv_cursor_error(intern, flags)              \
    php_unqlite_kv_error(                                       \
        zend_object_store_get_object((intern)->link TSRMLS_CC), \
        flags TSRMLS_CC);

ZEND_UNQLITE_METHOD(KvCursor, __construct)
{
    int rc;
    php_unqlite_kv_cursor_t *intern;
    zval *link;
    long pos = -1;
#if ZEND_MODULE_API_NO >= 20090626
    zend_error_handling error_handling;
    zend_replace_error_handling(EH_THROW, NULL, &error_handling TSRMLS_CC);
#else
    php_set_error_handling(EH_THROW, NULL TSRMLS_CC);
#endif

    rc = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O|l",
                               &link, php_unqlite_kv_ce, &pos);

#if ZEND_MODULE_API_NO >= 20090626
    zend_restore_error_handling(&error_handling TSRMLS_CC);
#else
    php_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
#endif
    if (rc == FAILURE) {
        return;
    }

    UNQLITE_KV_CURSOR(intern);

    intern->link = link;
    zval_add_ref(&intern->link);

    if (unqlite_kv_cursor_init(php_unqlite_kv_get_db(intern->link TSRMLS_CC),
                               &intern->cursor) != UNQLITE_OK ){
        zend_throw_exception(zend_exception_get_default(TSRMLS_C),
                             "Unable to init cursor", 0 TSRMLS_CC);
    }

    /* Point to the record */
    if (pos == PHP_UNQLITE_CURSOR_FIRST) {
        unqlite_kv_cursor_first_entry(intern->cursor);
    } else if (pos == PHP_UNQLITE_CURSOR_LAST) {
        unqlite_kv_cursor_last_entry(intern->cursor);
    }
}

ZEND_UNQLITE_METHOD(KvCursor, first)
{
    php_unqlite_kv_cursor_t *intern;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    UNQLITE_KV_CURSOR(intern);

    if (unqlite_kv_cursor_first_entry(intern->cursor) == UNQLITE_OK) {
        RETURN_TRUE;
    }

    RETURN_FALSE;
}

ZEND_UNQLITE_METHOD(KvCursor, last)
{
    php_unqlite_kv_cursor_t *intern;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    UNQLITE_KV_CURSOR(intern);

    if (unqlite_kv_cursor_last_entry(intern->cursor) == UNQLITE_OK) {
        RETURN_TRUE;
    }

    RETURN_FALSE;
}

ZEND_UNQLITE_METHOD(KvCursor, next)
{
    php_unqlite_kv_cursor_t *intern;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    UNQLITE_KV_CURSOR(intern);

    if (unqlite_kv_cursor_next_entry(intern->cursor) == UNQLITE_OK) {
        RETURN_TRUE;
    }

    RETURN_FALSE;
}

ZEND_UNQLITE_METHOD(KvCursor, prev)
{
    php_unqlite_kv_cursor_t *intern;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    UNQLITE_KV_CURSOR(intern);

    if (unqlite_kv_cursor_prev_entry(intern->cursor) == UNQLITE_OK) {
        RETURN_TRUE;
    }

    RETURN_FALSE;
}

ZEND_UNQLITE_METHOD(KvCursor, exists)
{
    php_unqlite_kv_cursor_t *intern;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    UNQLITE_KV_CURSOR(intern);

    if (unqlite_kv_cursor_valid_entry(intern->cursor)) {
        RETURN_TRUE;
    }

    RETURN_FALSE;
}

ZEND_UNQLITE_METHOD(KvCursor, delete)
{
    php_unqlite_kv_cursor_t *intern;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    UNQLITE_KV_CURSOR(intern);

    if (unqlite_kv_cursor_delete_entry(intern->cursor) == UNQLITE_OK) {
        RETURN_TRUE;
    }

    RETURN_FALSE;
}

ZEND_UNQLITE_METHOD(KvCursor, seek)
{
    php_unqlite_kv_cursor_t *intern;
    char *key;
    int key_len;
    long opt = UNQLITE_CURSOR_MATCH_EXACT;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l",
                              &key, &key_len, &opt) == FAILURE) {
        return;
    }

    UNQLITE_KV_CURSOR(intern);

    if (unqlite_kv_cursor_seek(intern->cursor, key, key_len,
                               opt) == UNQLITE_OK) {
        RETURN_TRUE;
    }

    RETURN_FALSE;
}

ZEND_UNQLITE_METHOD(KvCursor, key)
{
    php_unqlite_kv_cursor_t *intern;
    char *buf;
    int len;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    UNQLITE_KV_CURSOR(intern);

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
    php_unqlite_kv_cursor_error(intern, E_WARNING);

    if (buf) {
        efree(buf);
    }

    RETURN_FALSE;
}

ZEND_UNQLITE_METHOD(KvCursor, data)
{
    php_unqlite_kv_cursor_t *intern;
    char *buf;
    unqlite_int64 len;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    UNQLITE_KV_CURSOR(intern);

    if (unqlite_kv_cursor_data(intern->cursor, NULL, &len) != UNQLITE_OK) {
        RETURN_FALSE;
    }

    buf = (char *)emalloc(len + 1);
    if (!buf) {
        php_error_out_of_memory();
        RETURN_FALSE;
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
    php_unqlite_kv_cursor_error(intern, E_WARNING);

    if (buf) {
        efree(buf);
    }

    RETURN_FALSE;
}

ZEND_INI_BEGIN()
ZEND_INI_END()

static zend_function_entry php_unqlite_kv_methods[] = {
    ZEND_UNQLITE_ME(Kv, __construct, arginfo_unqlite_kv___construct,
                    ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    ZEND_UNQLITE_ME(Kv, close, arginfo_unqlite_none, ZEND_ACC_PUBLIC)
    ZEND_UNQLITE_ME(Kv, store, arginfo_unqlite_key_val, ZEND_ACC_PUBLIC)
    ZEND_UNQLITE_ME(Kv, append, arginfo_unqlite_key_val, ZEND_ACC_PUBLIC)
    ZEND_UNQLITE_ME(Kv, fetch, arginfo_unqlite_key, ZEND_ACC_PUBLIC)
    ZEND_UNQLITE_ME(Kv, delete, arginfo_unqlite_key, ZEND_ACC_PUBLIC)
    ZEND_UNQLITE_ME(Kv, cursor, arginfo_unqlite_option, ZEND_ACC_PUBLIC)
    ZEND_UNQLITE_ME(Kv, config, arginfo_unqlite_option, ZEND_ACC_PUBLIC)
    ZEND_UNQLITE_ME(Kv, begin, arginfo_unqlite_none, ZEND_ACC_PUBLIC)
    ZEND_UNQLITE_ME(Kv, commit, arginfo_unqlite_none, ZEND_ACC_PUBLIC)
    ZEND_UNQLITE_ME(Kv, rollback, arginfo_unqlite_none, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

static void
php_unqlite_kv_free_storage(void *object TSRMLS_DC)
{
    php_unqlite_kv_t *intern = (php_unqlite_kv_t *)object;

    if (!intern) {
        return;
    }

    if (intern->init && intern->db) {
        unqlite_close(intern->db);
        intern->init = 0;
    }

    zend_object_std_dtor(&intern->std TSRMLS_CC);
    efree(object);
}

static inline zend_object_value
php_unqlite_kv_new_ex(zend_class_entry *ce, php_unqlite_kv_t **ptr TSRMLS_DC)
{
    php_unqlite_kv_t *intern;
    zend_object_value retval;
#if ZEND_MODULE_API_NO < 20100525
    zval *tmp;
#endif

    intern = emalloc(sizeof(php_unqlite_kv_t));
    memset(intern, 0, sizeof(php_unqlite_kv_t));
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
        (zend_objects_free_object_storage_t)php_unqlite_kv_free_storage,
        NULL TSRMLS_CC);
    retval.handlers = &php_unqlite_kv_handlers;

    intern->db = NULL;
    intern->init = 0;

    return retval;
}

static inline zend_object_value
php_unqlite_kv_new(zend_class_entry *ce TSRMLS_DC)
{
    return php_unqlite_kv_new_ex(ce, NULL TSRMLS_CC);
}

static inline zend_object_value
php_unqlite_kv_clone(zval *this_ptr TSRMLS_DC)
{
    php_unqlite_kv_t *new_obj = NULL;
    php_unqlite_kv_t *old_obj =
        (php_unqlite_kv_t *)zend_object_store_get_object(this_ptr TSRMLS_CC);
    zend_object_value new_ov = php_unqlite_kv_new_ex(old_obj->std.ce,
                                                     &new_obj TSRMLS_CC);

    zend_objects_clone_members(&new_obj->std, new_ov, &old_obj->std,
                               Z_OBJ_HANDLE_P(this_ptr) TSRMLS_CC);

    return new_ov;
}

static inline int
php_unqlite_kv_class_register(TSRMLS_D)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, ZEND_NS_NAME("UnQLite", "Kv"), php_unqlite_kv_methods);

    ce.create_object = php_unqlite_kv_new;

    php_unqlite_kv_ce = zend_register_internal_class(&ce TSRMLS_CC);
    if (php_unqlite_kv_ce == NULL) {
        return FAILURE;
    }

    memcpy(&php_unqlite_kv_handlers, zend_get_std_object_handlers(),
           sizeof(zend_object_handlers));

    php_unqlite_kv_handlers.clone_obj = php_unqlite_kv_clone;

    return SUCCESS;
}

static zend_function_entry php_unqlite_kv_cursor_methods[] = {
    ZEND_UNQLITE_ME(KvCursor, __construct,
                    arginfo_unqlite_kv_cursor___construct,
                    ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    ZEND_UNQLITE_ME(KvCursor, first, arginfo_unqlite_none, ZEND_ACC_PUBLIC)
    ZEND_UNQLITE_ME(KvCursor, last, arginfo_unqlite_none, ZEND_ACC_PUBLIC)
    ZEND_UNQLITE_ME(KvCursor, next, arginfo_unqlite_none, ZEND_ACC_PUBLIC)
    ZEND_UNQLITE_ME(KvCursor, prev, arginfo_unqlite_none, ZEND_ACC_PUBLIC)
    ZEND_UNQLITE_ME(KvCursor, seek, arginfo_unqlite_key_option, ZEND_ACC_PUBLIC)
    ZEND_UNQLITE_ME(KvCursor, exists, arginfo_unqlite_none, ZEND_ACC_PUBLIC)
    ZEND_UNQLITE_ME(KvCursor, delete, arginfo_unqlite_none, ZEND_ACC_PUBLIC)
    ZEND_UNQLITE_ME(KvCursor, key, arginfo_unqlite_none, ZEND_ACC_PUBLIC)
    ZEND_UNQLITE_ME(KvCursor, data, arginfo_unqlite_none, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

static void
php_unqlite_kv_cursor_free_storage(void *object TSRMLS_DC)
{
    php_unqlite_kv_cursor_t *intern = (php_unqlite_kv_cursor_t *)object;

    if (!intern) {
        return;
    }

    if (intern->link) {
        if (intern->cursor) {
            unqlite_kv_cursor_release(
                php_unqlite_kv_get_db(intern->link TSRMLS_CC), intern->cursor);
        }
        zval_ptr_dtor(&intern->link);
    }

    zend_object_std_dtor(&intern->std TSRMLS_CC);
    efree(object);
}

static inline zend_object_value
php_unqlite_kv_cursor_new_ex(zend_class_entry *ce,
                             php_unqlite_kv_cursor_t **ptr TSRMLS_DC)
{
    php_unqlite_kv_cursor_t *intern;
    zend_object_value retval;
#if ZEND_MODULE_API_NO < 20100525
    zval *tmp;
#endif

    intern = emalloc(sizeof(php_unqlite_kv_cursor_t));
    memset(intern, 0, sizeof(php_unqlite_kv_cursor_t));
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
        (zend_objects_free_object_storage_t)php_unqlite_kv_cursor_free_storage,
        NULL TSRMLS_CC);
    retval.handlers = &php_unqlite_kv_cursor_handlers;

    intern->link = NULL;
    intern->cursor = NULL;

    return retval;
}

static inline zend_object_value
php_unqlite_kv_cursor_new(zend_class_entry *ce TSRMLS_DC)
{
    return php_unqlite_kv_cursor_new_ex(ce, NULL TSRMLS_CC);
}

static inline zend_object_value
php_unqlite_kv_cursor_clone(zval *this_ptr TSRMLS_DC)
{
    php_unqlite_kv_cursor_t *new_obj = NULL;
    php_unqlite_kv_cursor_t *old_obj =
        (php_unqlite_kv_cursor_t *)zend_object_store_get_object(
            this_ptr TSRMLS_CC);
    zend_object_value new_ov = php_unqlite_kv_cursor_new_ex(old_obj->std.ce,
                                                            &new_obj TSRMLS_CC);

    zend_objects_clone_members(&new_obj->std, new_ov, &old_obj->std,
                               Z_OBJ_HANDLE_P(this_ptr) TSRMLS_CC);

    MAKE_STD_ZVAL(new_obj->link);
    *new_obj->link = *old_obj->link;
    zval_copy_ctor(new_obj->link);

    return new_ov;
}

static inline int
php_unqlite_kv_cursor_class_register(TSRMLS_D)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, ZEND_NS_NAME("UnQLite", "KvCursor"),
                     php_unqlite_kv_cursor_methods);

    ce.create_object = php_unqlite_kv_cursor_new;

    php_unqlite_kv_cursor_ce = zend_register_internal_class(&ce TSRMLS_CC);
    if (php_unqlite_kv_cursor_ce == NULL) {
        return FAILURE;
    }

    memcpy(&php_unqlite_kv_cursor_handlers,
           zend_get_std_object_handlers(), sizeof(zend_object_handlers));

    php_unqlite_kv_cursor_handlers.clone_obj = php_unqlite_kv_cursor_clone;

    return SUCCESS;
}

/*
static void
php_unqlite_init_globals(zend_unqlite_globals *unqlite_globals)
{}
*/

#define UNQLITE_LONG_CONSTANT(name, val) \
    REGISTER_NS_LONG_CONSTANT("UnQLite", name, val, CONST_CS|CONST_PERSISTENT)
#define UNQLITE_STRING_CONSTANT(name, val) \
    REGISTER_NS_STRING_CONSTANT("UnQLite", name, val, CONST_CS|CONST_PERSISTENT)

ZEND_MINIT_FUNCTION(unqlite)
{
    /* ZEND_INIT_MODULE_GLOBALS(unqlite, php_unqlite_init_globals, NULL); */

    php_unqlite_kv_class_register(TSRMLS_C);
    php_unqlite_kv_cursor_class_register(TSRMLS_C);

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

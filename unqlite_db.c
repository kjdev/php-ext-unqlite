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
#include "unqlite_doc.h"
#include "unqlite_exception.h"

zend_class_entry *php_unqlite_db_ce;
static zend_object_handlers php_unqlite_db_handlers;

ZEND_BEGIN_ARG_INFO_EX(arginfo_unqlite_db___construct, 0, 0, 1)
    ZEND_ARG_INFO(0, filename)
    ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_unqlite_db_config, 0, 0, 0)
    ZEND_ARG_INFO(0, option)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_unqlite_db_kvs, 0, 0, 0)
    ZEND_ARG_INFO(0, option)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_unqlite_db_doc, 0, 0, 1)
    ZEND_ARG_INFO(0, collection)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_unqlite_db_none, 0, 0, 0)
ZEND_END_ARG_INFO()

#define php_error_out_of_memory() \
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "unable to allocate memory")

#define UNQLITE_DB(self) \
    self = (php_unqlite_db_t *)zend_object_store_get_object(getThis() TSRMLS_CC)

#define UNQLITE_DB_CHECKED(self)                        \
    UNQLITE_DB(self);                                   \
    if (!((self)->init)) {                              \
        php_error_docref(NULL TSRMLS_CC, E_WARNING,     \
                         "UnQLite\\DB object has not "  \
                         "been correctly initialized"); \
        RETURN_FALSE;                                   \
    }

static void
php_unqlite_db_error(php_unqlite_db_t *intern, int flags TSRMLS_DC)
{
    if (!intern || !intern->init || !intern->db) {
        return;
    }

    php_unqlite_error(intern->db, flags TSRMLS_CC);
}

UNQLITE_ZEND_METHOD(DB, __construct)
{
    int rc;
    php_unqlite_db_t *intern;
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

    UNQLITE_DB(intern);

    if (intern->init) {
        UNQLITE_EXCEPTION("already initialized DB Object");
        return;
    }

    if (strlen(filename) != filename_len) {
        UNQLITE_EXCEPTION("unknown to open database: %s(%d)",
                          filename, filename_len);
        return;
    }

    if (memcmp(filename, ":mem:", sizeof(":mem:")) != 0) {
        if (!(fullpath = expand_filepath(filename, NULL TSRMLS_CC))) {
            UNQLITE_EXCEPTION("unable to expand filepath: %s", filename);
            return;
        }
#if PHP_API_VERSION < 20100412
        if (PG(safe_mode) &&
            (!php_checkuid(fullpath, NULL, CHECKUID_CHECK_FILE_AND_DIR))) {
            UNQLITE_EXCEPTION("safe_mode prohibits opening %s", fullpath);
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
        UNQLITE_EXCEPTION("unable to open database: %s", fullpath);
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

UNQLITE_ZEND_METHOD(DB, close)
{
    php_unqlite_db_t *intern;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    UNQLITE_DB(intern);

    if (intern->init) {
        if (unqlite_close(intern->db) != UNQLITE_OK) {
            php_unqlite_db_error(intern, E_ERROR TSRMLS_CC);
            RETURN_FALSE;
        }
        intern->init = 0;
    }

    RETURN_TRUE;
}

UNQLITE_ZEND_METHOD(DB, config)
{
    php_unqlite_db_t *intern;
    long opt;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
                              &opt) == FAILURE) {
        return;
    }

    UNQLITE_DB_CHECKED(intern);

    if (opt == UNQLITE_CONFIG_DISABLE_AUTO_COMMIT) {
        if (unqlite_config(intern->db, opt) == UNQLITE_OK) {
            RETURN_TRUE;
        }
        php_unqlite_db_error(intern, E_WARNING TSRMLS_CC);
    } else {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "unknown config option");
    }

    RETURN_FALSE;
}

UNQLITE_ZEND_METHOD(DB, kvs)
{
    php_unqlite_db_t *intern;
    php_unqlite_kvs_t *kvs;
    long pos = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                              "|l", &pos) == FAILURE) {
        return;
    }

    UNQLITE_DB_CHECKED(intern);

    object_init_ex(return_value, php_unqlite_kvs_ce);

    kvs = zend_object_store_get_object(return_value TSRMLS_CC);

    if (!kvs) {
        UNQLITE_EXCEPTION("The UnQLite\\Kvs object has not been "
                          "correctly initialized by its constructor");
        return;
    }

    kvs->link = getThis();
    zval_add_ref(&kvs->link);

    if (php_unqlite_kvs_class_init(kvs TSRMLS_CC) != SUCCESS) {
        UNQLITE_EXCEPTION("unable to init UnQLite\\Kvs");
    }
}

UNQLITE_ZEND_METHOD(DB, doc)
{
    php_unqlite_db_t *intern;
    php_unqlite_doc_t *doc;
    char *name = NULL;
    int name_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                              "s", &name, &name_len) == FAILURE) {
        return;
    }

    if (!name || name_len <= 0) {
        UNQLITE_EXCEPTION("expects parameter 1 to be an collection name");
        return;
    }

    UNQLITE_DB_CHECKED(intern);

    object_init_ex(return_value, php_unqlite_doc_ce);

    doc = zend_object_store_get_object(return_value TSRMLS_CC);

    if (!doc) {
        UNQLITE_EXCEPTION("The UnQLite\\Doc object has not been "
                          "correctly initialized by its constructor");
        return;
    }

    doc->link = getThis();
    zval_add_ref(&doc->link);

    MAKE_STD_ZVAL(doc->name);
    ZVAL_STRINGL(doc->name, name, name_len, 1);

    if (php_unqlite_doc_class_init(doc TSRMLS_CC) != SUCCESS) {
        UNQLITE_EXCEPTION("unable to init UnQLite\\Doc");
        return;
    }
}

static zend_function_entry php_unqlite_db_methods[] = {
    UNQLITE_ZEND_ME(DB, __construct, arginfo_unqlite_db___construct,
                    ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    UNQLITE_ZEND_ME(DB, close, arginfo_unqlite_db_none, ZEND_ACC_PUBLIC)
    UNQLITE_ZEND_ME(DB, config, arginfo_unqlite_db_config, ZEND_ACC_PUBLIC)
    UNQLITE_ZEND_ME(DB, kvs, arginfo_unqlite_db_kvs, ZEND_ACC_PUBLIC)
    UNQLITE_ZEND_ME(DB, doc, arginfo_unqlite_db_doc, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

static void
php_unqlite_db_free_storage(void *object TSRMLS_DC)
{
    php_unqlite_db_t *intern = (php_unqlite_db_t *)object;

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
php_unqlite_db_new_ex(zend_class_entry *ce, php_unqlite_db_t **ptr TSRMLS_DC)
{
    php_unqlite_db_t *intern;
    zend_object_value retval;
#if ZEND_MODULE_API_NO < 20100525
    zval *tmp;
#endif

    intern = emalloc(sizeof(php_unqlite_db_t));
    memset(intern, 0, sizeof(php_unqlite_db_t));
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
        (zend_objects_free_object_storage_t)php_unqlite_db_free_storage,
        NULL TSRMLS_CC);
    retval.handlers = &php_unqlite_db_handlers;

    intern->db = NULL;
    intern->init = 0;

    return retval;
}

static inline zend_object_value
php_unqlite_db_new(zend_class_entry *ce TSRMLS_DC)
{
    return php_unqlite_db_new_ex(ce, NULL TSRMLS_CC);
}

PHP_UNQLITE_API int
php_unqlite_db_class_register(TSRMLS_D)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, ZEND_NS_NAME(UNQLITE_NS, "DB"), php_unqlite_db_methods);

    ce.create_object = php_unqlite_db_new;

    php_unqlite_db_ce = zend_register_internal_class(&ce TSRMLS_CC);
    if (php_unqlite_db_ce == NULL) {
        return FAILURE;
    }

    memcpy(&php_unqlite_db_handlers, zend_get_std_object_handlers(),
           sizeof(zend_object_handlers));

    php_unqlite_db_handlers.clone_obj = NULL;

    return SUCCESS;
}

PHP_UNQLITE_API void
php_unqlite_error(unqlite *db, int flags TSRMLS_DC)
{
    const char *msg = NULL;
    int len = 0;

    unqlite_config(db, UNQLITE_CONFIG_ERR_LOG, &msg, &len);
    if (msg) {
        php_error_docref(NULL TSRMLS_CC, flags, msg);
    } else {
        php_error_docref(NULL TSRMLS_CC, flags, "unknown error");
    }
}

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
#include "unqlite_json.h"
#include "unqlite_doc.h"
#include "unqlite_exception.h"

ZEND_EXTERN_MODULE_GLOBALS(unqlite)

zend_class_entry *php_unqlite_doc_ce;
static zend_object_handlers php_unqlite_doc_handlers;

ZEND_BEGIN_ARG_INFO_EX(arginfo_unqlite_doc___construct, 0, 0, 2)
    ZEND_ARG_INFO(0, doc)
    ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_unqlite_doc_eval, 0, 0, 1)
    ZEND_ARG_INFO(0, code)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_unqlite_doc_store, 0, 0, 1)
    ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_unqlite_doc_fetch, 0, 0, 0)
    ZEND_ARG_INFO(0, offset)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_unqlite_doc_fetch_all, 0, 0, 0)
    ZEND_ARG_INFO(0, option)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_unqlite_doc_fetch_id, 0, 0, 1)
    ZEND_ARG_INFO(0, id)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_unqlite_doc_remove, 0, 0, 1)
    ZEND_ARG_INFO(0, id)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_unqlite_doc_none, 0, 0, 0)
ZEND_END_ARG_INFO()

#define JX9_CREATE                      \
    "if(db_exists($argv[0])){print 1;}" \
    "print intval(db_create($argv[0]));"

#define JX9_DROP                          \
    "if(!db_exists($argv[0])){print 1;} " \
    "print intval(db_drop_collection($argv[0]));"

#define JX9_FETCH                                \
    "$i=0;$n=intval($argv[1]);"                  \
    "while(($record=db_fetch($argv[0]))!=NULL){" \
    "if($n==$i++){print $record;return;}"        \
    "}"

//#define JX9_FETCH     "print db_fetch($argv[0]); "
#define JX9_FETCH_ALL "print db_fetch_all($argv[0]);"
#define JX9_FETCH_ID  "print db_fetch_by_id($argv[0],intval($argv[1]));"
#define JX9_STORE     "print intval(db_store($argv[0],%s));"
#define JX9_COUNT     "print intval(db_total_records($argv[0]));"
#define JX9_REMOVE    "print intval(db_drop_record($argv[0],intval($argv[1])));"
#define JX9_BEGIN     "print intval(db_begin());"
#define JX9_COMMIT    "print intval(db_commit());"
#define JX9_ROLLBACK  "print intval(db_rollback());"
#define JX9_ERROR     "print db_errlog();"

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

#define UNQLITE_DOC_DB(self) php_unqlite_get_db((self)->link TSRMLS_CC)

#define UNQLITE_DOC(self)                                     \
    self = (php_unqlite_doc_t *)zend_object_store_get_object( \
        getThis() TSRMLS_CC)

/*
static void
php_unqlite_doc_error(php_unqlite_doc_t *intern, int flags TSRMLS_DC)
{
    if (!intern || !intern->link) {
        return;
    }

    php_unqlite_error(UNQLITE_DOC_DB(intern), flags TSRMLS_CC);
}
*/

static void
php_unqlite_doc_jx9_error(php_unqlite_doc_t *intern, int flags TSRMLS_DC)
{
    const char *msg = NULL;
    int len = 0;

    if (!intern || !intern->link) {
        return;
    }

    unqlite_config(UNQLITE_DOC_DB(intern), UNQLITE_CONFIG_JX9_ERR_LOG,
                   &msg, &len);
    if (msg && len > 0) {
        php_error_docref(NULL TSRMLS_CC, flags, msg);
    } else {
        php_unqlite_error(UNQLITE_DOC_DB(intern), flags TSRMLS_CC);
    }
}

#define UNQLITE_RETURN_JX9_RESET()                                  \
    if (intern->vm) {                                               \
        if (unqlite_vm_reset(intern->vm) != UNQLITE_OK) {           \
            php_unqlite_doc_jx9_error(intern, E_WARNING TSRMLS_CC); \
            RETURN_FALSE;                                           \
        }                                                           \
    }

#define UNQLITE_RETURN_JX9_COMPILE(script, script_len)              \
    UNQLITE_RETURN_JX9_RESET();                                     \
    if (unqlite_compile(UNQLITE_DOC_DB(intern), script, script_len, \
                        &(intern->vm)) != UNQLITE_OK) {             \
        php_unqlite_doc_jx9_error(intern, E_WARNING TSRMLS_CC);     \
        RETURN_FALSE;                                               \
    }

#define UNQLITE_RETURN_JX9_ARGS(val)                                   \
    if (unqlite_vm_config(intern->vm, UNQLITE_VM_CONFIG_ARGV_ENTRY, \
                          val) != UNQLITE_OK) {                     \
        php_unqlite_doc_jx9_error(intern, E_WARNING TSRMLS_CC);     \
        RETURN_FALSE;                                               \
    }

#define UNQLITE_RETURN_JX9_EXEC_SUCCESS(retval, debug)                  \
    if (unqlite_vm_exec(intern->vm) == UNQLITE_OK) {                    \
        const void *out = NULL;                                         \
        unsigned int len = 0;                                           \
        unqlite_vm_config(intern->vm, UNQLITE_VM_CONFIG_EXTRACT_OUTPUT, \
                          &out, &len);                                  \
        if (len > 0 && ((char *)out)[0] == retval) {                    \
            RETURN_TRUE;                                                \
        } else if (debug) {                                             \
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "%.*s",         \
                             (int)len, (char *)out);                    \
        }                                                               \
    } else {                                                            \
        php_unqlite_doc_jx9_error(intern, E_WARNING TSRMLS_CC);         \
    }

#define UNQLITE_RETURN_JX9_EXEC_ZVAL(warn)                              \
    if (unqlite_vm_exec(intern->vm) == UNQLITE_OK) {                    \
        const void *out = NULL;                                         \
        unsigned int len = 0;                                           \
        unqlite_vm_config(intern->vm, UNQLITE_VM_CONFIG_EXTRACT_OUTPUT, \
                          &out, &len);                                  \
        if (len > 0 && out) {                                           \
            php_unqlite_json_to_zval(return_value,                      \
                                     (char *)out, (int)len TSRMLS_CC);  \
        } else {                                                        \
            if (warn) {                                                 \
                php_error_docref(NULL TSRMLS_CC, E_WARNING,             \
                                 "record not found");                   \
            }                                                           \
            RETVAL_NULL();                                              \
        }                                                               \
    } else {                                                            \
        php_unqlite_doc_jx9_error(intern, E_WARNING TSRMLS_CC);         \
        RETVAL_NULL();                                                  \
    }


UNQLITE_ZEND_METHOD(Doc, __construct)
{
    int rc;
    php_unqlite_doc_t *intern;
    zval *link;
    char *name = NULL;
    int name_len;
#if ZEND_MODULE_API_NO >= 20090626
    zend_error_handling error_handling;
    zend_replace_error_handling(EH_THROW, NULL, &error_handling TSRMLS_CC);
#else
    php_set_error_handling(EH_THROW, NULL TSRMLS_CC);
#endif

    rc = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Os",
                               &link, php_unqlite_db_ce, &name, &name_len);

#if ZEND_MODULE_API_NO >= 20090626
    zend_restore_error_handling(&error_handling TSRMLS_CC);
#else
    php_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
#endif
    if (rc == FAILURE) {
        return;
    }

    UNQLITE_DOC(intern);

    intern->link = link;
    zval_add_ref(&intern->link);

    MAKE_STD_ZVAL(intern->name);
    ZVAL_STRINGL(intern->name, name, name_len, 1);

    if (php_unqlite_doc_class_init(intern TSRMLS_CC) != SUCCESS) {
        UNQLITE_EXCEPTION("unable to init UnQLite\\Doc");
        return;
    }
}

UNQLITE_ZEND_METHOD(Doc, eval)
{
    php_unqlite_doc_t *intern;
    char *script;
    int script_len;
    unqlite_vm *vm;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
                              &script, &script_len) == FAILURE) {
        return;
    }

    if (!script || script_len <= 0) {
        RETURN_FALSE;
    }

    UNQLITE_DOC(intern);

    RETVAL_FALSE;

    if (unqlite_compile(UNQLITE_DOC_DB(intern),
                        script, script_len, &vm) != UNQLITE_OK) {
        php_unqlite_doc_jx9_error(intern, E_WARNING TSRMLS_CC);
        return;
    }

    if (unqlite_vm_exec(vm) == UNQLITE_OK) {
        const void *out = NULL;
        unsigned int len = 0;
        unqlite_vm_config(vm, UNQLITE_VM_CONFIG_EXTRACT_OUTPUT,
                          &out, &len);
        if (out && len > 0) {
            RETVAL_STRINGL((char *)out, len, 1);
        }
    } else {
        php_unqlite_doc_jx9_error(intern, E_WARNING TSRMLS_CC);
    }

    unqlite_vm_release(vm);

    return;
}

UNQLITE_ZEND_METHOD(Doc, drop)
{
    php_unqlite_doc_t *intern;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    UNQLITE_DOC(intern);

    UNQLITE_RETURN_JX9_COMPILE(JX9_DROP, sizeof(JX9_DROP)-1);
    UNQLITE_RETURN_JX9_ARGS(Z_STRVAL_P(intern->name));
    UNQLITE_RETURN_JX9_EXEC_SUCCESS('1', 0);

    RETURN_FALSE;
}

UNQLITE_ZEND_METHOD(Doc, count)
{
    php_unqlite_doc_t *intern;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    UNQLITE_DOC(intern);

    UNQLITE_RETURN_JX9_COMPILE(JX9_COUNT, sizeof(JX9_COUNT)-1);
    UNQLITE_RETURN_JX9_ARGS(Z_STRVAL_P(intern->name));


    if (unqlite_vm_exec(intern->vm) == UNQLITE_OK) {
        const void *out = NULL;
        unsigned int len = 0;
        unqlite_vm_config(intern->vm, UNQLITE_VM_CONFIG_EXTRACT_OUTPUT,
                          &out, &len);
        if (len > 0 && out) {
            RETVAL_STRINGL((char *)out, (int)len, 1);
            convert_to_long(return_value);
        } else {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "record not found");
            RETVAL_LONG(0);
        }
    } else {
        php_unqlite_doc_jx9_error(intern, E_WARNING TSRMLS_CC);
        RETVAL_LONG(0);
    }
}

UNQLITE_ZEND_METHOD(Doc, store)
{
    php_unqlite_doc_t *intern;
    zval *data = NULL;
    size_t len, size;
    char *buf = NULL;
    char *json = NULL;
    size_t json_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z",
                              &data) == FAILURE) {
        return;
    }

    if (!(Z_TYPE_P(data) == IS_ARRAY || Z_TYPE_P(data) == IS_OBJECT)) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING,
                         "expects parameter %d to be an array or object,"
                         " %s given", 1, zend_get_type_by_const(Z_TYPE_P(data)));
        RETURN_FALSE;
    }

    UNQLITE_DOC(intern);

    UNQLITE_RETURN_JX9_RESET();

    json = php_unqlite_to_json(data TSRMLS_CC);
    if (!json) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "store data not found");
        RETURN_FALSE;
    }
    json_len = strlen(json);

    size = sizeof(JX9_STORE) + json_len;
    buf = (char *)emalloc(size);
    if (!buf) {
        php_error_out_of_memory();
        free(json);
        RETURN_FALSE;
    }

    len = snprintf(buf, size - 1, JX9_STORE, json);
    free(json);

    if (unqlite_compile(UNQLITE_DOC_DB(intern), buf, len,
                        &(intern->vm)) != UNQLITE_OK) {
        php_unqlite_doc_jx9_error(intern, E_WARNING TSRMLS_CC);
        efree(buf);
        RETURN_FALSE;
    }
    efree(buf);

    UNQLITE_RETURN_JX9_ARGS(Z_STRVAL_P(intern->name));
    UNQLITE_RETURN_JX9_EXEC_SUCCESS('1', 0);

    RETURN_FALSE;
}

UNQLITE_ZEND_METHOD(Doc, fetch)
{
    php_unqlite_doc_t *intern;
    zval *offset = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|z",
                              &offset) == FAILURE) {
        return;
    }

    UNQLITE_DOC(intern);

    UNQLITE_RETURN_JX9_COMPILE(JX9_FETCH, sizeof(JX9_FETCH)-1);
    UNQLITE_RETURN_JX9_ARGS(Z_STRVAL_P(intern->name));

    if (offset) {
        convert_to_string(offset);
        if (is_numeric_string(Z_STRVAL_P(offset), Z_STRLEN_P(offset),
                              NULL, NULL, -1) != IS_LONG) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                             "expects parameter %d to be an numeric: %s",
                             1, Z_STRVAL_P(offset));
            RETURN_NULL();
        }
        UNQLITE_RETURN_JX9_ARGS(Z_STRVAL_P(offset));
    }

    UNQLITE_RETURN_JX9_EXEC_ZVAL(0);
}

UNQLITE_ZEND_METHOD(Doc, fetch_all)
{
    php_unqlite_doc_t *intern;
    zval *option = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|z",
                              &option) == FAILURE) {
        return;
    }

    UNQLITE_DOC(intern);

    UNQLITE_RETURN_JX9_COMPILE(JX9_FETCH_ALL, sizeof(JX9_FETCH_ALL)-1);
    UNQLITE_RETURN_JX9_ARGS(Z_STRVAL_P(intern->name));

    /*
    if (option) {
        convert_to_string(option);
        UNQLITE_RETURN_JX9_ARGS(Z_STRVAL_P(option));
    }
    */

    UNQLITE_RETURN_JX9_EXEC_ZVAL(0);
}

UNQLITE_ZEND_METHOD(Doc, fetch_id)
{
    php_unqlite_doc_t *intern;
    zval *id = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z",
                              &id) == FAILURE) {
        return;
    }

    convert_to_string(id);

    if (is_numeric_string(Z_STRVAL_P(id), Z_STRLEN_P(id),
                          NULL, NULL, -1) != IS_LONG) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING,
                         "expects parameter %d to be an numeric: %s",
                         1, Z_STRVAL_P(id));
        RETURN_NULL();
    }

    UNQLITE_DOC(intern);

    UNQLITE_RETURN_JX9_COMPILE(JX9_FETCH_ID, sizeof(JX9_FETCH_ID)-1);

    UNQLITE_RETURN_JX9_ARGS(Z_STRVAL_P(intern->name));
    UNQLITE_RETURN_JX9_ARGS(Z_STRVAL_P(id));

    UNQLITE_RETURN_JX9_EXEC_ZVAL(0);
}

UNQLITE_ZEND_METHOD(Doc, remove)
{
    php_unqlite_doc_t *intern;
    zval *id = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z",
                              &id) == FAILURE) {
        return;
    }

    convert_to_string(id);

    if (is_numeric_string(Z_STRVAL_P(id), Z_STRLEN_P(id),
                          NULL, NULL, -1) != IS_LONG) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING,
                         "expects parameter %d to be an numeric: %s",
                         1, Z_STRVAL_P(id));
        RETURN_FALSE;
    }

    UNQLITE_DOC(intern);

    UNQLITE_RETURN_JX9_COMPILE(JX9_REMOVE, sizeof(JX9_REMOVE)-1);

    UNQLITE_RETURN_JX9_ARGS(Z_STRVAL_P(intern->name));
    UNQLITE_RETURN_JX9_ARGS(Z_STRVAL_P(id));

    UNQLITE_RETURN_JX9_EXEC_SUCCESS('1', 0);

    RETURN_FALSE;
}

UNQLITE_ZEND_METHOD(Doc, begin)
{
    php_unqlite_doc_t *intern;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    UNQLITE_DOC(intern);
    UNQLITE_RETURN_JX9_COMPILE(JX9_BEGIN, sizeof(JX9_BEGIN)-1);
    UNQLITE_RETURN_JX9_EXEC_SUCCESS('1', 0);

    RETURN_FALSE;
}

UNQLITE_ZEND_METHOD(Doc, commit)
{
    php_unqlite_doc_t *intern;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    UNQLITE_DOC(intern);
    UNQLITE_RETURN_JX9_COMPILE(JX9_COMMIT, sizeof(JX9_COMMIT)-1);
    UNQLITE_RETURN_JX9_EXEC_SUCCESS('1', 0);

    RETURN_FALSE;
}

UNQLITE_ZEND_METHOD(Doc, rollback)
{
    php_unqlite_doc_t *intern;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    UNQLITE_DOC(intern);
    UNQLITE_RETURN_JX9_COMPILE(JX9_ROLLBACK, sizeof(JX9_ROLLBACK)-1);
    UNQLITE_RETURN_JX9_EXEC_SUCCESS('1', 0);

    RETURN_FALSE;
}

/*
UNQLITE_ZEND_METHOD(Doc, error)
{
    php_unqlite_doc_t *intern;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    UNQLITE_DOC(intern);

    UNQLITE_RETURN_JX9_COMPILE(JX9_ERROR, sizeof(JX9_ERROR)-1);

    if (unqlite_vm_exec(intern->vm) == UNQLITE_OK) {
        const void *out = NULL;
        unsigned int len = 0;
        unqlite_vm_config(intern->vm, UNQLITE_VM_CONFIG_EXTRACT_OUTPUT,
                          &out, &len);
        if (len > 0 && out) {
            RETURN_STRINGL((char *)out, len, 1);
        }
    } else {
        php_unqlite_doc_jx9_error(intern, E_WARNING TSRMLS_CC);
    }

    RETURN_EMPTY_STRING();
}
*/

static zend_function_entry php_unqlite_doc_methods[] = {
    UNQLITE_ZEND_ME(Doc, __construct, arginfo_unqlite_doc___construct,
                    ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    UNQLITE_ZEND_ME(Doc, eval, arginfo_unqlite_doc_eval, ZEND_ACC_PUBLIC)
    UNQLITE_ZEND_ME(Doc, drop, arginfo_unqlite_doc_none, ZEND_ACC_PUBLIC)
    UNQLITE_ZEND_ME(Doc, count, arginfo_unqlite_doc_none, ZEND_ACC_PUBLIC)
    UNQLITE_ZEND_ME(Doc, store, arginfo_unqlite_doc_store, ZEND_ACC_PUBLIC)
    UNQLITE_ZEND_ME(Doc, fetch, arginfo_unqlite_doc_fetch, ZEND_ACC_PUBLIC)
    UNQLITE_ZEND_ME(Doc, fetch_all,
                    arginfo_unqlite_doc_fetch_all, ZEND_ACC_PUBLIC)
    UNQLITE_ZEND_ME(Doc, fetch_id,
                    arginfo_unqlite_doc_fetch_id, ZEND_ACC_PUBLIC)
    UNQLITE_ZEND_ME(Doc, remove, arginfo_unqlite_doc_remove, ZEND_ACC_PUBLIC)
    UNQLITE_ZEND_ME(Doc, begin, arginfo_unqlite_doc_none, ZEND_ACC_PUBLIC)
    UNQLITE_ZEND_ME(Doc, commit, arginfo_unqlite_doc_none, ZEND_ACC_PUBLIC)
    UNQLITE_ZEND_ME(Doc, rollback, arginfo_unqlite_doc_none, ZEND_ACC_PUBLIC)
    /*UNQLITE_ZEND_ME(Doc, error, arginfo_unqlite_doc_none, ZEND_ACC_PUBLIC)*/
    UNQLITE_ZEND_MALIAS(Doc, fetchAll, fetch_all,
                        arginfo_unqlite_doc_fetch_all, ZEND_ACC_PUBLIC)
    UNQLITE_ZEND_MALIAS(Doc, fetchId, fetch_id,
                        arginfo_unqlite_doc_fetch_id, ZEND_ACC_PUBLIC)
    UNQLITE_ZEND_MALIAS(Doc, delete, remove,
                        arginfo_unqlite_doc_remove, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

static void
php_unqlite_doc_free_storage(void *object TSRMLS_DC)
{
    php_unqlite_doc_t *intern = (php_unqlite_doc_t *)object;

    if (!intern) {
        return;
    }

    if (intern->vm) {
        unqlite_vm_release(intern->vm);
    }

    if (intern->name) {
        zval_ptr_dtor(&intern->name);
    }

    if (intern->link) {
        zval_ptr_dtor(&intern->link);
    }

    zend_object_std_dtor(&intern->std TSRMLS_CC);
    efree(object);
}

static inline zend_object_value
php_unqlite_doc_new_ex(zend_class_entry *ce, php_unqlite_doc_t **ptr TSRMLS_DC)
{
    php_unqlite_doc_t *intern;
    zend_object_value retval;
#if ZEND_MODULE_API_NO < 20100525
    zval *tmp;
#endif

    intern = emalloc(sizeof(php_unqlite_doc_t));
    memset(intern, 0, sizeof(php_unqlite_doc_t));
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
        (zend_objects_free_object_storage_t)php_unqlite_doc_free_storage,
        NULL TSRMLS_CC);
    retval.handlers = &php_unqlite_doc_handlers;

    intern->link = NULL;
    intern->vm = NULL;
    intern->name = NULL;

    return retval;
}

static inline zend_object_value
php_unqlite_doc_new(zend_class_entry *ce TSRMLS_DC)
{
    return php_unqlite_doc_new_ex(ce, NULL TSRMLS_CC);
}

PHP_UNQLITE_API int
php_unqlite_doc_class_init(php_unqlite_doc_t *intern TSRMLS_DC)
{
    unqlite_vm *vm;

    if (unqlite_compile(UNQLITE_DOC_DB(intern),
                        JX9_CREATE, sizeof(JX9_CREATE)-1, &vm) != UNQLITE_OK) {
        php_unqlite_doc_jx9_error(intern, E_WARNING TSRMLS_CC);
        return FAILURE;
    }

    if (unqlite_vm_config(vm, UNQLITE_VM_CONFIG_ARGV_ENTRY,
                          Z_STRVAL_P(intern->name)) != UNQLITE_OK) {
        php_unqlite_doc_jx9_error(intern, E_WARNING TSRMLS_CC);
        unqlite_vm_release(vm);
        return FAILURE;
    }

    if (unqlite_vm_exec(vm) == UNQLITE_OK) {
        const void *out = NULL;
        unsigned int len = 0;
        unqlite_vm_config(vm, UNQLITE_VM_CONFIG_EXTRACT_OUTPUT, &out, &len);
        if (len > 0 && ((char *)out)[0] == '1') {
            unqlite_vm_release(vm);
            return SUCCESS;
        }
    } else {
        php_unqlite_doc_jx9_error(intern, E_WARNING TSRMLS_CC);
    }

    unqlite_vm_release(vm);

    return FAILURE;
}

PHP_UNQLITE_API int
php_unqlite_doc_class_register(TSRMLS_D)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, ZEND_NS_NAME(UNQLITE_NS, "Doc"),
                     php_unqlite_doc_methods);

    ce.create_object = php_unqlite_doc_new;

    php_unqlite_doc_ce = zend_register_internal_class(&ce TSRMLS_CC);
    if (php_unqlite_doc_ce == NULL) {
        return FAILURE;
    }

    memcpy(&php_unqlite_doc_handlers, zend_get_std_object_handlers(),
           sizeof(zend_object_handlers));

    php_unqlite_doc_handlers.clone_obj = NULL;

    return SUCCESS;
}

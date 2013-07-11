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

#include "unqlite_json.h"

#include "jansson/jansson.h"

ZEND_EXTERN_MODULE_GLOBALS(unqlite)

static inline int php_unqlite_to_json_object(zval **struc,
                                             json_t *json TSRMLS_DC);

static inline int
php_unqlite_is_array(zval *val TSRMLS_DC)
{
    ulong index = 0;
    HashTable *myht;
    Bucket *p;

    myht = Z_ARRVAL_P(val);
    p = myht->pListHead;
    while (p) {
        if (p->nKeyLength || index != p->h) {
            return 0;
        }
        p = p->pListNext;
        index++;
    }

    return 1;
}

static inline void
php_unqlite_json_array_append(json_t *obj, zval *val TSRMLS_DC)
{
    json_t *element;

    switch (Z_TYPE_P(val)) {
        case IS_BOOL:
            json_array_append_new(obj, json_boolean(Z_LVAL_P(val)));
            break;
        case IS_NULL:
            json_array_append_new(obj, json_null());
            break;
        case IS_LONG:
            json_array_append_new(obj, json_integer(Z_LVAL_P(val)));
            break;
        case IS_DOUBLE: {
            double dbl = Z_DVAL_P(val);
            if (!zend_isinf(dbl) && !zend_isnan(dbl)) {
                json_array_append_new(obj, json_real(dbl));
            } else {
                php_error_docref(NULL TSRMLS_CC, E_WARNING,
                                 "Inf and Nan cannot be JSON encoded");
                json_array_append_new(obj, json_integer(0));
            }
            break;
        }
        case IS_STRING:
            json_array_append_new(obj, json_string(Z_STRVAL_P(val)));
            break;
        case IS_ARRAY:
            if (php_unqlite_is_array(val TSRMLS_CC)) {
                element = json_array();
            } else {
                element = json_object();
            }
            if (!element) {
                return;
            }
            if (php_unqlite_to_json_object(&val, element TSRMLS_CC) == 0) {
                json_array_append_new(obj, element);
            } else {
                json_array_append_new(obj, json_null());
                json_delete(element);
            }
            break;
        case IS_OBJECT:
            element = json_object();
            if (!element) {
                return;
            }
            if (php_unqlite_to_json_object(&val, element TSRMLS_CC) == 0) {
                json_array_append_new(obj, element);
            } else {
                json_array_append_new(obj, json_null());
                json_delete(element);
            }
            break;
        default:
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "type is not supported");
            json_array_append_new(obj, json_null());
            break;
    }
}

static inline void
php_unqlite_json_object_append(json_t *obj, char *key, zval *val TSRMLS_DC)
{
    json_t *element;

    switch (Z_TYPE_P(val)) {
        case IS_BOOL:
            json_object_set_new(obj, key, json_boolean(Z_LVAL_P(val)));
            break;
        case IS_NULL:
            json_object_set_new(obj, key, json_null());
            break;
        case IS_LONG:
            json_object_set_new(obj, key, json_integer(Z_LVAL_P(val)));
            break;
        case IS_DOUBLE: {
            double dbl = Z_DVAL_P(val);
            if (!zend_isinf(dbl) && !zend_isnan(dbl)) {
                json_object_set_new(obj, key, json_real(dbl));
            } else {
                php_error_docref(NULL TSRMLS_CC, E_WARNING,
                                 "Inf and Nan cannot be JSON encoded");
                json_object_set_new(obj, key, json_integer(0));
            }
            break;
        }
        case IS_STRING:
            json_object_set_new(obj, key, json_string(Z_STRVAL_P(val)));
            break;
        case IS_ARRAY:
            if (php_unqlite_is_array(val TSRMLS_CC)) {
                element = json_array();
            } else {
                element = json_object();
            }
            if (!element) {
                return;
            }
            if (php_unqlite_to_json_object(&val, element TSRMLS_CC) == 0) {
                json_object_set_new(obj, key, element);
            } else {
                json_object_set_new(obj, key, json_null());
                json_delete(element);
            }
            break;
        case IS_OBJECT:
            element = json_object();
            if (!element) {
                return;
            }
            if (php_unqlite_to_json_object(&val, element TSRMLS_CC) == 0) {
                json_object_set_new(obj, key, element);
            } else {
                json_object_set_new(obj, key, json_null());
                json_delete(element);
            }
            break;
        default:
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "type is not supported");
            json_object_set_new(obj, key, json_null());
            break;
    }
}

static inline int
php_unqlite_to_json_object(zval **struc, json_t *json TSRMLS_DC)
{
    HashTable *ht;
    int n = 0;

    if (Z_TYPE_PP(struc) == IS_ARRAY) {
        ht = Z_ARRVAL_PP(struc);
    } else {
        ht = Z_OBJPROP_PP(struc);
    }

    if (ht && ht->nApplyCount > 1) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "recursion detected");
        return -1;
    }

    if (ht) {
        n = zend_hash_num_elements(ht);
    }

    if (n > 0) {
        zval **data;
        char *str_key;
        uint str_key_len;
        ulong num_key;
        HashPosition pos;
        HashTable *tmp_ht;

        zend_hash_internal_pointer_reset_ex(ht, &pos);
        for (;; zend_hash_move_forward_ex(ht, &pos)) {
            n = zend_hash_get_current_key_ex(ht, &str_key, &str_key_len,
                                             &num_key, 0, &pos);
            if (n == HASH_KEY_NON_EXISTANT) {
                break;
            }

            if (zend_hash_get_current_data_ex(ht, (void **)&data,
                                              &pos) == SUCCESS) {
                tmp_ht = HASH_OF(*data);
                if (tmp_ht) {
                    tmp_ht->nApplyCount++;
                }

                if (json_is_array(json)) {
                    php_unqlite_json_array_append(json, *data TSRMLS_CC);
                } else {
                    if (n == HASH_KEY_IS_STRING) {
                        if (str_key[0] == '\0' &&
                            Z_TYPE_PP(struc) == IS_OBJECT) {
                            if (tmp_ht) {
                                tmp_ht->nApplyCount--;
                            }
                            continue;
                        }
                        php_unqlite_json_object_append(json, str_key,
                                                       *data TSRMLS_CC);
                    } else {
                        char *tmp_str;
                        spprintf(&tmp_str, 0, "%ld", (long)num_key);
                        php_unqlite_json_object_append(json, tmp_str,
                                                       *data TSRMLS_CC);
                        efree(tmp_str);
                    }
                }

                if (tmp_ht) {
                    tmp_ht->nApplyCount--;
                }
            }
        }
    }

    return 0;
}

static void
php_unqlite_json_object_to_zval(json_t *obj, zval *return_value TSRMLS_DC)
{
    switch (json_typeof(obj)) {
        case JSON_NULL:
            RETVAL_NULL();
            break;
        case JSON_TRUE:
            RETVAL_BOOL(1);
            break;
        case JSON_FALSE:
            RETVAL_BOOL(0);
            break;
        case JSON_REAL: {
            double d;
            json_unpack(obj, "f", &d);
            RETVAL_DOUBLE(d);
            break;
        }
        case JSON_INTEGER: {
            int i;
            json_unpack(obj, "i", &i);
            RETVAL_LONG((long)i);
            break;
        }
        case JSON_STRING: {
            const char *str;
            json_unpack(obj, "s", &str);
            RETVAL_STRING(str, 1);
            break;
        }
        case JSON_ARRAY: {
            size_t i, size;
            zval *retval;

            array_init(return_value);

            size = json_array_size(obj);
            for (i = 0; i < size; i++) {
                MAKE_STD_ZVAL(retval);
                php_unqlite_json_object_to_zval(json_array_get(obj, i),
                                                retval TSRMLS_CC);
                add_next_index_zval(return_value, retval);
            }
            break;
        }
        case JSON_OBJECT: {
            void *iter;
            const char *key;
            json_t *value;
            zval *retval;

            array_init(return_value);
            /* AS_OBJECT:
             * object_init(return_value);
             */

            iter = json_object_iter(obj);
            while (iter) {
                MAKE_STD_ZVAL(retval);
                key = json_object_iter_key(iter);
                value = json_object_iter_value(iter);

                php_unqlite_json_object_to_zval(value, retval TSRMLS_CC);

                add_assoc_zval(return_value, key, retval);
                /* AS_OBJECT:
                 * add_property_zval(return_value,
                 *                   (*key ? key : "_empty_"), retval);
                 * Z_DELREF_P(retval);
                 */

                iter = json_object_iter_next(obj, iter);
            }
            break;
        }
        default:
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                             "type '%d' no yet implemented", json_typeof(obj));
            RETVAL_NULL();
    }
}

PHP_UNQLITE_API char *
php_unqlite_to_json(zval *data TSRMLS_DC)
{
    json_t *json = NULL;
    char *retval = NULL;
    int flags = JSON_COMPACT | JSON_ENCODE_ANY | JSON_PRESERVE_ORDER;
    /* flags |= JSON_ENSURE_ASCII; */

    switch (Z_TYPE_P(data)) {
        case IS_ARRAY:
            if (php_unqlite_is_array(data TSRMLS_CC)) {
                json = json_array();
            }
        case IS_OBJECT:
            if (json == NULL) {
                json = json_object();
            }
            break;
    }

    if (json == NULL) {
        return NULL;
    }

    php_unqlite_to_json_object(&data, json TSRMLS_CC);

    retval = json_dumps(json, flags);

    json_delete(json);

    if (!retval) {
        return NULL;
    }

    return retval;
}

PHP_UNQLITE_API void
php_unqlite_json_to_zval(zval *return_value, char *data, size_t len TSRMLS_DC)
{
    json_t *json = NULL;

    json = json_loadb(data, len, JSON_DECODE_ANY, NULL);
    if (!json) {
        RETURN_STRINGL(data, len, 1);
    }

    php_unqlite_json_object_to_zval(json, return_value TSRMLS_CC);

    json_delete(json);
}

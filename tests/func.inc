<?php
namespace UnQLite;

/* db */
function _db_init($scriptfile) {
    $db = dirname($scriptfile) . '/' . basename($scriptfile, '.php') .'.db';
    return $db;
}

function _db_release($db) {
    if (is_file($db)) {
        unlink($db);
    }
}

/* kv */
function _kv_store($kv, $key, $val) {
    echo "store: $key: ", var_export($kv->store($key, $val), true), "\n";
}
function _kv_append($kv, $key, $val) {
    echo "append: $key: ", var_export($kv->append($key, $val), true), "\n";
}
function _kv_fetch($kv, $key) {
    echo "fetch: $key: ", var_export($kv->fetch($key), true), "\n";
}
function _kv_delete($kv, $key) {
    echo "delete: $key: ", var_export($kv->delete($key), true), "\n";
}

/* cursor */
function _cursor_key($cur) {
    echo "key: ", var_export($cur->key(), true), "\n";
}
function _cursor_data($cur) {
    echo "data: ", var_export($cur->data(), true), "\n";
}
function _cursor_first($cur) {
    echo "first: ", var_export($cur->first(), true), "\n";
}
function _cursor_last($cur) {
    echo "last: ", var_export($cur->last(), true), "\n";
}
function _cursor_next($cur) {
    echo "next: ", var_export($cur->next(), true), "\n";
}
function _cursor_prev($cur) {
    echo "prev: ", var_export($cur->prev(), true), "\n";
}
function _cursor_exists($cur) {
    echo "exists: ", var_export($cur->exists(), true), "\n";
}
function _cursor_delete($cur) {
    echo "delete: ", var_export($cur->delete(), true), "\n";
}
function _cursor_seek($cur, $key) {
    echo "seek: $key: ", var_export($cur->seek($key), true), "\n";
}

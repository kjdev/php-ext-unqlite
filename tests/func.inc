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

/* kvs */
function _kvs_store($kvs, $key, $val) {
    echo "store: $key: ", var_export($kvs->store($key, $val), true), "\n";
}
function _kvs_append($kvs, $key, $val) {
    echo "append: $key: ", var_export($kvs->append($key, $val), true), "\n";
}
function _kvs_fetch($kvs, $key) {
    echo "fetch: $key: ", var_export($kvs->fetch($key), true), "\n";
}
function _kvs_delete($kvs, $key) {
    echo "delete: $key: ", var_export($kvs->delete($key), true), "\n";
}

/* kvs cursor */
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

/* doc */
function _doc_store($doc, $data) {
    echo "store: ", var_export($doc->store($data), true), "\n";
}
function _doc_fetch($doc, $offset = 0) {
    echo "fetch: ", var_export($doc->fetch($offset), true), "\n";
}
function _doc_delete($doc, $id) {
    echo "delete: ", var_export($doc->remove($id), true), "\n";
}

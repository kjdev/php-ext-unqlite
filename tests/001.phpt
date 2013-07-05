--TEST--
simple
--SKIPIF--
--FILE--
<?php
namespace UnQLite;

if (!extension_loaded('unqlite')) {
    dl('callmap.' . PHP_SHLIB_SUFFIX);
}

include_once dirname(__FILE__) . '/func.inc';

$db = _db_init(__FILE__);

$kv = new Kv($db);
var_dump($kv);

_kv_store($kv, "foo", "bar");
_kv_fetch($kv, "foo");
_kv_delete($kv, "foo");
_kv_fetch($kv, "foo");

_kv_store($kv, "test", "123");
_kv_fetch($kv, "test");
_kv_append($kv, "test", "456");
_kv_fetch($kv, "test");

$kv = new Kv("/path/to/unexisted/file");
var_dump($kv);
_kv_fetch($kv, "x");

_db_release($db);
?>
--EXPECTF--
object(UnQLite\Kv)#%d (0) {
}
store: foo: true
fetch: foo: 'bar'
delete: foo: true
fetch: foo: false
store: test: true
fetch: test: '123'
append: test: true
fetch: test: '123456'
object(UnQLite\Kv)#%d (0) {
}
fetch: x: false

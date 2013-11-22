--TEST--
kvs simple
--SKIPIF--
--FILE--
<?php
namespace UnQLite;

if (!extension_loaded('unqlite')) {
    dl('unqlite.' . PHP_SHLIB_SUFFIX);
}

include_once dirname(__FILE__) . '/func.inc';

$dbfile = _db_init(__FILE__);

$db = new DB($dbfile);
var_dump($db);

$kvs = $db->kvs();
var_dump($kvs);

_kvs_store($kvs, "foo", "bar");
_kvs_fetch($kvs, "foo");
_kvs_delete($kvs, "foo");
_kvs_fetch($kvs, "foo");

unset($kvs);


$kvs = new Kvs($db);
var_dump($kvs);

_kvs_store($kvs, "test", "123");
_kvs_fetch($kvs, "test");
_kvs_append($kvs, "test", "456");
_kvs_fetch($kvs, "test");

unset($kvs);
unset($db);


$db = new DB("/path/to/unexisted/file");
var_dump($db);
$kvs = $db->kvs();
var_dump($kvs);
_kvs_fetch($kvs, "x");

_db_release($dbfile);
?>
--EXPECTF--
object(UnQLite\DB)#%d (0) {
}
object(UnQLite\Kvs)#%d (0) {
}
store: foo: true
fetch: foo: 'bar'
delete: foo: true
fetch: foo: NULL
object(UnQLite\Kvs)#%d (0) {
}
store: test: true
fetch: test: '123'
append: test: true
fetch: test: '123456'
object(UnQLite\DB)#%d (0) {
}
object(UnQLite\Kvs)#%d (0) {
}
fetch: x: NULL

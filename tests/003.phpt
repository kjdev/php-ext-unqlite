--TEST--
kvs transaction
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
$kvs = new Kvs($db);

_kvs_store($kvs, "foo", "bar");
_kvs_store($kvs, "hoge", "fuga");

_kvs_fetch($kvs, "foo");
_kvs_fetch($kvs, "hoge");
_kvs_fetch($kvs, "test");

echo "=== commit ===\n";
echo "commit: ", var_export($kvs->commit(), true), "\n";

echo "=== transaction : begin ===\n";
echo "begin: ", var_export($kvs->begin(), true), "\n";
_kvs_store($kvs, "test", "message");
_kvs_fetch($kvs, "test");
_kvs_append($kvs, "foo", "message");
_kvs_fetch($kvs, "foo");

echo "=== transaction : rollback ===\n";
echo "rollback: ", var_export($kvs->rollback(), true), "\n";
_kvs_fetch($kvs, "foo");
_kvs_fetch($kvs, "hoge");
_kvs_fetch($kvs, "test");

echo "=== transaction : begin ===\n";
echo "begin: ", var_export($kvs->begin(), true), "\n";
_kvs_store($kvs, "test", "MESSAGE");
_kvs_fetch($kvs, "test");
_kvs_append($kvs, "foo", "MESSAGE");
_kvs_fetch($kvs, "foo");

echo "=== transaction : commit ===\n";
echo "commit: ", var_export($kvs->commit(), true), "\n";
_kvs_fetch($kvs, "foo");
_kvs_fetch($kvs, "hoge");
_kvs_fetch($kvs, "test");

$db->close();
_db_release($dbfile);
?>
--EXPECTF--
store: foo: true
store: hoge: true
fetch: foo: 'bar'
fetch: hoge: 'fuga'
fetch: test: NULL
=== commit ===
commit: true
=== transaction : begin ===
begin: true
store: test: true
fetch: test: 'message'
append: foo: true
fetch: foo: 'barmessage'
=== transaction : rollback ===
rollback: true
fetch: foo: 'bar'
fetch: hoge: 'fuga'
fetch: test: NULL
=== transaction : begin ===
begin: true
store: test: true
fetch: test: 'MESSAGE'
append: foo: true
fetch: foo: 'barMESSAGE'
=== transaction : commit ===
commit: true
fetch: foo: 'barMESSAGE'
fetch: hoge: 'fuga'
fetch: test: 'MESSAGE'

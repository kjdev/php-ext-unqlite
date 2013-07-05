--TEST--
transaction
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

_kv_store($kv, "foo", "bar");
_kv_store($kv, "hoge", "fuga");

_kv_fetch($kv, "foo");
_kv_fetch($kv, "hoge");
_kv_fetch($kv, "test");

echo "=== commit ===\n";
echo "commit: ", var_export($kv->commit(), true), "\n";

echo "=== transaction : begin ===\n";
echo "begin: ", var_export($kv->begin(), true), "\n";
_kv_store($kv, "test", "message");
_kv_fetch($kv, "test");
_kv_append($kv, "foo", "message");
_kv_fetch($kv, "foo");

echo "=== transaction : rollback ===\n";
echo "rollback: ", var_export($kv->rollback(), true), "\n";
_kv_fetch($kv, "foo");
_kv_fetch($kv, "hoge");
_kv_fetch($kv, "test");

echo "=== transaction : begin ===\n";
echo "begin: ", var_export($kv->begin(), true), "\n";
_kv_store($kv, "test", "MESSAGE");
_kv_fetch($kv, "test");
_kv_append($kv, "foo", "MESSAGE");
_kv_fetch($kv, "foo");

echo "=== transaction : commit ===\n";
echo "commit: ", var_export($kv->commit(), true), "\n";
_kv_fetch($kv, "foo");
_kv_fetch($kv, "hoge");
_kv_fetch($kv, "test");

_db_release($db);
?>
--EXPECTF--
store: foo: true
store: hoge: true
fetch: foo: 'bar'
fetch: hoge: 'fuga'
fetch: test: false
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
fetch: test: false
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

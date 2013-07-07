--TEST--
cursor constructor
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

echo "=== Cursor 1 ===\n";
$cur1 = new KvCursor($kv, CURSOR_FIRST);
var_dump($cur1);

//_cursor_first($cur1);
_cursor_exists($cur1);
_cursor_key($cur1);
_cursor_data($cur1);

_cursor_next($cur1);
_cursor_exists($cur1);
_cursor_key($cur1);
_cursor_data($cur1);

_cursor_next($cur1);
_cursor_exists($cur1);

echo "=== Cursor 2 ===\n";
$cur2 = new KvCursor($kv);
var_dump($cur2);

for ($cur2->first(); $cur2->exists(); $cur2->next()) {
    _cursor_key($cur2);
    _cursor_data($cur2);
}

echo "=== Cursor 3 ===\n";
$cur3 = new KvCursor($kv, CURSOR_LAST);
var_dump($cur3);

//_cursor_last($cur3);
_cursor_exists($cur3);
_cursor_key($cur3);
_cursor_data($cur3);

_cursor_prev($cur3);
_cursor_exists($cur3);
_cursor_key($cur3);
_cursor_data($cur3);

_cursor_prev($cur3);
_cursor_exists($cur3);

echo "=== Cursor 4 ===\n";
$cur4 = new KvCursor($kv);
var_dump($cur4);

_cursor_seek($cur4, "NON EXISTENT");

_cursor_seek($cur4, "foo");
_cursor_key($cur4);
_cursor_data($cur4);

_cursor_delete($cur4);
_cursor_seek($cur4, "foo");

_db_release($db);
?>
--EXPECTF--
store: foo: true
store: hoge: true
=== Cursor 1 ===
object(UnQLite\KvCursor)#%d (0) {
}
exists: true
key: 'hoge'
data: 'fuga'
next: true
exists: true
key: 'foo'
data: 'bar'
next: false
exists: false
=== Cursor 2 ===
object(UnQLite\KvCursor)#%d (0) {
}
key: 'hoge'
data: 'fuga'
key: 'foo'
data: 'bar'
=== Cursor 3 ===
object(UnQLite\KvCursor)#%d (0) {
}
exists: true
key: 'foo'
data: 'bar'
prev: true
exists: true
key: 'hoge'
data: 'fuga'
prev: false
exists: false
=== Cursor 4 ===
object(UnQLite\KvCursor)#%d (0) {
}
seek: NON EXISTENT: false
seek: foo: true
key: 'foo'
data: 'bar'
delete: true
seek: foo: false

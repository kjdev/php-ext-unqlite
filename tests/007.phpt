--TEST--
kvs cursor constructor
--SKIPIF--
--FILE--
<?php
namespace UnQLite;

if (!extension_loaded('unqlite')) {
    dl('callmap.' . PHP_SHLIB_SUFFIX);
}

include_once dirname(__FILE__) . '/func.inc';

$dbfile = _db_init(__FILE__);

$db = new DB($dbfile);
$kvs = $db->kvs();

_kvs_store($kvs, "a", "AAA");
_kvs_store($kvs, "b", "BBB");
_kvs_store($kvs, "c", "CCC");
_kvs_store($kvs, "d", "DDD");

echo "=== Cursor 1 ===\n";
$cur1 = new Kvs\Cursor($kvs, CURSOR_FIRST);
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
_cursor_key($cur1);
_cursor_data($cur1);

_cursor_next($cur1);
_cursor_exists($cur1);
_cursor_key($cur1);
_cursor_data($cur1);

_cursor_next($cur1);
_cursor_exists($cur1);
_cursor_key($cur1);
_cursor_data($cur1);

echo "=== Cursor 2 ===\n";
$cur2 = new Kvs\Cursor($kvs);
var_dump($cur2);

for ($cur2->first(); $cur2->exists(); $cur2->next()) {
    _cursor_key($cur2);
    _cursor_data($cur2);
}

echo "=== Cursor 3 ===\n";
$cur3 = new Kvs\Cursor($kvs, CURSOR_LAST);
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
_cursor_key($cur3);
_cursor_data($cur3);

_cursor_prev($cur3);
_cursor_exists($cur3);
_cursor_key($cur3);
_cursor_data($cur3);

_cursor_prev($cur3);
_cursor_exists($cur3);
_cursor_key($cur3);
_cursor_data($cur3);

echo "=== Cursor 4 ===\n";
$cur4 = new Kvs\Cursor($kvs);
var_dump($cur4);

_cursor_seek($cur4, "NON EXISTENT");

_cursor_seek($cur4, "b");
_cursor_key($cur4);
_cursor_data($cur4);

_cursor_delete($cur4);
_cursor_seek($cur4, "b");

echo "=== Cursor 5 ===\n";
$cur5 = new Kvs\Cursor($kvs);
_cursor_first($cur5);
_cursor_key($cur5);
_cursor_data($cur5);

echo "=== Cursor 6 ===\n";
$cur6 = new Kvs\Cursor($kvs);
_cursor_last($cur6);
_cursor_key($cur6);
_cursor_data($cur6);

_db_release($dbfile);
?>
--EXPECTF--
store: a: true
store: b: true
store: c: true
store: d: true
=== Cursor 1 ===
object(UnQLite\Kvs\Cursor)#%d (0) {
}
exists: true
key: 'd'
data: 'DDD'
next: true
exists: true
key: 'c'
data: 'CCC'
next: true
exists: true
key: 'b'
data: 'BBB'
next: true
exists: true
key: 'a'
data: 'AAA'
next: false
exists: false
key: false
data: NULL
=== Cursor 2 ===
object(UnQLite\Kvs\Cursor)#%d (0) {
}
key: 'd'
data: 'DDD'
key: 'c'
data: 'CCC'
key: 'b'
data: 'BBB'
key: 'a'
data: 'AAA'
=== Cursor 3 ===
object(UnQLite\Kvs\Cursor)#%d (0) {
}
exists: true
key: 'a'
data: 'AAA'
prev: true
exists: true
key: 'b'
data: 'BBB'
prev: true
exists: true
key: 'c'
data: 'CCC'
prev: true
exists: true
key: 'd'
data: 'DDD'
prev: false
exists: false
key: false
data: NULL
=== Cursor 4 ===
object(UnQLite\Kvs\Cursor)#%d (0) {
}
seek: NON EXISTENT: false
seek: b: true
key: 'b'
data: 'BBB'
delete: true
seek: b: false
=== Cursor 5 ===
first: true
key: 'd'
data: 'DDD'
=== Cursor 6 ===
last: true
key: 'a'
data: 'AAA'

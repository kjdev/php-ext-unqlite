--TEST--
cursor
--SKIPIF--
--FILE--
<?php
namespace UnQLite;

if (!extension_loaded('unqlite')) {
    dl('callmap.' . PHP_SHLIB_SUFFIX);
}

$db = dirname(__FILE__) . '/002.db';

$kv = new Kv($db);
$kv->store("foo", "bar");
$kv->store("hoge", "fuga");

echo "=== Cursor 1 ===\n";
$cur1 = $kv->cursor();
var_dump($cur1);

var_dump($cur1->first());
var_dump($cur1->exists());
var_dump($cur1->key());
var_dump($cur1->data());

var_dump($cur1->next());
var_dump($cur1->exists());
var_dump($cur1->key());
var_dump($cur1->data());

var_dump($cur1->next());
var_dump($cur1->exists());

echo "=== Cursor 2 ===\n";
$cur2 = $kv->cursor();
var_dump($cur2);

for ($cur2->first(); $cur2->exists(); $cur2->next()) {
    var_dump($cur2->key());
    var_dump($cur2->data());
}

echo "=== Cursor 3 ===\n";
$cur3 = $kv->cursor();
var_dump($cur3);

var_dump($cur3->last());
var_dump($cur3->exists());
var_dump($cur3->key());
var_dump($cur3->data());

var_dump($cur3->prev());
var_dump($cur3->exists());
var_dump($cur3->key());
var_dump($cur3->data());

var_dump($cur3->prev());
var_dump($cur3->exists());

echo "=== Cursor 4 ===\n";
$cur4 = $kv->cursor();
var_dump($cur4);

var_dump($cur4->seek("NON EXISTENT"));

var_dump($cur4->seek("foo"));
var_dump($cur4->key());
var_dump($cur4->data());

var_dump($cur4->delete());
var_dump($cur4->seek("foo"));

if (is_file($db)) {
    unlink($db);
}
?>
--EXPECTF--
=== Cursor 1 ===
object(UnQLite\KvCursor)#%d (0) {
}
bool(true)
bool(true)
string(4) "hoge"
string(4) "fuga"
bool(true)
bool(true)
string(3) "foo"
string(3) "bar"
bool(false)
bool(false)
=== Cursor 2 ===
object(UnQLite\KvCursor)#%d (0) {
}
string(4) "hoge"
string(4) "fuga"
string(3) "foo"
string(3) "bar"
=== Cursor 3 ===
object(UnQLite\KvCursor)#%d (0) {
}
bool(true)
bool(true)
string(3) "foo"
string(3) "bar"
bool(true)
bool(true)
string(4) "hoge"
string(4) "fuga"
bool(false)
bool(false)
=== Cursor 4 ===
object(UnQLite\KvCursor)#%d (0) {
}
bool(false)
bool(true)
string(3) "foo"
string(3) "bar"
bool(true)
bool(false)

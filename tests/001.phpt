--TEST--
simple
--SKIPIF--
--FILE--
<?php
namespace UnQLite;

if (!extension_loaded('unqlite')) {
    dl('callmap.' . PHP_SHLIB_SUFFIX);
}

$db = dirname(__FILE__) . '/001.db';

$kv = new Kv($db);
var_dump($kv);

var_dump($kv->store("foo", "bar"));
var_dump($kv->fetch("foo"));
var_dump($kv->delete("foo"));
var_dump($kv->fetch("foo"));

var_dump($kv->store("test", "123"));
var_dump($kv->fetch("test"));
var_dump($kv->append("test", "456"));
var_dump($kv->fetch("test"));

$kv = new Kv("/path/to/unexisted/file");
var_dump($kv);
var_dump($kv->fetch("x"));

if (is_file($db)) {
    unlink($db);
}
?>
--EXPECTF--
object(UnQLite\Kv)#%d (0) {
}
bool(true)
string(3) "bar"
bool(true)
bool(false)
bool(true)
string(3) "123"
bool(true)
string(6) "123456"
object(UnQLite\Kv)#%d (0) {
}
bool(false)

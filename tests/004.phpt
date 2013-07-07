--TEST--
duplicate kv store
--SKIPIF--
--FILE--
<?php
namespace UnQLite;

if (!extension_loaded('unqlite')) {
    dl('callmap.' . PHP_SHLIB_SUFFIX);
}

include_once dirname(__FILE__) . '/func.inc';

$db = _db_init(__FILE__);

$kv1 = new Kv($db);
$kv2 = new Kv($db);

var_dump($kv1->store("foo", "bar"));
var_dump($kv2->store("foo", "bar"));

_db_release($db);
?>
--EXPECTF--
bool(true)

Warning: UnQLite\Kv::store(): Another process or thread hold the requested lock
Another process or thread have a reserved lock on this database
xOpen() method of the underlying KV engine 'hash' failed
 in %s on line %d
bool(false)

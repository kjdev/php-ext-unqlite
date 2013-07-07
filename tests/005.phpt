--TEST--
kv open/close
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
$kv->close();

$kv = new Kv(':mem:');
var_dump($kv);
unset($kv);

_db_release($db);
?>
--EXPECTF--
object(UnQLite\Kv)#%d (0) {
}
object(UnQLite\Kv)#%d (0) {
}
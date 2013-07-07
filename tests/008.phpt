--TEST--
clone kv
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

_db_release($db);
?>
--EXPECTF--
object(UnQLite\Kv)#%d (0) {
}

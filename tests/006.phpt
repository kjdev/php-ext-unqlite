--TEST--
kv config
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
var_dump($kv->config(CONFIG_DISABLE_AUTO_COMMIT));
var_dump($kv->config(12345));

_db_release($db);
?>
--EXPECTF--
bool(true)

Warning: UnQLite\Kv::config(): unknown config option in %s on line %d
bool(false)

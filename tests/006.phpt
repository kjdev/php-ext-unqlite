--TEST--
db config
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
var_dump($db->config(CONFIG_DISABLE_AUTO_COMMIT));
var_dump($db->config(12345));

_db_release($dbfile);
?>
--EXPECTF--
bool(true)

Warning: UnQLite\DB::config(): unknown config option in %s on line %d
bool(false)

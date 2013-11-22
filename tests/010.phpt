--TEST--
doc drop
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
var_dump($db);

$doc = $db->doc('test');

echo "=== db_exists ===\n";
var_dump($doc->eval("print db_exists('test')"));

echo "=== drop ===\n";
var_dump($doc->drop());

echo "=== db_exists ===\n";
var_dump($doc->eval("print db_exists('test')"));

_db_release($dbfile);
?>
--EXPECTF--
object(UnQLite\DB)#%d (0) {
}
=== db_exists ===
string(4) "true"
=== drop ===
bool(true)
=== db_exists ===
string(5) "false"

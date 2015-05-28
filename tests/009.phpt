--TEST--
doc eval
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
var_dump($doc->eval("print db_exists('hoge')"));

echo "=== empty ===\n";
var_dump($doc->eval(""));
var_dump($doc->eval(null));

echo "=== invalid script ===\n";
var_dump($doc->eval("print db_exists("));

$db->close();
_db_release($dbfile);
?>
--EXPECTF--
object(UnQLite\DB)#%d (0) {
}
=== db_exists ===
string(4) "true"
string(5) "false"
=== empty ===
bool(false)
bool(false)
=== invalid script ===

Warning: UnQLite\Doc::eval(): 1 Error: Syntax error, mismatched '(', '[' or '{'
 in %s on line %d
bool(false)

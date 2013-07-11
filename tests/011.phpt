--TEST--
doc count
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
var_dump($db);

$doc = $db->doc('test');

echo "=== count ===\n";
var_dump($doc->count());

echo "=== store * 2 ===\n";
var_dump($doc->store(array(array('a' => 'AAA'),
                           array('b' => 'BBB'))));

echo "=== count ===\n";
var_dump($doc->count());

echo "=== store * 3 ===\n";
var_dump($doc->store(array(array('x' => 'XXX'),
                           array('y' => 'YYY'),
                           array('z' => 'ZZZ'))));

echo "=== count ===\n";
var_dump($doc->count());

_db_release($dbfile);
?>
--EXPECTF--
object(UnQLite\DB)#%d (0) {
}
=== count ===
int(0)
=== store * 2 ===
bool(true)
=== count ===
int(2)
=== store * 3 ===
bool(true)
=== count ===
int(5)

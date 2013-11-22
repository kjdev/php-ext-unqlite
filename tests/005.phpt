--TEST--
db open/close
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
$db->close();
unset($db);

$db = new DB(':mem:');
var_dump($db);
unset($db);

//option
$db = new DB(':mem:', -1);
var_dump($db);
unset($db);

//error
try {
    $db = new DB(null);
    var_dump($db);
    unset($db);
} catch (\Exception $e) {
    echo get_class($e), "\n";
    echo $e->getMessage(), "\n";
}

_db_release($dbfile);
?>
--EXPECTF--
object(UnQLite\DB)#%d (0) {
}
object(UnQLite\DB)#%d (0) {
}
object(UnQLite\DB)#%d (0) {
}
UnQLite\Exception
unable to expand filepath:

--TEST--
kvs duplicate
--SKIPIF--
--FILE--
<?php
namespace UnQLite;

if (!extension_loaded('unqlite')) {
    dl('unqlite.' . PHP_SHLIB_SUFFIX);
}

include_once dirname(__FILE__) . '/func.inc';

$dbfile = _db_init(__FILE__);

function test($dbfile, $method, $params = array()) {
    $db1 = new DB($dbfile);
    $db2 = new DB($dbfile);

    $kvs1 = $db1->kvs();
    $kvs2 = $db2->kvs();

    var_dump(call_user_func_array(array($kvs1, $method), $params));
    var_dump(call_user_func_array(array($kvs2, $method), $params));

    unset($kvs1);
    unset($kvs2);

    $db1->close();
    $db2->close();
    unset($db1);
    unset($db2);
}

echo "=== store ===\n";
test($dbfile, 'store', array("foo", "bar"));

echo "=== append ===\n";
test($dbfile, 'append', array("foo", "bar"));

echo "=== fetch ===\n";
test($dbfile, 'fetch', array("foo"));

echo "=== delete ===\n";
test($dbfile, 'delete', array("foo"));

echo "=== begin ===\n";
test($dbfile, 'begin');

echo "=== rollback ===\n";
test($dbfile, 'rollback');

echo "=== commit ===\n";
test($dbfile, 'commit');

_db_release($dbfile);
?>
--EXPECTF--
=== store ===
bool(true)

Warning: UnQLite\Kvs::store(): Another process or thread hold the requested lock
Another process or thread have a reserved lock on this database
xOpen() method of the underlying KV engine 'hash' failed
 in %s on line %d
bool(false)
=== append ===
bool(true)

Warning: UnQLite\Kvs::append(): Another process or thread hold the requested lock
Another process or thread have a reserved lock on this database
 in %s on line %d
bool(false)
=== fetch ===
string(3) "bar"
string(3) "bar"
=== delete ===
bool(true)

Warning: UnQLite\Kvs::delete(): Another process or thread hold the requested lock
Another process or thread have a reserved lock on this database
 in %s on line %d
bool(false)
=== begin ===
bool(true)

Warning: UnQLite\Kvs::begin(): Another process or thread hold the requested lock
Another process or thread have a reserved lock on this database
 in %s on line %d
bool(false)
=== rollback ===
bool(true)
bool(true)
=== commit ===
bool(true)
bool(true)

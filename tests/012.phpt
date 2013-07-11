--TEST--
doc store
--SKIPIF--
--FILE--
<?php
namespace UnQLite;

if (!extension_loaded('unqlite')) {
    dl('callmap.' . PHP_SHLIB_SUFFIX);
}

include_once dirname(__FILE__) . '/func.inc';

$dbfile = _db_init(__FILE__);

function test($dbfile, $data) {
    $db = new DB($dbfile);
    $doc = $db->doc('test');

    echo "=== store ===\n";
    echo "DATA:\n";
    var_dump($data);
    echo "RESULT:\n";
    var_dump($doc->store($data));

    unset($doc);
    unset($db);
}

test($dbfile, null);
test($dbfile, 12345);
test($dbfile, 'test message');
test($dbfile, array('a', 'b'));
test($dbfile, array('a' => 'AAA', 'b' => 'BBB'));
test($dbfile, (object)array('a', 'b'));
test($dbfile, (object)array('a' => 'AAA', 'b' => 'BBB'));

_db_release($dbfile);
?>
--EXPECTF--
=== store ===
DATA:
NULL
RESULT:

Warning: UnQLite\Doc::store(): expects parameter 1 to be an array or object, null given in %s on line %d
bool(false)
=== store ===
DATA:
int(12345)
RESULT:

Warning: UnQLite\Doc::store(): expects parameter 1 to be an array or object, integer given in %s on line %d
bool(false)
=== store ===
DATA:
string(12) "test message"
RESULT:

Warning: UnQLite\Doc::store(): expects parameter 1 to be an array or object, string given in %s on line %d
bool(false)
=== store ===
DATA:
array(2) {
  [0]=>
  string(1) "a"
  [1]=>
  string(1) "b"
}
RESULT:
bool(true)
=== store ===
DATA:
array(2) {
  ["a"]=>
  string(3) "AAA"
  ["b"]=>
  string(3) "BBB"
}
RESULT:
bool(true)
=== store ===
DATA:
object(stdClass)#1 (2) {
  [0]=>
  string(1) "a"
  [1]=>
  string(1) "b"
}
RESULT:
bool(true)
=== store ===
DATA:
object(stdClass)#1 (2) {
  ["a"]=>
  string(3) "AAA"
  ["b"]=>
  string(3) "BBB"
}
RESULT:
bool(true)

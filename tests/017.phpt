--TEST--
doc fetch id (alias: fetchId)
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
$doc = $db->doc('test');

echo "=== store ===\n";
$ret = $doc->store(array(array('a' => 'AAA', 'b' => 'BBB'),
                         array('x' => 'XXX', 'y' => 'YYY', 'z' => 'ZZZ'),
                         array('foo' => 'Bar'),
                         array('hoge' => 'HOGE', 'foo' => 'bar')));
var_dump($ret);
echo 'Count: ', var_export($doc->count()), "\n";

echo "=== fetch all ===\n";
var_dump($doc->fetch_all());

function test($doc, $id) {
    echo "=== fetch id ===\n";
    echo "ID: ", var_export($id, true), "\n";
    echo "RESULT:\n";
    var_dump($doc->fetchId($id));
}

test($doc, 0);
test($doc, 1);
test($doc, 3);
test($doc, 2);
test($doc, 10);
test($doc, 'a');
test($doc, null);

_db_release($dbfile);
?>
--EXPECTF--
=== store ===
bool(true)
Count: 4
=== fetch all ===
array(4) {
  [0]=>
  array(3) {
    ["a"]=>
    string(3) "AAA"
    ["__id"]=>
    int(0)
    ["b"]=>
    string(3) "BBB"
  }
  [1]=>
  array(4) {
    ["x"]=>
    string(3) "XXX"
    ["y"]=>
    string(3) "YYY"
    ["z"]=>
    string(3) "ZZZ"
    ["__id"]=>
    int(1)
  }
  [2]=>
  array(2) {
    ["foo"]=>
    string(3) "Bar"
    ["__id"]=>
    int(2)
  }
  [3]=>
  array(3) {
    ["hoge"]=>
    string(4) "HOGE"
    ["foo"]=>
    string(3) "bar"
    ["__id"]=>
    int(3)
  }
}
=== fetch id ===
ID: 0
RESULT:
array(3) {
  ["a"]=>
  string(3) "AAA"
  ["__id"]=>
  int(0)
  ["b"]=>
  string(3) "BBB"
}
=== fetch id ===
ID: 1
RESULT:
array(4) {
  ["x"]=>
  string(3) "XXX"
  ["y"]=>
  string(3) "YYY"
  ["z"]=>
  string(3) "ZZZ"
  ["__id"]=>
  int(1)
}
=== fetch id ===
ID: 3
RESULT:
array(3) {
  ["hoge"]=>
  string(4) "HOGE"
  ["foo"]=>
  string(3) "bar"
  ["__id"]=>
  int(3)
}
=== fetch id ===
ID: 2
RESULT:
array(2) {
  ["foo"]=>
  string(3) "Bar"
  ["__id"]=>
  int(2)
}
=== fetch id ===
ID: 10
RESULT:
NULL
=== fetch id ===
ID: 'a'
RESULT:

Warning: UnQLite\Doc::fetchId(): expects parameter 1 to be an numeric: a in %s on line %d
NULL
=== fetch id ===
ID: NULL
RESULT:

Warning: UnQLite\Doc::fetchId(): expects parameter 1 to be an numeric:  in %s on line %d
NULL

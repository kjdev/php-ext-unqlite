--TEST--
doc fetch offset
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

$doc->store(array(array('a' => 'AAA'),
                  array('b' => 'BBB'),
                  array('c' => 'CCC'),
                  array('d' => 'DDD'),
                  array('e' => 'EEE')));

echo "=== fetch all ===\n";
var_dump($doc->fetch_all());

function test($doc, $offset) {
    echo "=== fetch: offset -> ", var_export($offset, true), " ===\n";
    var_dump($doc->fetch($offset));
}

test($doc, 0);
test($doc, 1);
test($doc, 3);
test($doc, 5);
test($doc, 'a');
test($doc, null);

$db->close();
_db_release($dbfile);
?>
--EXPECTF--
=== fetch all ===
array(5) {
  [0]=>
  array(2) {
    ["a"]=>
    string(3) "AAA"
    ["__id"]=>
    int(0)
  }
  [1]=>
  array(2) {
    ["__id"]=>
    int(1)
    ["b"]=>
    string(3) "BBB"
  }
  [2]=>
  array(2) {
    ["c"]=>
    string(3) "CCC"
    ["__id"]=>
    int(2)
  }
  [3]=>
  array(2) {
    ["d"]=>
    string(3) "DDD"
    ["__id"]=>
    int(3)
  }
  [4]=>
  array(2) {
    ["e"]=>
    string(3) "EEE"
    ["__id"]=>
    int(4)
  }
}
=== fetch: offset -> 0 ===
array(2) {
  ["a"]=>
  string(3) "AAA"
  ["__id"]=>
  int(0)
}
=== fetch: offset -> 1 ===
array(2) {
  ["__id"]=>
  int(1)
  ["b"]=>
  string(3) "BBB"
}
=== fetch: offset -> 3 ===
array(2) {
  ["d"]=>
  string(3) "DDD"
  ["__id"]=>
  int(3)
}
=== fetch: offset -> 5 ===
NULL
=== fetch: offset -> 'a' ===

Warning: UnQLite\Doc::fetch(): expects parameter 1 to be an numeric: a in %s on line %d
NULL
=== fetch: offset -> NULL ===

Warning: UnQLite\Doc::fetch(): expects parameter 1 to be an numeric:  in %s on line %d
NULL

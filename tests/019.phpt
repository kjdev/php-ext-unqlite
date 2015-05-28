--TEST--
doc transaction
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
$ret = $doc->store(array(array('a' => 'AAA'),
                         array('b' => 'BBB'),
                         array('c' => 'CCC')));
var_dump($ret);
echo "COUNT:\n";
var_dump($doc->count());
echo "FETCH:\n";
var_dump($doc->fetch_all());

echo "=== commit ===\n";
var_dump($doc->commit());

echo "=== begin ===\n";
var_dump($doc->begin());

function test($doc) {
    echo "... store ...\n";
    var_dump($doc->store(array(array('x' => 'X'))));
    echo "... fetch all ...\n";
    var_dump($doc->fetch_all());

    echo "... remove ...\n";
    var_dump($doc->remove(1));
    echo "... fetch all ...\n";
    var_dump($doc->fetch_all());
}

test($doc);

echo "=== rollback ===\n";
var_dump($doc->rollback());
echo "FETCH:\n";
var_dump($doc->fetch_all());

echo "=== begin ===\n";
var_dump($doc->begin());

test($doc);

echo "=== commit ===\n";
var_dump($doc->commit());
echo "FETCH:\n";
var_dump($doc->fetch_all());

$db->close();
_db_release($dbfile);
?>
--EXPECTF--
=== store ===
bool(true)
COUNT:
int(3)
FETCH:
array(3) {
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
}
=== commit ===
bool(true)
=== begin ===
bool(true)
... store ...
bool(true)
... fetch all ...
array(4) {
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
    ["x"]=>
    string(1) "X"
    ["__id"]=>
    int(3)
  }
}
... remove ...
bool(true)
... fetch all ...
array(3) {
  [0]=>
  array(2) {
    ["a"]=>
    string(3) "AAA"
    ["__id"]=>
    int(0)
  }
  [1]=>
  array(2) {
    ["c"]=>
    string(3) "CCC"
    ["__id"]=>
    int(2)
  }
  [2]=>
  array(2) {
    ["x"]=>
    string(1) "X"
    ["__id"]=>
    int(3)
  }
}
=== rollback ===
bool(true)
FETCH:
array(3) {
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
}
=== begin ===
bool(true)
... store ...
bool(true)
... fetch all ...
array(4) {
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
    ["x"]=>
    string(1) "X"
    ["__id"]=>
    int(3)
  }
}
... remove ...
bool(true)
... fetch all ...
array(3) {
  [0]=>
  array(2) {
    ["a"]=>
    string(3) "AAA"
    ["__id"]=>
    int(0)
  }
  [1]=>
  array(2) {
    ["c"]=>
    string(3) "CCC"
    ["__id"]=>
    int(2)
  }
  [2]=>
  array(2) {
    ["x"]=>
    string(1) "X"
    ["__id"]=>
    int(3)
  }
}
=== commit ===
bool(true)
FETCH:
array(3) {
  [0]=>
  array(2) {
    ["a"]=>
    string(3) "AAA"
    ["__id"]=>
    int(0)
  }
  [1]=>
  array(2) {
    ["c"]=>
    string(3) "CCC"
    ["__id"]=>
    int(2)
  }
  [2]=>
  array(2) {
    ["x"]=>
    string(1) "X"
    ["__id"]=>
    int(3)
  }
}

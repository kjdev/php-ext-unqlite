--TEST--
doc fetch all
--SKIPIF--
--FILE--
<?php
namespace UnQLite;

if (!extension_loaded('unqlite')) {
    dl('unqlite.' . PHP_SHLIB_SUFFIX);
}

include_once dirname(__FILE__) . '/func.inc';

function test($data) {
    $dbfile = _db_init(__FILE__);

    $db = new DB($dbfile);
    $doc = $db->doc('test');

    echo "=== fetch all ===\n";
    echo "DATA:\n";
    var_dump($data);
    echo "STORE:\n";
    var_dump($doc->store($data));
    echo "RESULT:\n";
    var_dump($doc->fetch_all());

    unset($doc);

    $db->close();
    unset($db);

    _db_release($dbfile);
}

test(array('a', 'b'));
test(array('a' => 'AAA-1', 'b' => 'BBB-1'));
test(array(array('a' => 'AAA-2', 'b' => 'BBB-2'),
           array('x' => 'XXX-2', 'y' => 'YYY-2', 'z' => 'ZZZ-2')));
test((object)array('a', 'b'));
test((object)array('a' => 'AAA-4', 'b' => 'BBB-4'));
test((object)array((object)array('a' => 'AAA-5', 'b' => 'BBB-5'),
                   (object)array('x' => 'XXX-5', 'y' => 'YYY-5', 'z' => 'ZZZ-5')));
?>
--EXPECTF--
=== fetch all ===
DATA:
array(2) {
  [0]=>
  string(1) "a"
  [1]=>
  string(1) "b"
}
STORE:
bool(true)
RESULT:
array(2) {
  [0]=>
  string(1) "a"
  [1]=>
  string(1) "b"
}
=== fetch all ===
DATA:
array(2) {
  ["a"]=>
  string(5) "AAA-1"
  ["b"]=>
  string(5) "BBB-1"
}
STORE:
bool(true)
RESULT:
array(1) {
  [0]=>
  array(3) {
    ["a"]=>
    string(5) "AAA-1"
    ["__id"]=>
    int(0)
    ["b"]=>
    string(5) "BBB-1"
  }
}
=== fetch all ===
DATA:
array(2) {
  [0]=>
  array(2) {
    ["a"]=>
    string(5) "AAA-2"
    ["b"]=>
    string(5) "BBB-2"
  }
  [1]=>
  array(3) {
    ["x"]=>
    string(5) "XXX-2"
    ["y"]=>
    string(5) "YYY-2"
    ["z"]=>
    string(5) "ZZZ-2"
  }
}
STORE:
bool(true)
RESULT:
array(2) {
  [0]=>
  array(3) {
    ["a"]=>
    string(5) "AAA-2"
    ["__id"]=>
    int(0)
    ["b"]=>
    string(5) "BBB-2"
  }
  [1]=>
  array(4) {
    ["x"]=>
    string(5) "XXX-2"
    ["y"]=>
    string(5) "YYY-2"
    ["z"]=>
    string(5) "ZZZ-2"
    ["__id"]=>
    int(1)
  }
}
=== fetch all ===
DATA:
object(stdClass)#%d (2) {
  [0]=>
  string(1) "a"
  [1]=>
  string(1) "b"
}
STORE:
bool(true)
RESULT:
array(1) {
  [0]=>
  array(3) {
    [0]=>
    string(1) "a"
    [1]=>
    string(1) "b"
    [2]=>
    int(0)
  }
}
=== fetch all ===
DATA:
object(stdClass)#%d (2) {
  ["a"]=>
  string(5) "AAA-4"
  ["b"]=>
  string(5) "BBB-4"
}
STORE:
bool(true)
RESULT:
array(1) {
  [0]=>
  array(3) {
    ["a"]=>
    string(5) "AAA-4"
    ["__id"]=>
    int(0)
    ["b"]=>
    string(5) "BBB-4"
  }
}
=== fetch all ===
DATA:
object(stdClass)#%d (2) {
  [0]=>
  object(stdClass)#%d (2) {
    ["a"]=>
    string(5) "AAA-5"
    ["b"]=>
    string(5) "BBB-5"
  }
  [1]=>
  object(stdClass)#2 (3) {
    ["x"]=>
    string(5) "XXX-5"
    ["y"]=>
    string(5) "YYY-5"
    ["z"]=>
    string(5) "ZZZ-5"
  }
}
STORE:
bool(true)
RESULT:
array(1) {
  [0]=>
  array(3) {
    [0]=>
    array(2) {
      ["a"]=>
      string(5) "AAA-5"
      ["b"]=>
      string(5) "BBB-5"
    }
    [1]=>
    array(3) {
      ["x"]=>
      string(5) "XXX-5"
      ["y"]=>
      string(5) "YYY-5"
      ["z"]=>
      string(5) "ZZZ-5"
    }
    [2]=>
    int(0)
  }
}

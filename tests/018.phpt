--TEST--
doc remove
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
echo "COUNT:\n";
var_dump($doc->count());
echo "FETCH:\n";
var_dump($doc->fetch_all());

function test($doc, $id) {
    echo "=== remove ===\n";
    echo "ID: ", var_export($id, true), "\n";
    echo "RESULT:\n";
    var_dump($doc->remove($id));
    echo "COUNT:\n";
    var_dump($doc->count());
    echo "FETCH:\n";
    var_dump($doc->fetch_all());
}

test($doc, 0);
test($doc, 2);

echo "=== store ===\n";
$ret = $doc->store(array(array('test' => 'TEST'),
                         array('huge' => 'HUGE')));
var_dump($ret);
echo "COUNT:\n";
var_dump($doc->count());
echo "FETCH:\n";
var_dump($doc->fetch_all());


test($doc, 1);
test($doc, 5);

test($doc, 10);
test($doc, 'a');
test($doc, null);

_db_release($dbfile);
?>
--EXPECTF--
=== store ===
bool(true)
COUNT:
int(4)
FETCH:
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
=== remove ===
ID: 0
RESULT:
bool(true)
COUNT:
int(3)
FETCH:
array(3) {
  [0]=>
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
  [1]=>
  array(2) {
    ["foo"]=>
    string(3) "Bar"
    ["__id"]=>
    int(2)
  }
  [2]=>
  array(3) {
    ["hoge"]=>
    string(4) "HOGE"
    ["foo"]=>
    string(3) "bar"
    ["__id"]=>
    int(3)
  }
}
=== remove ===
ID: 2
RESULT:
bool(true)
COUNT:
int(2)
FETCH:
array(2) {
  [0]=>
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
  [1]=>
  array(3) {
    ["hoge"]=>
    string(4) "HOGE"
    ["foo"]=>
    string(3) "bar"
    ["__id"]=>
    int(3)
  }
}
=== store ===
bool(true)
COUNT:
int(4)
FETCH:
array(4) {
  [0]=>
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
  [1]=>
  array(3) {
    ["hoge"]=>
    string(4) "HOGE"
    ["foo"]=>
    string(3) "bar"
    ["__id"]=>
    int(3)
  }
  [2]=>
  array(2) {
    ["test"]=>
    string(4) "TEST"
    ["__id"]=>
    int(4)
  }
  [3]=>
  array(2) {
    ["huge"]=>
    string(4) "HUGE"
    ["__id"]=>
    int(5)
  }
}
=== remove ===
ID: 1
RESULT:
bool(true)
COUNT:
int(3)
FETCH:
array(3) {
  [0]=>
  array(3) {
    ["hoge"]=>
    string(4) "HOGE"
    ["foo"]=>
    string(3) "bar"
    ["__id"]=>
    int(3)
  }
  [1]=>
  array(2) {
    ["test"]=>
    string(4) "TEST"
    ["__id"]=>
    int(4)
  }
  [2]=>
  array(2) {
    ["huge"]=>
    string(4) "HUGE"
    ["__id"]=>
    int(5)
  }
}
=== remove ===
ID: 5
RESULT:
bool(true)
COUNT:
int(2)
FETCH:
array(2) {
  [0]=>
  array(3) {
    ["hoge"]=>
    string(4) "HOGE"
    ["foo"]=>
    string(3) "bar"
    ["__id"]=>
    int(3)
  }
  [1]=>
  array(2) {
    ["test"]=>
    string(4) "TEST"
    ["__id"]=>
    int(4)
  }
}
=== remove ===
ID: 10
RESULT:
bool(false)
COUNT:
int(2)
FETCH:
array(2) {
  [0]=>
  array(3) {
    ["hoge"]=>
    string(4) "HOGE"
    ["foo"]=>
    string(3) "bar"
    ["__id"]=>
    int(3)
  }
  [1]=>
  array(2) {
    ["test"]=>
    string(4) "TEST"
    ["__id"]=>
    int(4)
  }
}
=== remove ===
ID: 'a'
RESULT:

Warning: UnQLite\Doc::remove(): expects parameter 1 to be an numeric: a in %s on line %d
bool(false)
COUNT:
int(2)
FETCH:
array(2) {
  [0]=>
  array(3) {
    ["hoge"]=>
    string(4) "HOGE"
    ["foo"]=>
    string(3) "bar"
    ["__id"]=>
    int(3)
  }
  [1]=>
  array(2) {
    ["test"]=>
    string(4) "TEST"
    ["__id"]=>
    int(4)
  }
}
=== remove ===
ID: NULL
RESULT:

Warning: UnQLite\Doc::remove(): expects parameter 1 to be an numeric:  in %s on line %d
bool(false)
COUNT:
int(2)
FETCH:
array(2) {
  [0]=>
  array(3) {
    ["hoge"]=>
    string(4) "HOGE"
    ["foo"]=>
    string(3) "bar"
    ["__id"]=>
    int(3)
  }
  [1]=>
  array(2) {
    ["test"]=>
    string(4) "TEST"
    ["__id"]=>
    int(4)
  }
}

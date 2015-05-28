--TEST--
doc json types
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

$data = array('int' => 12345,
              'double' => 1.2345,
              'bool-true' => true,
              'bool-false' => false,
              'null' => null,
              'string' => 'message',
              'array' => array('a'),
              'object' => (object)array('b'));

$doc->store($data);
var_dump($doc->fetch_all());
$doc->remove(0);

/* fail ? */
$data = array(-9876,
              -98.765,
              true,
              false,
              null,
              'MESSAGE',
              array('A'),
              (object)array('B'));

$doc->store($data);
var_dump($doc->fetch_all());

$db->close();
_db_release($dbfile);
?>
--EXPECTF--
array(1) {
  [0]=>
  array(9) {
    ["object"]=>
    array(1) {
      [0]=>
      string(1) "b"
    }
    ["int"]=>
    int(12345)
    ["null"]=>
    NULL
    ["bool-false"]=>
    bool(false)
    ["bool-true"]=>
    bool(true)
    ["double"]=>
    float(1.2345)
    ["string"]=>
    string(7) "message"
    ["array"]=>
    array(1) {
      [0]=>
      string(1) "a"
    }
    ["__id"]=>
    int(0)
  }
}
array(0) {
}

--TEST--
doc simple
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

$doc = $db->doc('test-1');
var_dump($doc);

_doc_store($doc, array("foo" => "bar"));
_doc_fetch($doc);
_doc_delete($doc, 0);
_doc_fetch($doc);

unset($doc);


$doc = new Doc($db, 'test-2');
var_dump($doc);

_doc_store($doc, array("test" => "123"));
_doc_fetch($doc);

unset($doc);
unset($db);

_db_release($dbfile);
?>
--EXPECTF--
object(UnQLite\DB)#%d (0) {
}
object(UnQLite\Doc)#%d (0) {
}
store: true
fetch: array (
  'foo' => 'bar',
  '__id' => 0,
)
delete: true
fetch: NULL
object(UnQLite\Doc)#%d (0) {
}
store: true
fetch: array (
  'test' => '123',
  '__id' => 0,
)

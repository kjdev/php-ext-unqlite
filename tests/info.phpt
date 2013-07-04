--TEST--
phpinfo() displays unqlite info
--SKIPIF--
--FILE--
<?php
namespace UnQLite;

if (!extension_loaded('unqlite')) {
    dl('callmap.' . PHP_SHLIB_SUFFIX);
}

phpinfo();
?>
--EXPECTF--
%a
unqlite

unqlite support => enabled
Extension Version => %d.%d.%d
%a

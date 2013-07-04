# UnQLite Extension for PHP

This extension allows UnQLite.

Documentation for UnQLite can be found at [» http://unqlite.org/](http://unqlite.org/).

## Build

    % phpize
    % ./configure
    % make
    % make test
    % make install

## Configration

unqlite.ini:

    extension=unqlite.so

## Class

Namespace: UnQLite

### UnQLite\\Kv

* UnQLite\\Kv::\_\_construct — Create database.
* UnQLite\\Kv::store — Saves entry
* UnQLite\\Kv::fetch — Fetch data specified by key
* UnQLite\\Kv::append — Append entry data specified by key
* UnQLite\\Kv::delete — Delete entry specified by key
* UnQLite\\Kv::cursor — Create a new cursor

### UnQLite\\KvCursor

* UnQLite\\KvCursor::\_\_construct — Create a new cursor
* UnQLite\\KvCursor::first — Advances the cursor to the first
* UnQLite\\KvCursor::last — Advances the cursor to the last
* UnQLite\\KvCursor::next — Advances the cursor to the next
* UnQLite\\KvCursor::prev — Advances the cursor to the prev
* UnQLite\\KvCursor::seek — Seek cursor to key
* UnQLite\\KvCursor::exists — Check whether cursor exists
* UnQLite\\KvCursor::delete — Delete the current cursor
* UnQLite\\KvCursor::key — Returns the current key
* UnQLite\\KvCursor::data — Returns the current value

## Class Methods

* UnQLite\\Kv::\_\_construct — Create database.

  **Description**

  public **UnQLite\\Kv::\_\_construct** ( string _$filename_ [ , long _$flags_ ] )

  Open UnQLite database.

  **Pameters**

  * _fielname_

    database filaname.

  * _flags_

    options.

  **Return Values**

  Returns a new database object.

* UnQLite\\Kv::store — Saves entry

  **Description**

  public bool **UnQLite\\Kv::store** ( string _$key_ , string _$value_ )

  Saves entry.

  **Pameters**

  * _key_

    The key of the entry to be inserted.

  * _value_

    The value to be inserted.

  **Return Values**

  Returns TRUE on success or FALSE on failure.

* UnQLite\\Kv::fetch — Fetch data specified by key

  **Description**

  public string **UnQLite\\Kv::fetch** ( string _$key_ )

  Fetch data specified by key.

  **Pameters**

  * _key_

    The key of the entry to be inserted.

  **Return Values**

  Returns the associated string if the key/data pair is found, FALSE otherwise.

* UnQLite\\Kv::append — Append entry data specified by key

  **Description**

  public bool **UnQLite\\Kv::append** ( string _$key_ , string _$value_ )

  Append entry data specified by key.

  **Pameters**

  * _key_

    The key of the entry to be appended.

  * _value_

    The value to be appended.

  **Return Values**

  Returns TRUE on success or FALSE on failure.

* UnQLite\\Kv::delete — Delete entry specified by key

  **Description**

  public bool **UnQLite\\Kv::delete** ( string _$key_ )

  Delete entry specified by key.

  **Pameters**

  * _key_

    The key of the entry to be appended.

  **Return Values**

  Returns TRUE on success or FALSE on failure.

* UnQLite\\Kv::cursor — Create a new cursor

  **Description**

  public object **UnQLite\\Kv::cursor** ( [ long _$option_ ] )

  Create a new cursor.

  **Pameters**

  * _option_

    options.

  **Return Values**

  Returns a new cursor object.

* UnQLite\\KvCursor::\_\_construct — Create a new cursor

  **Description**

  public **UnQLite\\KvCursor::\_\_construct** ( object _$db_ , [ long _$option_ ] )

  Create a new cursor.

  **Pameters**

  * _db_

    database object.

  * _option_

    options.

  **Return Values**

  Returns a new cursor object.

* UnQLite\\KvCursor::first — Advances the cursor to the first

  **Description**

  public bool **UnQLite\\KvCursor::first** ( void )

  Advances the cursor to the first.

  **Pameters**

  This function has no parameters.

  **Return Values**

  Returns TRUE on success or FALSE on failure.

* UnQLite\\KvCursor::last — Advances the cursor to the last

  **Description**

  public bool **UnQLite\\KvCursor::last** ( void )

  Advances the cursor to the last.

  **Pameters**

  This function has no parameters.

  **Return Values**

  Returns TRUE on success or FALSE on failure.

* UnQLite\\KvCursor::next — Advances the cursor to the next

  **Description**

  public bool **UnQLite\\KvCursor::next** ( void )

  Advances the cursor to the next.

  **Pameters**

  This function has no parameters.

  **Return Values**

  Returns TRUE on success or FALSE on failure.

* UnQLite\\KvCursor::prev — Advances the cursor to the prev

  **Description**

  public bool **UnQLite\\KvCursor::prev** ( void )

  Advances the cursor to the prev.

  **Pameters**

  This function has no parameters.

  **Return Values**

  Returns TRUE on success or FALSE on failure.

* UnQLite\\KvCursor::seek — Seek cursor to key

  **Description**

  public bool **UnQLite\\KvCursor::seek** ( string _$key_ , [ long _$option_ ] )

  Seek cursor to key.

  **Pameters**

  * _key_

    The key of the entry to be seek.

  * _option_

    options.

  **Return Values**

  Returns TRUE on success or FALSE on failure.

* UnQLite\\KvCursor::exists — Check whether cursor exists

  **Description**

  public bool **UnQLite\\KvCursor::exists** ( void )

  Check whether cursor exists.

  **Pameters**

  This function has no parameters.

  **Return Values**

  Returns TRUE on success or FALSE on failure.

* UnQLite\\KvCursor::delete — Delete the current cursor

  **Description**

  public bool **UnQLite\\KvCursor::delete** ( void )

  Delete the current cursor.

  **Pameters**

  This function has no parameters.

  **Return Values**

  Returns TRUE on success or FALSE on failure.

* UnQLite\\KvCursor::key — Returns the current key

  **Description**

  public string **UnQLite\\KvCursor::key** ( void )

  Returns the current key.

  **Pameters**

  This function has no parameters.

  **Return Values**

  The current cursor's key as a string.

* UnQLite\\KvCursor::data — Returns the current value

  **Description**

  public string **UnQLite\\KvCursor::data** ( void )

  Returns the current value.

  **Pameters**

  This function has no parameters.

  **Return Values**

  The current cursor's key as a string.


## Examples

    namespace UnQLite;

    $db = dirname(__FILE__) . '/test.db';

    $kv = new Kv($db);
    $kv->store("foo", "bar");
    $kv->fetch("foo"); // => bar

    $kv->delete("foo");

    $kv->store("a", "A");
    $kv->store("b", "B");
    $kv->store("c", "C");

    $cursor = $kv->cursor();
    $cursor->first();
    do {
        $cursor->key();  // => a .. b  .. c
        $cursor->data(); // => A .. B  .. C
    } while ($cursor->next());

## TODO

Document-Store.

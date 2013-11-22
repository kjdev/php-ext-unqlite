# UnQLite Extension for PHP

[![Build Status](https://travis-ci.org/kjdev/php-ext-unqlite.png?branch=master)](https://travis-ci.org/kjdev/php-ext-unqlite)

This extension allows UnQLite.

Documentation for UnQLite can be found at [» http://unqlite.org/](http://unqlite.org/).


## Dependencies

An Embeddable NoSQL Database Engine.

* [UnQLite](http://unqlite.org/)

json processing is not using the jansson library.

* [jansson](http://www.digip.org/jansson/)

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

**Namespace:** UnQLite

### UnQLite\\DB

* UnQLite\\DB::\_\_construct — Create database
* UnQLite\\DB::close — Close database
* UnQLite\\DB::config — Configure a database
* UnQLite\\DB::kvs — Create a new key/value store
* UnQLite\\DB::doc — Create a new document store

### UnQLite\\Kvs

* UnQLite\\Kvs::\_\_construct — Create a new Key/Value store
* UnQLite\\Kvs::store — Saves entry
* UnQLite\\Kvs::fetch — Fetch data specified by key
* UnQLite\\Kvs::append — Append entry data specified by key
* UnQLite\\Kvs::remove — Delete entry specified by key
* UnQLite\\Kvs::begin — Manually begin a write-transaction
* UnQLite\\Kvs::commit — Commit all changes to the database
* UnQLite\\Kvs::rollback — Rollback a write-transaction
* UnQLite\\Kvs::cursor — Create a new Key/Value stotre cursor

### UnQLite\\Kvs\\Cursor

* UnQLite\\Kvs\\Cursor::\_\_construct — Create a new Key/Value stotre cursor
* UnQLite\\Kvs\\Cursor::first — Advances the cursor to the first
* UnQLite\\Kvs\\Cursor::last — Advances the cursor to the last
* UnQLite\\Kvs\\Cursor::next — Advances the cursor to the next
* UnQLite\\Kvs\\Cursor::prev — Advances the cursor to the prev
* UnQLite\\Kvs\\Cursor::seek — Seek cursor to key
* UnQLite\\Kvs\\Cursor::exists — Check whether cursor exists
* UnQLite\\Kvs\\Cursor::remove — Delete the current cursor
* UnQLite\\Kvs\\Cursor::key — Returns the current key
* UnQLite\\Kvs\\Cursor::data — Returns the current value

### UnQLite\\Doc

* UnQLite\\Doc::\_\_construct — Create a new document store
* UnQLite\\Doc::eval — Evaluate a string as Jx9 code
* UnQLite\\Doc::drop — remove a collection
* UnQLite\\Doc::count — total number of inserted records
* UnQLite\\Doc::store — Store one or more JSON values
* UnQLite\\Doc::fetch — Fetch the current record
* UnQLite\\Doc::fetch_all — Retrieve all records
* UnQLite\\Doc::fetch_id — Fetch a record via its unique ID
* UnQLite\\Doc::remove — remove a stored record
* UnQLite\\Doc::begin — Manually begin a write-transaction
* UnQLite\\Doc::commit — Commit all changes to the database
* UnQLite\\Doc::rollback — Rollback a write-transaction

## Class Methods

* UnQLite\\DB::\_\_construct — Create database

  **Description:**

  public **UnQLite\\DB::\_\_construct** ( string _$filename_ [ , long _$mode_ = UnQLite\\OPEN\_CREATE ] )

  Open UnQLite database.

  **Pameters:**

  * _fielname_

     database filaname.

  * _flags_

     options.

     * UnQLite\\OPEN\_READONLY
     * UnQLite\\OPEN\_READWRITE
     * UnQLite\\OPEN\_CREATE
     * UnQLite\\OPEN\_NOMUTEX
     * UnQLite\\OPEN\_IN\_MEMORY
     * UnQLite\\OPEN\_MMAP

  **Return Values:**

  Returns a new UnQLite\\DB object.

---

* UnQLite\\DB::close — Close database

  **Description:**

  public **UnQLite\\DB::close** ( void )

  Close database.

  **Return Values:**

  Returns TRUE on success or FALSE on failure.

---

* UnQLite\\DB::config — Configure a database

  **Description:**

  public bool **UnQLite\\DB::config** ( long _$option_ )

  Configure a database.

  **Pameters:**

  * _option_

     options.

     * UnQLite\\CONFIG\_DISABLE\_AUTO\_COMMIT

  **Return Values:**

  Returns TRUE on success or FALSE on failure.

---

* UnQLite\\DB::kvs — Create a new key/value store

  **Description:**

  public object **UnQLite\\DB::kvs** ( void )

  Create a new key/value store.

  **Return Values:**

  Returns a new UnQLite\\Kvs object

---

* UnQLite\\DB::doc — Create a new document store

  **Description:**

  public object **UnQLite\\DB::doc** ( string _$collection_ )

  Create a new document store.

  **Pameters:**

  * _collection_

     collection name.

**Return Values:**

  Returns a new UnQLite\\Doc object

---

* UnQLite\\Kvs::\_\_construct — Create a new Key/Value store

  **Description:**

  public **UnQLite\\Kvs::\_\_construct** ( object _$db_ )

  Create a new Key/Value store.

  **Pameters:**

  * _db_

     UnQLite\\DB class Object.

  **Return Values:**

  Returns a new UnQLite\\Kvs object

---

* UnQLite\\Kvs::store — Saves entry

  **Description:**

  public bool **UnQLite\\Kvs::store** ( string _$key_ , string _$value_ )

  Saves entry.

  **Pameters:**

  * _key_

     The key of the entry to be inserted.

  * _value_

     The value to be inserted.

  **Return Values:**

  Returns TRUE on success or FALSE on failure.

---

* UnQLite\\Kvs::append — Append entry data specified by key

  **Description:**

  public bool **UnQLite\\Kvs::append** ( string _$key_ , string _$value_ )

  Append entry data specified by key.

  **Pameters:**

  * _key_

     The key of the entry to be appended.

  * _value_

     The value to be appended.

  **Return Values:**

  Returns TRUE on success or FALSE on failure.

---

* UnQLite\\Kvs::fetch — Fetch data specified by key

  **Description:**

  public string **UnQLite\\Kvs::fetch** ( string _$key_ )

  Fetch data specified by key.

  **Pameters:**

  * _key_

     The key of the entry to be inserted.

  **Return Values:**

  Returns the associated string if the key/data pair is found, FALSE otherwise.

---

* UnQLite\\Kvs::remove — Delete entry specified by key

  **Description:**

  public bool **UnQLite\\Kvs::remove** ( string _$key_ )

  Delete entry specified by key.

  alias: UnQLite\\Kvs::delete

  **Pameters:**

  * _key_

     The key of the entry to be appended.

  **Return Values:**

  Returns TRUE on success or FALSE on failure.

---

* UnQLite\\Kvs::begin — Manually begin a write-transaction

  **Description:**

  public bool **UnQLite\\Kvs::begin** ( void )

  Manually begin a write-transaction.

  **Return Values:**

  Returns TRUE on success or FALSE on failure.

---

* UnQLite\\Kvs::commit — Commit all changes to the database

  **Description:**

  public bool **UnQLite\\Kvs::commit** ( void )

  Commit all changes to the database.

  **Return Values:**

  Returns TRUE on success or FALSE on failure.

---

* UnQLite\\Kvs::rollback — Rollback a write-transaction

  **Description:**

  public bool **UnQLite\\Kvs::rollback** ( void )

  Rollback a write-transaction.

  **Return Values:**

  Returns TRUE on success or FALSE on failure.

---

* UnQLite\\Kvs::cursor — Create a new Key/Value stotre cursor

  **Description:**

  public object **UnQLite\\Kvs::cursor** ( [ long _$option_ = NULL ] )

  Create a new Key/Value stotre cursor.

  **Pameters:**

  * _option_

     options.

     * UnQLite\\CURSOR\_FIRST
     * UnQLite\\CURSOR\_LAST

  **Return Values:**

  Returns a new UnQLite\\Kvs\\Cursor object.

---

* UnQLite\\Kvs\\Cursor::\_\_construct — Create a new Key/Value stotre cursor.

  **Description:**

  public **UnQLite\\Kvs\\Cursor::\_\_construct** ( object _$kvs_ , [ long _$option_ ] )

  Create a new Key/Value stotre cursor.

  **Pameters:**

  * _kvs_

     UnQLite\\Kvs object.

  * _option_

     options.

     * UnQLite\\CURSOR\_FIRST
     * UnQLite\\CURSOR\_LAST

  **Return Values:**

  Returns a new UnQLite\\Kvs\\Cursor object.

---

* UnQLite\\Kvs\\Cursor::first — Advances the cursor to the first

  **Description:**

  public bool **UnQLite\\Kvs\\Cursor::first** ( void )

  Advances the cursor to the first.

  **Return Values:**

  Returns TRUE on success or FALSE on failure.

---

* UnQLite\\Kvs\\Cursor::last — Advances the cursor to the last

  **Description:**

  public bool **UnQLite\\Kvs\\Cursor::last** ( void )

  Advances the cursor to the last.

  **Return Values:**

  Returns TRUE on success or FALSE on failure.

---

* UnQLite\\Kvs\\Cursor::next — Advances the cursor to the next

  **Description:**

  public bool **UnQLite\\Kvs\\Cursor::next** ( void )

  This function has no parameters.

  **Return Values:**

  Returns TRUE on success or FALSE on failure.

---

* UnQLite\\Kvs\\Cursor::prev — Advances the cursor to the prev

  **Description:**

  public bool **UnQLite\\Kvs\\Cursor::prev** ( void )

  Advances the cursor to the prev.

  **Return Values:**

  Returns TRUE on success or FALSE on failure.

---

* UnQLite\\Kvs\\Cursor::seek — Seek cursor to key

  **Description:**

  public bool **UnQLite\\Kvs\\Cursor::seek** ( string _$key_ , [ long _$option_ = UnQLite\\CURSOR\_MATCH\_EXACT ] )

  Seek cursor to key.

  **Pameters:**

  * _key_

     The key of the entry to be seek.

  * _option_

     options.

     * UnQLite\\CURSOR\_MATCH\_EXACT
     * UnQLite\\CURSOR\_MATCH\_LE
     * UnQLite\\CURSOR\_MATCH\_GE

  **Return Values:**

  Returns TRUE on success or FALSE on failure.

---

* UnQLite\\Kvs\\Cursor::exists — Check whether cursor exists

  **Description:**

  public bool **UnQLite\\Kvs\\Cursor::exists** ( void )

  Check whether cursor exists.

  **Return Values:**

  Returns TRUE on success or FALSE on failure.

---

* UnQLite\\Kvs\\Cursor::remove — Delete the current cursor

  **Description:**

  public bool **UnQLite\\Kvs\\Cursor::remove** ( void )

  Delete the current cursor.

  alias: UnQLite\\Kvs\\Cursor::delete

  **Return Values:**

  Returns TRUE on success or FALSE on failure.

---

* UnQLite\\Kvs\\Cursor::key — Returns the current key

  **Description:**

  public string **UnQLite\\Kvs\\Cursor::key** ( void )

  Returns the current key.

  **Return Values:**

  The current cursor's key as a string.

---

* UnQLite\\Kvs\\Cursor::data — Returns the current value

  **Description:**

  public string **UnQLite\\Kvs\\Cursor::data** ( void )

  Returns the current value.

  **Return Values:**

  The current cursor's key as a string.

---

* UnQLite\\Doc::\_\_construct — Create a new document store

  **Description:**

  public **UnQLite\\Doc::\_\_construct** ( object _$db_ , string _$collection_ )

  Create a new document store.

  **Pameters:**

  * _db_

     UnQLite\\DB class object.

  * _collection_

     collection name.

  **Return Values:**

  Returns a new UnQLite\\Doc object.

---

* UnQLite\\Doc::eval — Evaluate a string as Jx9 code

  **Description:**

  public string **UnQLite\\Doc::eval** ( string _$code_ )

  Evaluate a string as Jx9 code.

  **Pameters:**

  * _code_

     Jx9 code.

  **Return Values:**

  Returns execution result on success or FALSE on failure.

---

* UnQLite\\Doc::drop — remove a collection

  **Description:**

  public bool **UnQLite\\Doc::drop** ( void )

  remove a collection.

  **Return Values:**

  Returns TRUE on success or FALSE on failure.

---

* UnQLite\\Doc::count — total number of inserted records

  **Description:**

  public int **UnQLite\\Doc::drop** ( void )

  total number of inserted records.

  **Return Values:**

  total nuber of records.

---

* UnQLite\\Doc::store — Store one or more JSON values

  **Description:**

  public bool **UnQLite\\Doc::store** ( array|object _$data_ )

  Store one or more JSON values.

  **Pameters:**

  * _data_

     JSON values.

  **Return Values:**

  Returns TRUE on success or FALSE on failure.

---

* UnQLite\\Doc::fetch — Fetch the current record

  **Description:**

  public mixed **UnQLite\\Doc::fetch** ( [ int _$offset_ = 0 ] )

  Fetch the current record.

  **Pameters:**

  * _offset_

     cursor offset.

  **Return Values:**

  record value.

---

* UnQLite\\Doc::fetch_all — Retrieve all records

  **Description:**

  public array **UnQLite\\Doc::fetch_all** ( void )

  Retrieve all records.

  alias: UnQLite\\Doc::fetchAll

  **Return Values:**

  JSON array holding the filtered records.

---

* UnQLite\\Doc::fetch_id — Fetch a record via its unique ID

  **Description:**

  public mixed **UnQLite\\Doc::fetch_id** ( int _$id_ )

  Fetch a record via its unique ID.

  alias: UnQLite\\Doc::fetchId

  **Pameters:**

  * _id_

     id number.

  **Return Values:**

  record value.

---

* UnQLite\\Doc::remove — remove a stored record

  **Description:**

  public bool **UnQLite\\Doc::remove** ( int _$id_ )

  remove a stored record.

  **Pameters:**

  * _id_

     id number.

  **Return Values:**

  Returns TRUE on success or FALSE on failure.

---

* UnQLite\\Doc::begin — Manually begin a write-transaction

  **Description:**

  public bool **UnQLite\\Doc::begin** ( void )

  Manually begin a write-transaction.

  **Return Values:**

  Returns TRUE on success or FALSE on failure.

---

* UnQLite\\Doc::commit — Commit all changes to the database

  **Description:**

  public bool **UnQLite\\Doc::commit** ( void )

  Commit all changes to the database.

  **Return Values:**

  Returns TRUE on success or FALSE on failure.

---

* UnQLite\\Doc::rollback — Rollback a write-transaction

  **Description:**

  public bool **UnQLite\\Doc::rollback** ( void )

  Rollback a write-transaction.

  **Return Values:**

  Returns TRUE on success or FALSE on failure.

---

## Examples

### Key/Value store

    namespace UnQLite;

    $db = new DB(dirname(__FILE__) . '/kvs.db');
    $kvs = $db->kvs(); // or $kvs = new Kvs($db);
    $kvs->store("foo", "bar");
    $kvs->fetch("foo"); // => bar

    $kvs->remove("foo"); // or $kvs->delete("foo")

    $kvs->store("a", "A");
    $kvs->store("b", "B");
    $kvs->store("c", "C");

    $cursor = $kvs->cursor();
    $cursor->first();
    do {
        $cursor->key();  // => a .. b  .. c
        $cursor->data(); // => A .. B  .. C
    } while ($cursor->next());

### Document store

    namespace UnQLite;

    $db = new DB(dirname(__FILE__) . '/doc.db');
    $doc = $db->doc('test'); // or $doc = new Doc($db, 'test');
    $doc->store(array("a" => "A"));
    $doc->store(array("b" => "B"));
    $doc->store(array("c" => "C"));
    $doc->fetch();
    // => array (
    //      [a] => A
    //      [__id] => 0
    //    )

    $doc->fetch_all(); // or $doc ->fetchAll()
    // =>
    // => array (
    //      [0] => array (
    //              [a] => A
    //              [__id] => 0
    //             )
    //      [1] => array (
    //              [__id] => 1
    //              [b] => B
    //             )
    //      [2] => array (
    //              [c] => C
    //              [__id] => 2
    //             )
    //    )

    $doc->fetch_id(1); // or $doc ->fetchId(1)
    // => bar
    // => array (
    //      [__id] => 1
    //      [b] => B
    //    )

    $doc->remove(1); // or $doc->delete(1)

    $doc->fetch_all(); // or $doc ->fetchAll()
    // => array (
    //      [0] => array (
    //              [a] => A
    //              [__id] => 0
    //             )
    //      [1] => array (
    //              [c] => C
    //              [__id] => 2
    //             )
    //    )

### Transaction

    namespace UnQLite;

    $db = new DB(dirname(__FILE__) . '/kvs.db');
    $kvs = $db->kvs(); // begin is performed implicitly
    $kvs->store("foo", "bar");

    $kvs->fetch("foo");  // => bar
    $kvs->fetch("test"); // => NULL

    $kvs->commit(); // transaction commit (write here)

    $kvs->begin(); // transaction start

    $kvs->store("test", "message");
    $kvs->remove("foo");

    $kvs->fetch("foo");  // => NULL
    $kvs->fetch("test"); // => message

    $kvs->rollback(); // transaction rollback

    $kvs->fetch("foo");  // => bar
    $kvs->fetch("test"); // => NULL

## Related

* [api document](http://api.at-ninja.jp/php-ext-unqlite/)

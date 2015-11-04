# iris-crypt

Encrypt Node.js modules into a single package file.

**Requires: Node.js version 0.12**

## Using

See [`test.js`](./tests/test.js) for usage example.

## Documentation

The module exports these functions:

  * `generateAuth()` - generate authorization key
  * `package()` - create encrypted package
  * `load()` - load encrypted package

### generateAuth(serial)

Create an authorization key string based on a `serial` number.
The serial number must be in a range [0..65535].

```
var serial = 1234;
var auth = irisCrypt.makeAuth(serial); // auth = 'XXXX-XXXX-XXXX-XXXX-YYYY-ZZZZ'
```

### package(auth, filename, files)

Create a single encrypted with `auth` key in a package file named as `filename`.

The package contents is set with `files` object, where each property in the
object is a module name and path pair:

```
irisCrypt.package(auth, 'some/where/filename.pkg', {
	'module1_name': 'path/to/module1',
	'module2_name': 'another/path/to/module2',
});
```

### load(auth, filename)

Load a package from a file named as `filename` and decrypt it with `auth`.

Returns a `Package` object with `require(name)` function wich loads a module from
the package.

```
var pkg = irisCrypt.load(auth, 'some/where/filename.pkg');

var module1 = pkg.require('module1_name');
// use exports from module1

var module2 = pkg.require('module2_name');
```

### Package.key

Public part of auth key that was used to create the package.
Read-only property.

```
var key = pkg.key; // 'YYYY-ZZZZ' part of auth
```

### Package.serial

The serial number that was used for the package auth key generation.
Read-only property.

```
var sn = pkg.serial; // 1234
```


### Package.names

An array of module names stored in the package.
Read-only property

```
var modules = pkg.names; // ['module1_name', 'module2_name']
```

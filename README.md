# iris-crypt

Store Node.js modules encrypted in a package file.

**Requires: Node.js version >=0.12**

## Building

Run `npm rebuild` to build native addon from the project sources. Additional
command line option `--target` allows to set specific Node.js version.

## Using

The module is indented to create an encrypted package with several Node.js modules
and to use these modules from the package file.

See [`encrypt.js`](./tests/test.js) and [`decrypt.js`](./tests/decrypt.js) for usage example.

## Documentation

The module consists of two binary parts exporting following functions:

  - Encrypt part (allowed only if `iris-encrypt.node` addon exists)
    * `generateAuth()` - generate authorization key
    * `package()` - create encrypted package

  - Decrypt part (in `iris-decrypt.node` addon)
    * `load()` - load encrypted package

Publisher encrypts a set of JavaScript files and modules into single package
file with generated authorization key.

Client loads the package with the supplied authorization key and use `require()`
function to load a particular module.

```
var irisCrypt = require('iris-crypt');
```

### generateAuth(password, serial)

Create an authorization key string based on a `password` string
and `serial` number. The serial number must be in a range [0..65535].

```
var password = 'zzz';
var serial = 1234;
var auth = irisCrypt.makeAuth(password, serial);
// assert(auth == 'EK4Z-3Z1E-SE4J-ANMZ-X390-917Z')
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
```

### Package.require(name)

Load a module stored in the package. This function fallbacks to original Node.js
`require()` function, if there is no such a module.

```
var module1 = pkg.require('module1_name');
// use exports from module1

var module2 = pkg.require('module2_name');

var fs = pkg.require('fs'); // load native Node.js module
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

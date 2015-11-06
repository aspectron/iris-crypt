var crypt = require('../');
var path = require('path');

console.log('crypt exports:', crypt);

var auth = process.argv[2];
var filename = 'test.pkg';

console.log('');
console.log('using auth: %s', auth);

var pkg = crypt.load(auth, filename);
console.log('loaded  package %s:', filename, pkg);
console.log('package %s key:', filename, pkg.key);
console.log('package %s serial:', filename, pkg.serial);
console.log('package %s names:', filename, pkg.names);

console.log('');
m1 = pkg.require('m1');
console.log('m1 exports:', m1);
console.log('m1.f():', m1.f());

console.log('');
m2 = pkg.require('m2');
console.log('m2 exports:', m2);
console.log('m2.f():', m2.f());

console.log('');
m3 = pkg.require('m3');
console.log('m3 exports:', m3);
console.log('m3.f():', m3.f());
console.log('m3.g():', m3.g());

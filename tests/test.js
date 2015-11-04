var crypt = require('../');
var path = require('path');

console.log('crypt exports:', crypt);

var serial = 1234;
if (process.argv.length > 2) serial = +process.argv[2];

var auth = crypt.generateAuth(serial);
var filename = 'test.pkg';

console.log('');
console.log('generated auth for serial %s: %s', serial, auth);

crypt.package(auth, filename, {
	'm1': path.join(__dirname, 'module1.js'),
	'm2': path.join(__dirname, 'module2.js'),
	'm3': path.join(__dirname, 'module3'),
});
console.log('created package %s', filename);

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

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

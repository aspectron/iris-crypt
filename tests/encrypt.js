var crypt = require('../');
var path = require('path');

console.log('crypt exports:', crypt);

var password = process.argv[2] || 'password';
var serial = +process.argv[3] || 1234;

var auth = crypt.generateAuth(password, serial);
var filename = 'test.pkg';

console.log('');
console.log('generated auth for serial %s: %s', serial, auth);

crypt.package(auth, filename, {
	'm1': path.join(__dirname, 'module1.js'),
	'm2': path.join(__dirname, 'module2.js'),
	'm3': path.join(__dirname, 'module3'),
});
console.log('created package %s', filename);

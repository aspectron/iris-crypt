//
// Copyright (c) 2015 ASPECTRON Inc.
// All Rights Reserved.
//
// This file is part of IrisCrypt (https://github.com/aspectron/iris-crypt) project.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE
//
var crypt = require('../');
var path = require('path');

console.log('crypt exports:', crypt);

var password = process.argv[2] || 'password';
var serial = +process.argv[3] || 1234;
var filename = process.argv[4] || 'test.pkg';

var auth = crypt.generateAuth(password, serial);
console.log('');
console.log('generated auth for serial %s: %s', serial, auth);

crypt.package(auth, filename, {
	'm1': path.join(__dirname, 'module1.js'),
	'm2': path.join(__dirname, 'module2.js'),
	'm3': path.join(__dirname, 'module3'),
});
console.log('');
console.log('created package %s', filename);

var pkg = crypt.load(auth, filename);
console.log('');
console.log('loaded package %s:', filename, pkg);
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

//
// Copyright (c) 2015 ASPECTRON Inc.
// All Rights Reserved.
//
// This file is part of IrisCrypt (https://github.com/aspectron/iris-crypt) project.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE
//
var path = require('path');

var utils = require("./utils");
var lib = require(path.join(__dirname, 'lib'));

exports.version = require('../package.json').version;
exports.name = require('../package.json').name;
exports.description = require('../package.json').description;
exports.f = function() { return "module3"; }
exports.g = utils.f;
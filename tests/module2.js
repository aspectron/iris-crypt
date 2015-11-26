//
// Copyright (c) 2015 ASPECTRON Inc.
// All Rights Reserved.
//
// This file is part of IrisCrypt (https://github.com/aspectron/iris-crypt) project.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE
//
require('console');
var m1 = require('m1');

exports.f = function() { return m1.f() + " in module2"; }
var path = require('path');

var utils = require("./utils");
var lib = require(path.join(__dirname, 'lib'));

exports.version = require('../package.json').version;
exports.name = require('../package.json').name;
exports.description = require('../package.json').description;
exports.f = function() { return "module3"; }
exports.g = utils.f;
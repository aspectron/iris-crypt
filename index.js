module.exports = require('./bin/' + process.platform + '/' + process.arch + '/iris-decrypt.node');
try { encrypt = require('./bin/' + process.platform + '/' + process.arch + '/iris-encrypt.node') } catch (e) { return; }
module.exports.generateAuth = encrypt.generateAuth;
module.exports.package = encrypt.package;

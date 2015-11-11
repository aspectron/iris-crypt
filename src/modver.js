var fs = require('fs');
var path = require('path');

var nodedir = process.argv[2];

function try_read(names)
{
	for (var i = 0; i < names.length; ++i)
	{
		try { return fs.readFileSync(names[i], { encoding: 'utf8'}) }
		catch (e) {}
	}
	throw new Error("can't read node_version.h in " + nodedir);
}

var source = try_read([
	path.join(nodedir, 'include/node/node_version.h'),
	path.join(nodedir, 'src/node_version.h')
]);

var version = source.match(/^#define\s+NODE_MODULE_VERSION\s+(\d+)/m);
if (!version) throw new Error("can't read #define NODE_MODULE_VERSION in node_version.h in " + nodedir);
console.log(version[1]);

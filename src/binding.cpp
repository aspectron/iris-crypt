#include "package.hpp"

#include <node.h>

#include <v8pp/module.hpp>
#include <v8pp/class.hpp>
#include <v8pp/property.hpp>
#include <v8pp/object.hpp>
#include <v8pp/call_v8.hpp>

static void init(v8::Handle<v8::Object>, v8::Handle<v8::Object> module)
{
	v8::Isolate* isolate = v8::Isolate::GetCurrent();
	v8::HandleScope scope(isolate);

	// store persistent handle to this module object
	package::node_module.Reset(isolate, module);
	v8::Local<v8::Function> require;

	// get original Node.js require() function
	v8pp::get_option(isolate, module, "require", require);
	package::node_require.Reset(isolate, require);

	// load Node.js crypto module
	v8::Local<v8::Object> crypto = v8pp::call_v8(isolate, require, module, "crypto").As<v8::Object>();
	if (crypto.IsEmpty() || crypto->IsUndefined())
	{
		v8pp::set_option(isolate, module, "exports", v8pp::throw_ex(isolate, "can't load crypto module"));
		return;
	}

	package::node_crypto.Reset(isolate, crypto);

	v8pp::class_<package> package_class(isolate);
	package_class
		.set("require", &package::require)
		.set("serial", v8pp::property(&package::serial))
		.set("names", v8pp::property(&package::names))
		;
	v8pp::module exports(isolate);
	exports
		.set("Package", package_class)
		.set("generateAuth", package::gen_auth)
		.set("package", package::make)
		.set("load", package::load)
		;

	v8pp::set_option(isolate, module, "exports", exports.new_instance());

	node::AtExit([](void*)
	{
		package::node_crypto.Reset();
		package::node_require.Reset();
		package::node_module.Reset();
	});
}

NODE_MODULE(lmdb_node, init);

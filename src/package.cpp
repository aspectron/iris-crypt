//
// Copyright (c) 2015 ASPECTRON Inc.
// All Rights Reserved.
//
// This file is part of IrisCrypt (https://github.com/aspectron/iris-crypt) project.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE
//
#include "package.hpp"
#include "base32.hpp"

#include <algorithm>
#include <iterator>
#include <numeric>

#pragma warning(push, 3)
#include <node_buffer.h>

#include <v8pp/class.hpp>
#include <v8pp/object.hpp>
#include <v8pp/call_v8.hpp>

#include <yas/binary_iarchive.hpp>
#include <yas/binary_oarchive.hpp>
#include <yas/mem_streams.hpp>
#include <yas/file_streams.hpp>
#include <yas/serializers/std_types_serializers.hpp>
#pragma warning(pop)

static uint32_t const SIGN = 0x30504349; // ICP0

v8::UniquePersistent<v8::Object> package::node_module;
v8::UniquePersistent<v8::Function> package::node_require;
v8::UniquePersistent<v8::Object> package::node_crypto;

static std::string pbkdf2(v8::Isolate* isolate, std::string const& password, std::string const& salt,
	size_t iterations, size_t keylen, char const* digest = "sha256")
{
	v8::HandleScope scope(isolate);

	v8::Local<v8::Object> crypto = v8pp::to_local(isolate, package::node_crypto);

	v8::Local<v8::Function> fun;
	v8pp::get_option(isolate, crypto, "pbkdf2Sync", fun);

	v8::Local<v8::Value> buf = v8pp::call_v8(isolate, fun, crypto, password, salt, iterations, keylen, digest);

	return std::string(node::Buffer::Data(buf), node::Buffer::Length(buf));
}

static std::string random_bytes(v8::Isolate* isolate, size_t size)
{
	v8::HandleScope scope(isolate);

	v8::Local<v8::Object> crypto = v8pp::to_local(isolate, package::node_crypto);

	v8::Local<v8::Function> fun;
	v8pp::get_option(isolate, crypto, "randomBytes", fun);

	v8::Local<v8::Value> buf = v8pp::call_v8(isolate, fun, crypto, size);

	return std::string(node::Buffer::Data(buf), node::Buffer::Length(buf));
}

static v8::Local<v8::Value> use_buffer(v8::Isolate* isolate, char const* data, size_t size)
{
#if NODE_MAJOR_VERSION < 3
	return node::Buffer::New(isolate, data, size);
#else
	// MaybeLocal has appeared in io.js version 3.0.0
	return node::Buffer::New(isolate, const_cast<char*>(data), size,
		[](char* data, void* hint){}, nullptr).ToLocalChecked();
#endif
}

static yas::shared_buffer do_crypt(v8::Isolate* isolate, char const* name, bool encrypt,
	std::string const& key, std::string const& iv,
	yas::shared_buffer* auth_tag, char const* data, size_t size)
{
	char const* const create_name = (encrypt ? "createCipheriv" : "createDecipheriv");

	v8::HandleScope scope(isolate);

	v8::Local<v8::Object> crypto = v8pp::to_local(isolate, package::node_crypto);

	v8::Local<v8::Function> create;
	v8pp::get_option(isolate, crypto, create_name, create);

	v8::Local<v8::Object> cipher = v8pp::call_v8(isolate, create, crypto, name,
		use_buffer(isolate, key.data(), key.size()), use_buffer(isolate, iv.data(), iv.size())).As<v8::Object>();
	if (!encrypt && auth_tag)
	{
		v8::Local<v8::Function> set_auth_tag;
		v8pp::get_option(isolate, cipher, "setAuthTag", set_auth_tag);
		v8pp::call_v8(isolate, set_auth_tag, cipher, use_buffer(isolate, auth_tag->data.get(), auth_tag->size));
	}
	v8::Local<v8::Function> update, final;
	v8pp::get_option(isolate, cipher, "update", update);
	v8pp::get_option(isolate, cipher, "final", final);

	v8::Local<v8::Value> buf1 = v8pp::call_v8(isolate, update, cipher, use_buffer(isolate, data, size));
	v8::Local<v8::Value> buf2 = v8pp::call_v8(isolate, final, cipher);
	if (buf1.IsEmpty() || buf2.IsEmpty())
	{
		throw std::runtime_error(encrypt? "encryption failed" : "decryption failed");
	}

	if (encrypt && auth_tag)
	{
		v8::Local<v8::Function> get_auth_tag;
		v8pp::get_option(isolate, cipher, "getAuthTag", get_auth_tag);
		v8::Local<v8::Value> tag_buf = v8pp::call_v8(isolate, get_auth_tag, cipher);
		auth_tag->assign(node::Buffer::Data(tag_buf), node::Buffer::Length(tag_buf));
	}
	char const* const buf1_data = node::Buffer::Data(buf1);
	char const* const buf2_data = node::Buffer::Data(buf2);
	size_t const buf1_size = node::Buffer::Length(buf1);
	size_t const buf2_size = node::Buffer::Length(buf2);

	yas::shared_buffer result(buf1_size + buf2_size);
	std::copy(buf1_data, buf1_data + buf1_size, result.data.get());
	std::copy(buf2_data, buf2_data + buf2_size, result.data.get() + buf1_size);

	return result;
}

static yas::shared_buffer encrypt(v8::Isolate* isolate, std::string const& key, std::string& iv,
	yas::shared_buffer& auth_tag, char const* data, size_t size)
{
	assert(key.size() == 128 / 8);
	iv = random_bytes(isolate, 96 / 8);
	return do_crypt(isolate, "aes-128-gcm", true, key, iv, &auth_tag, data, size);
}

static yas::shared_buffer decrypt(v8::Isolate* isolate, std::string const& key, std::string const& iv,
	yas::shared_buffer& auth_tag, char const* data, size_t size)
{
	assert(key.size() == 128 / 8);
	assert(iv.size() == 96 / 8);
	return do_crypt(isolate, "aes-128-gcm", false, key, iv, &auth_tag, data, size);
}

// Auth key like XXXX-XXXX-XXXX-XXXX-XXXX-XXXX-YYYY-ZZZZ
class auth_data
{
	// result string is in Base32, so binary data should be
	// a multipler for 40 bit block.
	static size_t const KEY_LEN = 16;
	static size_t const AUTH_LEN = KEY_LEN + sizeof(uint16_t) * 2; // + serial + checksum
	std::string data_;
public:
	auth_data(v8::Isolate* isolate, std::string const& password, uint16_t serial_number)
	{
		std::string const salt((char*)&serial_number, sizeof(serial_number));
		// XXXX
		data_ = pbkdf2(isolate, password, salt, 1000, KEY_LEN);
		// YYYY
		data_.append(salt);
		// ZZZZ
		uint16_t const checksum = std::accumulate(data_.begin(), data_.end(), uint16_t{});
		data_.append((char*)&checksum, sizeof(checksum));
	}

	explicit auth_data(std::string str)
	{
		str.erase(std::remove(str.begin(), str.end(), '-'), str.end());
		try { data_ = base32::decode<base32::crockford>(str); }
		catch (std::exception const&) {};

		if (data_.size() != AUTH_LEN
			|| checksum() != std::accumulate(data_.begin(), data_.end() - sizeof(uint16_t), uint16_t{}))
		{
			throw std::runtime_error("invalid auth data");
		}
	}

	std::string to_string(size_t const group_by = 4) const
	{
		std::string result = base32::encode<base32::crockford>(data_);
		if (group_by)
		{
			for (auto pos = result.begin() + group_by; pos < result.end();)
			{
				pos = result.insert(pos, '-') + group_by + 1;
			}
		}
		return result;
	}

	uint16_t serial_number() const
	{
		uint16_t result;
		memcpy(&result, data_.data() + KEY_LEN, sizeof(result));
		return result;
	}

	uint16_t checksum() const
	{
		uint16_t result;
		memcpy(&result, data_.data() + KEY_LEN + sizeof(uint16_t), sizeof(result));
		return result;
	}

	std::string pub_data() const { return data_.substr(KEY_LEN); }
	std::string priv_key() const { return data_.substr(0, KEY_LEN); }
};

void package::gen_auth(v8::FunctionCallbackInfo<v8::Value> const& args)
{
	v8::Isolate* isolate = args.GetIsolate();

	std::string const password = v8pp::from_v8<std::string>(isolate, args[0]);
	uint16_t const serial = v8pp::from_v8<uint16_t>(isolate, args[1]);

	auth_data const auth(isolate, password, serial);

	args.GetReturnValue().Set(v8pp::to_v8(isolate, auth.to_string()));
}

void package::make(v8::FunctionCallbackInfo<v8::Value> const& args)
try
{
	v8::Isolate* isolate = args.GetIsolate();
	v8::HandleScope scope(isolate);

	using string_map = std::map<std::string, std::string>;

	auth_data const auth(v8pp::from_v8<std::string>(isolate, args[0]));
	auto const filename = v8pp::from_v8<std::string>(isolate, args[1]);
	auto const files = v8pp::from_v8<string_map>(isolate, args[2]);

	sources_map sources;
	modules_map modules;
	for (auto const& file : files)
	{
		std::string const& id = file.first;
		path const p = file.second;
		p.is_dir() ? load_dir(isolate, modules, sources, id, p) : load_file(modules, sources, id, p);
	}

	yas::mem_ostream mem(sources.size() * 40 * 1024);
	yas::binary_oarchive<yas::mem_ostream> content(mem, yas::no_header);
	content.serialize(modules, sources);

	yas::intrusive_buffer const buf = mem.get_intrusive_buffer();
	yas::shared_buffer auth_tag;
	std::string iv;
	yas::shared_buffer const cipher = encrypt(isolate, auth.priv_key(), iv, auth_tag, buf.data, buf.size);

	yas::file_ostream file(filename.c_str(), yas::file_trunc);
	yas::binary_oarchive<yas::file_ostream> out(file, yas::no_header);
	out.serialize(SIGN, auth.pub_data(), iv, auth_tag, cipher);
}
catch (yas::io_exception const& ex)
{
	throw std::runtime_error(std::string("Package write error: ") + ex.what());
}

void package::load(v8::FunctionCallbackInfo<v8::Value> const& args)
try
{
	v8::Isolate* isolate = args.GetIsolate();

	auth_data const auth(v8pp::from_v8<std::string>(isolate, args[0]));
	auto const filename = v8pp::from_v8<std::string>(isolate, args[1]);

	std::decay<decltype(SIGN)>::type sign;
	std::string pub_data, iv;
	yas::shared_buffer auth_tag, cipher;

	yas::file_istream file(filename.c_str());
	yas::binary_iarchive<yas::file_istream> in(file, yas::no_header);
	in.serialize(sign, pub_data, iv, auth_tag, cipher);
	if (sign != SIGN)
	{
		throw std::runtime_error("Package invalid format");
	}
	if (pub_data != auth.pub_data())
	{
		throw std::runtime_error("Package invalid key");
	}

	std::unique_ptr<package> pkg(new package);
	pkg->serial_number_ = auth.serial_number();

	yas::shared_buffer const plain = decrypt(isolate, auth.priv_key(), iv, auth_tag, cipher.data.get(), cipher.size);
	yas::mem_istream mem(plain);
	yas::binary_iarchive<yas::mem_istream> content(mem, yas::no_header);
	content.serialize(pkg->modules_, pkg->sources_);

	pkg->js_modules_.Reset(isolate, v8::Object::New(isolate));
	v8::Local<v8::Object> result = v8pp::class_<package>::import_external(isolate, pkg.release());

	args.GetReturnValue().Set(result);
}
catch (yas::io_exception const& ex)
{
	throw std::runtime_error(std::string("Package read error: ") + ex.what());
}

std::vector<std::string> package::names() const
{
	std::vector<std::string> result;
	for (auto const& kv : modules_)
	{
		result.emplace_back(kv.first);
	}
	std::sort(result.begin(), result.end());
	return result;
}

v8::Local<v8::Value> package::require_module(v8::Isolate* isolate, std::string const& id, path const& file, std::string const& source)
{
	// re-define require() function in a wrapped source
	// because for some reason V8 can't reference it
	// from this package object prototype
	char const wrapper_begin[] = 
		"(function (exports, module, __filename, __dirname){"
		"var require = function(name) { return module.require(name) };";
	char const wrapper_end[] = "\n});";

	// wrap module.source into JavaScript (function(){}) to hide the module source code
	std::string wrapped_source;
	wrapped_source.reserve(source.length() + sizeof(wrapper_begin) + sizeof(wrapper_end) - 2);
	wrapped_source.append(wrapper_begin, sizeof(wrapper_begin) - 1);
	wrapped_source.append(source);
	wrapped_source.append(wrapper_end, sizeof(wrapper_end) - 1);

	v8::TryCatch try_catch;

	// compile and run wrapped source to get a wrapped JS function
	v8::ScriptOrigin origin(v8pp::to_v8(isolate, id));
	v8::Local<v8::Script> script = v8::Script::Compile(v8pp::to_v8(isolate, wrapped_source), &origin);
	if (try_catch.HasCaught())
	{
		try_catch.ReThrow();
		return v8::Undefined(isolate);
	}
	v8::Local<v8::Function> wrapped_script = script->Run().As<v8::Function>();
	if (try_catch.HasCaught())
	{
		try_catch.ReThrow();
		return v8::Undefined(isolate);
	}

	// create a module object and set it protoptype to this package
	v8::Local<v8::Object> js_module = v8::Object::New(isolate);
	js_module->SetPrototype(v8pp::to_v8(isolate, this));

	// setup the module object
	v8::Local<v8::Object> exports = v8::Object::New(isolate);
	v8pp::set_option(isolate, js_module, "exports", exports);
	v8pp::set_const(isolate, js_module, "id", id);
	v8pp::set_const(isolate, js_module, "filename", file);
	v8pp::set_option(isolate, js_module, "loaded", false);

	// call wrapped function
	v8pp::call_v8(isolate, wrapped_script, js_module, exports,js_module, file, file.parent());
	if (try_catch.HasCaught())
	{
		try_catch.ReThrow();
		return v8::Undefined(isolate);
	}

	// get exports object
	v8pp::set_option(isolate, js_module, "loaded", true);
	v8pp::get_option(isolate, js_module, "exports", exports);
	return exports;
}

v8::Local<v8::Value> package::require_original(v8::Isolate* isolate, std::string const& id)
{
	// load node module using original require function
	v8::Local<v8::Object> module = v8pp::to_local(isolate, node_module);
	v8::Local<v8::Function> require = v8pp::to_local(isolate, node_require);
	v8::TryCatch try_catch;
	v8::Local<v8::Value> result = v8pp::call_v8(isolate, require, module, id);
	if (try_catch.HasCaught())
	{
		return try_catch.ReThrow();
	}
	return result;
}

void package::require(v8::FunctionCallbackInfo<v8::Value> const& args)
{
	v8::Isolate* isolate = args.GetIsolate();

	std::string const id = v8pp::from_v8<std::string>(isolate, args[0], "");
	if (id.empty())
	{
		throw std::runtime_error("name argument empty");
	}

	path name;
	auto it = modules_.find(id);
	if (it != modules_.end())
	{
		name = it->second;
	}
	else
	{
		name = (id[0] == '.' && !require_dir_stack_.empty()? require_dir_stack_.top() / id : id);
		name.add_extension(".js");
	}

	v8::EscapableHandleScope scope(isolate);

	v8::Local<v8::Object> js_modules = v8pp::to_local(isolate, js_modules_);
	v8::Local<v8::String> js_name = v8pp::to_v8(isolate, name);
	v8::Local<v8::Value> js_module = js_modules->Get(js_name);
	if (js_module.IsEmpty() || js_module->IsUndefined())
	{
		auto src = sources_.find(name);
		if (src == sources_.end())
		{
			args.GetReturnValue().Set(require_original(isolate, id));
			return;
		}
		std::string& source = src->second;

		if (name.extension() == ".json")
		{
			js_module = v8::JSON::Parse(v8pp::to_v8(isolate, source));
		}
		else
		{
			require_dir_stack_.push(name.parent());
			js_module = require_module(isolate, id, name, source);
			require_dir_stack_.pop();
		}
		source.clear();
		source.shrink_to_fit();
		js_modules->Set(js_name, js_module);
	}
	args.GetReturnValue().Set(scope.Escape(js_module));
}

void package::load_dir(v8::Isolate* isolate, modules_map& modules, sources_map& sources, std::string const& id, path const& p)
{
	path const base = p.parent();
	path main = p / "index.js";

	for (path const& file : p.list_files())
	{
		std::string const content = file.content();
		if (file.relative_to(p) == "package.json")
		{
			v8::Local<v8::Object> json = v8::JSON::Parse(v8pp::to_v8(isolate, content)).As<v8::Object>();
			if (json.IsEmpty() || !json->IsObject())
			{
				throw std::runtime_error(id + ": can't load " + file.str());
			}
			if (!v8pp::get_option(isolate, json, "main", main))
			{
				throw std::runtime_error(id + ": no \"main\" in " + file.str());
			}
			main = p / main;
			main.add_extension(".js");
		}
		sources.emplace(file.relative_to(base), std::move(content));
	}
	modules.emplace(id, p.base() / main.relative_to(p));
}

void package::load_file(modules_map& modules, sources_map& sources, std::string const& id, path const& p)
{
	sources.emplace(p.base(), p.content());
	modules.emplace(id, p.base());
}

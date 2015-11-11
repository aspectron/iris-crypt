#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <tuple>
#include <stack>

#include <v8.h>

#include "path.hpp"

class package
{
public:
	static v8::UniquePersistent<v8::Object> node_module;
	static v8::UniquePersistent<v8::Function> node_require;
	static v8::UniquePersistent<v8::Object> node_crypto;

	static void gen_auth(v8::FunctionCallbackInfo<v8::Value> const& args);
	static void make(v8::FunctionCallbackInfo<v8::Value> const& args);
	static void load(v8::FunctionCallbackInfo<v8::Value> const& args);

	void require(v8::FunctionCallbackInfo<v8::Value> const& args);

	uint16_t serial() const { return serial_number_; }

	std::vector<std::string> names() const;

private:
	uint16_t serial_number_;

	v8::UniquePersistent<v8::Object> js_modules_;

	using sources_map = std::unordered_map<path, std::string>;
	sources_map sources_;

	using modules_map = std::unordered_map<std::string, path>;
	modules_map modules_;

	std::stack<path> require_dir_stack_;

	static void load_dir(v8::Isolate* isolate, modules_map& modules, sources_map& sources, std::string const& id, path const& p);
	static void load_file(modules_map& modules, sources_map& sources, std::string const& id, path const& p);

	v8::Local<v8::Value> require_module(v8::Isolate* isolate, std::string const& id,
		path const& file, std::string const& source);
	v8::Local<v8::Value> require_original(v8::Isolate* isolate, std::string const& id);
};

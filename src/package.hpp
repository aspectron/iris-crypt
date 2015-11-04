#pragma once

#include <string>
#include <vector>
#include <map>
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

	std::string const& key() const { return pub_key_; }
	uint16_t serial() const { return serial_number_; }

	std::vector<std::string> names() const;

private:
	struct module_name
	{
		std::string id;
		path dir, file;

		module_name() = default;

		bool is_root() const { return id[0] != '.'; }

		bool operator<(module_name const& rhs) const
		{
			return std::tie(id, dir.str()) < std::tie(rhs.id, rhs.dir.str());
		}

		template<typename Archive>
		void serialize(Archive& ar)
		{
			ar & id & dir & file;
		}
	};

	std::string pub_key_;
	uint16_t serial_number_;
	v8::UniquePersistent<v8::Object> js_modules_;

	using modules_map = std::map<module_name, std::string>;
	modules_map modules_;
	std::stack<path> require_dir_stack_;

	static void load_file(v8::Isolate* isolate, modules_map& modules, std::string const& id, path p, path base);

	v8::Local<v8::Value> require_module(v8::Isolate* isolate, module_name const& name, std::string const& source);
	v8::Local<v8::Value> require_original(v8::Isolate* isolate, std::string const& name);
};

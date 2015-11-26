//
// Copyright (c) 2015 ASPECTRON Inc.
// All Rights Reserved.
//
// This file is part of IrisCrypt (https://github.com/aspectron/iris-crypt) project.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE
//
#pragma once

#include <string>
#include <vector>
#include <utility>

#include <v8pp/convert.hpp>

class path
{
public:
	path() = default;
	path(std::string const& str);
	path(char const* str);

	bool operator==(path const& rhs) const { return str_ == rhs.str_; }
	bool operator!=(path const& rhs) const { return str_ != rhs.str_; }

	template<typename Archive>
	void serialize(Archive& ar) { ar & str_; }

	bool is_dir() const;
	bool is_file() const;

	bool empty() const { return str_.empty(); }
	void clear() { str_.clear(); }

	std::string const& str() const { return str_; }
	char const* c_str() const { return str_.c_str(); }

	path operator/(path const right) const;

	std::pair<path, path> parts() const;
	path parent() const { return parts().first; }
	path base() const { return parts().second; }

	std::string extension() const;
	void add_extension(std::string const& ext);

	path relative_to(path const& base) const;

	std::vector<path> list_files() const;
	std::string content() const;
	static path current();
private:
	void normalize();

	static const char sep;
	std::string str_;
};

namespace std {

template<>
struct hash<path> : hash<std::string>
{
	result_type operator()(path const& arg) const
	{
		return hash<std::string>::operator()(arg.str());
	}
};

} // std

namespace v8pp {

template<>
struct convert<path>
{
	using from_type = path;
	using to_type = v8::Handle<v8::String>;

	static bool is_valid(v8::Isolate*, v8::Handle<v8::Value> value)
	{
		return !value.IsEmpty() && value->IsString();
	}

	static from_type from_v8(v8::Isolate* isolate, v8::Handle<v8::Value> value)
	{
		if (!is_valid(isolate, value))
		{
			throw std::invalid_argument("expected path string");
		}
		return path(convert<std::string>::from_v8(isolate, value));
	}

	static to_type to_v8(v8::Isolate* isolate, path const& value)
	{
		return convert<std::string>::to_v8(isolate, value.str());
	}
};

} // namespace v8pp

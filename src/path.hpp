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
	void set_extension(std::string const& ext);

	path relative_to(path const& base) const;

	static path current();
private:
	void normalize();

	static const char sep;
	std::string str_;
};

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

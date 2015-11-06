#include "path.hpp"

#include <sys/stat.h>
#include <sys/types.h>

#include <cassert>
#include <algorithm>
#include <vector>
#include <fstream>

#ifdef _WIN32
#include <io.h>
#include <direct.h>
static char const path_sep = '\\';
#define get_current_dir_name() _getcwd(nullptr, 0)
#else
#include <unistd.h>
#include <dirent.h>
static char const path_sep = '/';
#endif

using std::string;
using strings = std::vector<std::string>;

static strings split(string const& str)
{
	strings result;
	string::size_type start = 0, end = 0;
	while ((end = str.find(path_sep, start)) != str.npos)
	{
		if (end != start) result.emplace_back(str.substr(start, end - start));
		start = end + 1;
	}
	if (end != start) result.emplace_back(str.substr(start, end - start));
	return result;
}

template<typename It>
static string join(It begin, It end)
{
	string result;
	for (; begin != end; ++begin)
	{
		if (!result.empty()) result += path_sep;
		result += *begin;
	}
	return result;
}

static string join(strings const& parts)
{
	return join(parts.begin(), parts.end());
}

path::path(string const& str)
	: str_(str)
{
	normalize();
}

path::path(char const* str)
	: str_(str ? str : "")
{
	normalize();
}

bool path::is_dir() const
{
	struct stat s;
	return stat(str_.c_str(), &s) == 0 && (s.st_mode & S_IFDIR);
}

bool path::is_file() const
{
	struct stat s;
	return stat(str_.c_str(), &s) == 0 && (s.st_mode & S_IFREG);
}

void path::normalize()
{
#ifdef _WIN32
	std::replace(str_.begin(), str_.end(), '/', '\\');
#endif

	strings parts;
	for (string const& part : split(str_))
	{
		if (part == "." || part.empty()) continue;
		if (part == ".." && !parts.empty())
		{
			parts.pop_back();
			continue;
		}
		parts.emplace_back(part);
	}
	str_ = join(parts);
}

std::pair<path, path> path::parts() const
{
	string::size_type pos = str_.find_last_of(path_sep);
	return pos == string::npos? std::make_pair("", str_)
		: std::make_pair(str_.substr(0, pos), str_.substr(pos + 1));
}

std::string path::extension() const
{
	string const base_str = base().str();
	string::size_type pos = base_str.find_last_of('.');
	return pos == string::npos? "" : base_str.substr(pos);
}

void path::add_extension(std::string const& ext)
{
	std::pair<path, path> p = parts();
	string& base = p.second.str_;
	if (!base.empty() && base.find_last_of('.') == string::npos)
	{
		str_ = p.first.str_ + path_sep + base + ext;
	}
}

path path::relative_to(path const& base) const
{
	strings const bparts = split(base.str_);
	strings const parts = split(str_);

	size_t same_size = std::min(bparts.size(), parts.size());
	for (size_t i = 0; i < same_size; ++i)
	{
		if (bparts[i] != parts[i])
		{
			same_size = i;
			break;
		}
	}

	strings result(same_size > bparts.size() ? same_size - bparts.size() : 0, "..");
	result.insert(result.end(), parts.begin() + same_size, parts.end());
	return path(join(result));
}

std::vector<path> path::list_files() const
{
	assert(is_dir());
	std::vector<path> result;
#ifdef _WIN32
	std::string const filename = str_ + path_sep + '*';
	_finddata_t data;
	intptr_t find = _findfirst(filename.c_str(), &data);
	if (find != -1)
	{
		do
		{
			if (strncmp(data.name, ".", 1) == 0 || strncmp(data.name, "..", 2) == 0) continue;
			std::string const name = str_ + path_sep + data.name;
			if (data.attrib & _A_SUBDIR)
			{
				std::vector<path> const sub = path(name).list_files();
				result.insert(result.end(), sub.begin(), sub.end());
			}
			else result.emplace_back(name);
		}
		while (_findnext(find, &data) == 0);
		_findclose(find);
	}
#else
	DIR* dir = opendir(str_.c_str());
	if (dir)
	{
		while (struct dirent* entry = readdir(dir))
		{
			result.emplace_back(entry->d_name);
		}
		closedir(dir);
	}
#endif
	return result;
}

std::string path::content() const
{
	assert(is_file());
	std::ifstream file(str_.c_str());
	if (!file.is_open())
	{
		throw std::runtime_error("can't open " + str_);
	}
	std::istreambuf_iterator<char> begin(file), end;
	return std::string(begin, end);
}

path path::current()
{
	char* buf = get_current_dir_name();
	path const result(buf);
	free(buf);
	return result;
}

path path::operator/(path const right) const
{
	return path(str_ + path_sep + right.str_);
}
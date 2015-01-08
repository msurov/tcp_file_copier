#ifndef __common_h__
#define __common_h__

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sstream>
#include <thread>
#include <stdexcept>

#include "err.h"


/*
 * unsorted functions are at here
 */

#define throw_if(condition, exception) \
	if (condition) \
	{ \
		throw exception; \
	}

inline std::string trim(std::string const& str, std::string const& symbols = " \t")
{
    auto const begin = str.find_first_not_of(symbols);

    if (begin == std::string::npos)
        return "";

    auto const end = str.find_last_not_of(symbols);
    auto const range = end - begin + 1;

    return str.substr(begin, range);
}

inline std::string get_thread_id()
{
	std::stringstream ss;
	ss << "[thread " << std::this_thread::get_id() << "]";
	return ss.str();
}

inline std::string fix_name(std::string const& s)
{
	std::string trimmed = trim(s, " \t\"'");
	throw_if(
		trimmed.find("\\/<>|\"':?*") != std::string::npos, 
		std::runtime_error("file name is incorrect")
	);
	return trimmed;
}

inline std::string strip_name(std::string const& path)
{
	auto slash = path.find_last_of('/');
	if (slash == std::string::npos)
		return path;

	return path.substr(++ slash);
}

inline uint64_t get_file_size(std::string const& path)
{
	struct stat64 s;
	int res = stat64(path.c_str(), &s);
	throw_if(res < 0, std::runtime_error("can't query size of file " + path));
	return s.st_size;
}

template <typename t>
inline std::string to_hexstr(t val)
{
	std::stringstream ss;
	ss.setf(std::ios_base::hex, std::ios_base::basefield);
	ss << val;
	return ss.str();
}

inline bool check_file_for_reading(std::string const& name)
{
	FILE* f = fopen(name.c_str(), "rb");
	if (!f)
		return false;

	fclose(f);
	return true;
}

inline bool isdir(std::string const& path)
{
	struct stat st;
	int res = stat(path.c_str(), &st);
	return res == 0 && S_ISDIR(st.st_mode);
}

inline bool check_dir_write_access(std::string const& path)
{
	return access(path.c_str(), W_OK) == 0;
}

#endif //__common_h__ 

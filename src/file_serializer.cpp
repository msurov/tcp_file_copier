#include <fstream>
#include <algorithm>
#include <assert.h>

#include "file_serializer.h"
#include "common.h"


/*
 * imp of file_deserializer
 */

file_deserializer::file_deserializer(std::string const& base_path) : base_path(base_path)
{
}

file_deserializer::~file_deserializer()
{
}

serializer_header file_deserializer::parse_header(serializer_buffer const& buf)
{
	std::vector<std::string> header;

	auto cur = buf.begin();
	auto end = std::find(cur, buf.end(), '\0');

	while (true)
	{
		auto next = std::find(cur, end, '\n');
		if (next == end)
			break;

		header.push_back(std::string(cur, std::distance(cur, next)));
		cur = ++next;
	}

	throw_if(header.size() < 2, 
		std::runtime_error("invalid format of the receiving message"));

	serializer_header res;

	res.file_size = std::stoull(header[0], nullptr, 16);
	res.file_name = fix_name(header[1]);
	res.data_offset = std::distance(buf.begin(), end) + 1;

	return res;
}

file_serializer::file_serializer(std::string const& filepath) : filepath(filepath)
{
}

file_serializer::~file_serializer()
{
}

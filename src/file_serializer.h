#include <iostream>
#include <array>
#include "common.h"
#include "trace.h"


/*
 * serialized data looks like: 
 *  	FILE_HEADER \0 FILE_DATA
 * 
 * where HEADER contains the at least 2 necessary members
 * 		FILE_SIZE is a hex string representation of uint64_t number
 * 		FILE_NAME is a string
 * 
 * all the members in header are represented as UTF8 strings
 * and separated by '\n' symbol
 *
 * maximum size of the HEADER is ('buffer_size' - 1)
 *
 */

/*
 * deserializer
 */

struct serializer_header
{
	uint64_t 		file_size;
	std::string 	file_name;
	size_t			data_offset;
};

static const int 				serializer_buffer_size = 4096;
typedef std::array<char, 
	serializer_buffer_size>		serializer_buffer;


class file_deserializer
{
private:

private:
	// path to storage
	std::string const	base_path;

private:
	serializer_header parse_header(serializer_buffer const& buf);

public:
	file_deserializer(file_deserializer const&) = delete;
	file_deserializer(std::string const& base_path);
	~file_deserializer();

	template <class t_istream>
	void deserialyze(t_istream& s);

	template <class t_istream>
	void operator << (t_istream& s);
};

template <class t_istream>
void file_deserializer::operator << (t_istream& s)
{
	deserialyze(s);
}

template <class t_istream>
void file_deserializer::deserialyze(t_istream& s)
{
	serializer_buffer 	buf;

	// read first 4k and extract reeiving
	// file properties
	s.read(buf.data(), buf.size());
	throw_if(!s.good(), std::runtime_error("can't read data from input stream"));
	auto header = parse_header(buf);

	// create output file stream
	std::string path = base_path + "/" + header.file_name;

	trace("receiving file " + path + "\n");

	std::ofstream f(path, std::ofstream::out | std::ofstream::binary);
	throw_if(!f.good(), std::runtime_error("can't create file " + path));

	// copy data from buffer to file
	uint64_t total_written = s.gcount() - header.data_offset;

	f.write(buf.data() + header.data_offset, total_written);
	throw_if(!f.good(), std::runtime_error("can't write to file " + path));

	while (!s.eof() && total_written < header.file_size)
	{
		s.read(buf.data(), buf.size());
		throw_if(!s.good(), std::runtime_error("can't read data from input stream"));

		f.write(buf.data(), s.gcount());
		throw_if(!f.good(), std::runtime_error("can't write to file " + path));

		total_written += s.gcount();
	}

	// check if the file size member in header matches saved size
	throw_if(header.file_size != total_written, 
		std::runtime_error("size of file " + path + " is incorrect"));

	trace("file " + path + " received and saved successfully\n");
}


/*
 * serializer
 */
class file_serializer
{
private:
	std::string 	filepath;

public:
	file_serializer(file_serializer const&) = delete;
	file_serializer(std::string const& filepath);
	~file_serializer();

	template <class t_ostream>
	void serialyze(t_ostream& s);

	template <class t_ostream>
	void operator >> (t_ostream& s);
};

template <class t_ostream>
void file_serializer::operator >> (t_ostream& s)
{
	serialyze(s);
}

template <class t_ostream>
void file_serializer::serialyze(t_ostream& s)
{
	uint64_t file_size = get_file_size(filepath);

	// format header
	std::string header = to_hexstr(file_size) + "\n" + strip_name(filepath) + "\n";
	header.push_back('\0');

	s.write(header.data(), header.size());
	throw_if(!s.good(), std::runtime_error("can't write to output stream"));

	// open input file stream
	std::ifstream file(filepath, std::ifstream::in | std::ifstream::binary);
	throw_if(!file.good(), std::runtime_error("can't open file " + filepath + " for reading"));

	// copy data
	serializer_buffer 	buf;
	uint64_t total_copied = 0;

	while (!file.eof() && total_copied < file_size)
	{
		size_t read = std::min<uint64_t>(buf.size(), file_size - total_copied);
		file.read(buf.data(), read);
		throw_if(
			!file.good(), 
			std::runtime_error("can't read file " + filepath + " at position " + std::to_string(total_copied))
		);

		s.write(buf.data(), read);
		throw_if(!s.good(), std::runtime_error("can't write to output stream"));

		total_copied += read;
	}
}

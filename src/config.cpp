#include <fstream>
#include "config.h"
#include "common.h"


config::config(std::istream& s)
{
	parse(s);
}

config::config(std::string const& config_path)
{
	std::ifstream config_file(config_path, std::ifstream::in);
	throw_if(!config_file.good(), std::runtime_error("can't open config file " + config_path));
	parse(config_file);
}

config::~config()
{
}

bool config::parse_value(std::string const& line, std::string& name, std::string& value)
{
	std::string s = trim(line);		

	// skip empty string and comments
	if (s.empty())
		return false;

	if (s[0] == '#')
		return false;

	// extract name and value
	size_t sep = s.find('=');
	if (sep == std::string::npos)
		throw std::runtime_error("config is invalid: expected symbol '=' in: " + line);

	// skip whitespaces and quates
	name = trim(s.substr(0, sep));
	value = trim(s.substr(sep + 1), " \t'\"");

	return true;
}

void config::parse(std::istream& s)
{
	while (!s.eof())
	{
		std::string line, name, value;
		std::getline(s, line);

		if (!parse_value(line, name, value))
			continue;

		data[name] = value;
	}
}

std::string const& config::gets(std::string const& name) const
{
	auto i = data.find(name);
	if (i == data.end())
		throw std::runtime_error("config is invalid: parameter '" + name + "' is undefined");

	return i->second;	
}

int config::geti(std::string const& name) const
{
	int res;

	try
	{
		res = std::stoi(gets(name));
	}
	catch (std::invalid_argument const& e)
	{
		throw std::runtime_error("config is invalid: expected parameter '" + name + "' is a number");
	}

	return res;
}

int config::geti(std::string const& name, int min, int max) const
{
	int res = geti(name);

	if (res < min || res > max)
		throw std::runtime_error("config is invalid: parameter '" + name + 
			"' should be in range [" + std::to_string(min) + "," + std::to_string(max) + "]");

	return res;
}


#ifndef __config_h__
#define __config_h__

#include <iostream>
#include <map>
#include <stdexcept>


typedef std::map<std::string, std::string>	string_map;


/*
 * config file parser
 *
 * constructor parses the given file stream and
 * saves all the pairs [key, value] into a map
 * it allows in simple form to require extracted values
 * by calling 
 *		config::getX(argument_name)
 * 
 */

class config
{
private:
	string_map	data;

	bool parse_value(std::string const& line, std::string& name, std::string& value);
	void parse(std::istream& s);

public:
	config(std::istream& s);
	config(std::string const& config_path);
	~config();

	int geti(std::string const& name) const;
	int geti(std::string const& name, int min, int max) const;

	std::string const& gets(std::string const& name) const;
};

#endif // __config_h__
